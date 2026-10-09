#include <vidre/io.hh>
#include <vidre/util/chrono.hh>

using namespace vidre;

/* A "hearbeat" packet.
 */
//! [simple-packet]
class heartbeat : public io::packet {
public:
    static constexpr uint16_t major = 0xFEED;  // our major number (user implementation)
    static constexpr uint16_t minor = 0x0001;  // avoiding "0" for group packets

public:
    heartbeat();
    heartbeat(const header &hdr_) noexcept : packet{hdr_} {}

    virtual ~heartbeat() {}

protected:
    chrono::timestamp cts_{};  ///< Computer time-stamp of the last heartbeat.

public:
    io::result emit(io::sink &f) override;
    void absorb(memory::view view, std::endian endian = std::endian::native) override;
};

heartbeat::heartbeat() : cts_{chrono::timestamp::now()} {
    // static size, no "finalize()" needed
    hdr.length = sizeof(chrono::timestamp);
}

io::result heartbeat::emit(io::sink &f) {
    return f.put(cts_);  // padding omitted, as the data is a multiple of `io::alignment`
}

void heartbeat::absorb(memory::view view, std::endian endian) {
    // check validity
    if (view.size() < size())
        throw exception::runtime() << "packet data is truncated";

    // copy data
    memory::view ts_view = view + sizeof(header);
    cts_ = *reinterpret_cast<const chrono::timestamp *>(ts_view.data());

    // convert endiannes if necessary
    if (endian != std::endian::native) {
        cts_.sec = endian::byteswap(cts_.sec, endian);
        cts_.nsec = endian::byteswap(cts_.nsec, endian);
    }
}
//! [simple-packet]

//! [simple-packet-registry]
static io::packet::registry<heartbeat>  //
    packet_factory_heartbeat(heartbeat::major, heartbeat::minor);
//! [simple-packet-registry]

//! [simple-source]
class heartbeat_source : public io::source {
public:
    heartbeat_source() = default;
    virtual ~heartbeat_source() {}
};
//! [simple-source]

int main(int argc, char *argv[]) {
    return 0;
}

/**@example examples/core/packetio.cc
 * @details
 *
 * This exmample demonstrates how to use the packet I/O functionality provided by the vidre library.
 *
 * FIXME... list of topics covered
 *
 * */
