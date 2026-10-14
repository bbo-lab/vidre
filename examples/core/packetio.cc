#include <vidre/io.hh>
#include <vidre/util/chrono.hh>
#include <vidre/log.hh>

#include <random>

#include <iostream>

using namespace vidre;
using namespace std::chrono_literals;

namespace examples {

    // forward declarations
    class store;

    // some tools for generating random data
    static std::random_device hwrand;

    // static std::mt19937 rng{std::random_device{}()};

    template <typename T>
    using uniform_random = std::uniform_int_distribution<T>;

    template <typename T>
    using normal_random = std::normal_distribution<T>;

    //! [simple-packet]
    // Reference time for heartbeat packets
    static chrono::reference heartbeat_reference = {};

    /* A "hearbeat" packet.
     * This packet represents a simple heartbeat message containing a computer time-stamp.
     */
    class heartbeat : public io::packet {
    public:
        static constexpr uint8_t major = 0xFE;  // our major number (user implementation)

        enum class minor : uint8_t {  // minor types
            undefined = 0x0000,
            systole = 0x0001
        };

    public:
        heartbeat() : io::packet(major, static_cast<uint16_t>(minor::systole)), cts_(chrono::timestamp::now()) {}
        explicit heartbeat(const header &hdr_) noexcept : packet{hdr_} {}

        virtual ~heartbeat() {}  // can be omitted

    protected:
        chrono::timestamp cts_{};  // Computer time-stamp of the last heartbeat.

    public:
        void finalize() noexcept override;
        io::result emit(io::sink &f) override;
        void absorb(memory::view view, std::endian endian = std::endian::native) override;

    private:
        friend class store;
    };

    void heartbeat::finalize() noexcept {
        // for fixed-size packets, this length could also be set in the constructors
        hdr.length = sizeof(chrono::timestamp);
    }

    io::result heartbeat::emit(io::sink &f) {
        // data is streamed in binary form
        return io::packet::emit(f)  // parent packet emit()
               + f.put(cts_)        // our data
               + pad(f);            // padding can be omitted if already aligned
    }

    void heartbeat::absorb(memory::view view, std::endian endian) {
        // check validity
        if (view.size() < size())
            throw exception::runtime() << "packet data is truncated";

        // copy data
        memory::copy(view + sizeof(header), cts_);

        // convert endiannes if necessary
        if (endian != std::endian::native) {
            cts_.sec = endian::byteswap(cts_.sec, endian);
            cts_.nsec = endian::byteswap(cts_.nsec, endian);
        }
    }
    //! [simple-packet]

    //! [simple-packet-registry]
    static io::packet::registry<heartbeat>  //
        packet_factory_heartbeat(heartbeat::major, heartbeat::minor::systole);
    //! [simple-packet-registry]

    //! [simple-source]
    /* A simple source that generates heartbeat packets at regular intervals.
     *
     * This source, emitting a periodic heartbeat packet, demonstrates how the
     * underlying I/O framework is used to emit the heartbeat packets. Actual
     * implementations, however, require additional logic and also require a
     * "manifest", representing a device's capabilities and configuration,
     * likewise in packet format, so that it can be stored or transmitted as
     * needed.
     *
     */
    class pacemaker : public io::source {
    public:
        pacemaker() = default;
        virtual ~pacemaker() {}

    protected:
        io::packet::queue packet_queue_{};  // dynamic store

    public:
        cap getcap() const noexcept override {
            // we can only do event-based transmission
            return {cap::packet | cap::event, {}};
        }

        io::size_type queue_depth() const noexcept override {
            // our packets are dynamically allocated
            return io::dynamic;
        }

        shared<io::packet> get() override {
            // FIXME stats
            if (!packet_queue_.empty())
                return packet_queue_.pop();

            return io::packet::nil;
        }

    protected:
        void source_handler(shared<task> self, std::chrono::milliseconds pause = std::chrono::milliseconds(1)) override;
    };

    void pacemaker::source_handler(shared<task> self, std::chrono::milliseconds pause) {
        task::scope scope(self);
        log::debug() << "pacemaker: started...\n";

        // wait for connection
        while (!source_follower_ or not source_follower_->isready()) {
            if (scope.isterminated())
                return;

            task::self::sleep_us(100);
        }

        // install "cancellation" handler
        log::debug() << "pacemaker: preparing...\n";
        self->on_terminate([this]() {
            this->cancel();
        });

        // event loop for generating heartbeat packets
        log::debug() << "pacemaker: running...\n";
        auto next = chrono::monotonic::now();
        while (not self->isterminated()) {
            next += 1s;

            // emit heartbeat packet
            // FIXME stats
            auto pkt = io::packet::make<heartbeat>();
            packet_queue_.push(pkt);
            notify();
            
            task::self::sleep_until(next);
        }
        
        log::debug() << "pacemaker: finished.\n";
    }
    //! [simple-source]

    //! [simple-store]
    class store : public io::sink {
    public:
        static constexpr size_t buffer_size = 4096;

    public:
        store() = default;
        virtual ~store() = default;

    protected:
        uint8_t buf_[buffer_size];
        io::offset_type len_ = 0;

    protected:
        io::offset_type write(const memory::view &buf) override {
            // write data to storage
            memory::copy(buf, memory::area{buffer_size - len_, buf_ + len_});
            len_ += buf.size();

            return buf.size();
        }

    public:
        cap getcap() const noexcept override {
            return {{}, cap::packet | cap::pull | cap::event};
        }

        status put(shared<io::packet> p) override {
            auto retval = p->emit(*this);
            // FIXME stats

            inspect();

            return retval ? status::ok : status::bad;
        }

        // inspect the data - for demonstration purposes only
        void inspect();

    private:
        // FIXME
    };
    //! [simple-store]

    void store::inspect() {
        try {
            auto pkt = io::packet::decode({static_cast<io::size_type>(len_), buf_});
            switch (pkt->getmajor()) {
                case io::packet::major::log:
                    // FIXME
                    break;
                default: {  // user types
                    auto id = pkt->getid();

                    if (id == heartbeat::major) {
                        auto hb = std::static_pointer_cast<heartbeat>(pkt);
                        auto t = static_cast<chrono::monotonic::time_point>(hb->cts_) -
                                 static_cast<chrono::monotonic::time_point>(heartbeat_reference);
                        double seconds = std::chrono::duration<double>(t).count();
                        std::cout << std::endl << "HEARTBEAT " << seconds << " s" << std::endl;
                    }
                }

                break;
            }
        } catch (...) {
            std::cout << "NIL" << std::endl;
        }

        len_ = 0;
    }

    //! [simple-filter]
    // FIXME
    //! [simple-filter]

}  // namespace examples

int main(int argc, char *argv[]) {
    examples::heartbeat_reference.set();

    // create endpoints
    shared<examples::pacemaker> pacemaker = io::endpoint::make<examples::pacemaker>();
    shared<examples::store> store = io::endpoint::make<examples::store>();

    // connect the endpoints
    io::endpoint::connect(pacemaker, store);

    // let the endpoints run for a while
    task::self::sleep_for(std::chrono::seconds(10));

    // disconnect the endpoints
    io::endpoint::disconnect(pacemaker);
    io::endpoint::disconnect(store);

    return 0;
}

/**@example examples/core/packetio.cc
 * @details
 *
 * This exmample demonstrates how to use the packet I/O functionality provided by the vidre library.
 *
 * FIXME
 *
 * @verbatim
 *                        _______
 *     Heartbeat --------|       |
 *                       |  fan  |     ________      _________
 *     Accellerometer ---|   -   |----| Filter |----| Storage |
 *                       |  in   |     --------      ---------
 *     Log --------------|       |
 *                        -------
 * @endverbatim
 *
 *
 *
 * */
