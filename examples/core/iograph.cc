#include <chrono>
#include <thread>
#include <vidre/io.hh>
#include <vidre/memory/base.hh>
#include <vidre/log.hh>

using namespace vidre;

class sensor : public virtual io::source {
public:
    sensor() = default;
    virtual ~sensor() = default;

protected:
    shared<io::packet::value> packet_{};

public:
    cap getcap() const noexcept override {
        return {cap::packet | cap::push | cap::event, {}};
    }

    shared<io::packet> pull() override {
        // FIXME
        // stats_.packets.dropped--;
        // stats_.packets.transmitted++;
        // stats_.bytes.transmitted += packet_->size();
        return packet_;
    }

protected:
    void source_step() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        packet_ = io::packet::make<io::packet::value>(chrono::isodate::now());

        if (cap_.source & cap::push) {
            auto retval = source_right_->push(packet_);
            stats_.update(packet_, retval);
        }
        else {
            notify();
            // FIXME
            // stats_.packets.dropped++;  // see get() above
        }
    }
};

class store : public virtual io::sink {
public:
    static void hexdump(memory::view view) {
        const auto *p = static_cast<const std::uint8_t *>(view.data());

        for (std::size_t offset = 0; offset < view.size(); offset += 16) {
            std::printf("%08zx  ", offset);

            // Hexadecimal representation
            for (std::size_t i = 0; i < 16; ++i) {
                if (offset + i < view.size())
                    std::printf("%02x ", p[offset + i]);
                else
                    std::printf("   ");

                if (i == 7)
                    std::printf(" ");
            }

            std::printf(" |");

            // ASCII representation
            const auto n = (view.size() - offset < 16) ? view.size() - offset : 16;

            for (std::size_t i = 0; i < n; ++i) {
                const auto c = p[offset + i];
                std::printf("%c", std::isprint(static_cast<unsigned char>(c)) ? static_cast<char>(c) : '.');
            }

            std::printf("|\n");
        }
    }

public:
    store() = default;
    virtual ~store() = default;

protected:
    uint8_t buf_[16];
    io::offset_type len_ = 0;

protected:
    io::offset_type write(const memory::view &buf) override {
        hexdump(buf);
        return buf.size();
    }

public:
    cap getcap() const noexcept override {
        return {{}, cap::packet | cap::pull | cap::event};
    }

    io::result push(shared<io::packet> p) override {
        auto retval = p->emit(*this);
        return retval;
    }
};

int main(int argc, char *argv[]) {
    log::debug() << "Testing datachain...\n" << std::format("{}", io::endpoint::cap::pull) << "\n";
    log::trace::pattern("*");

    auto inp = io::endpoint::make<sensor>();
    auto chp = io::endpoint::make<io::channel>();
    auto outp = io::endpoint::make<store>();

    inp->settag("input");
    chp->settag("channel");
    outp->settag("output");

    io::endpoint::connect(chp, outp);
    io::endpoint::connect(inp, chp);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    chp->unlink();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    outp->unlink();
    inp->unlink();

    return 0;
}

/**@example examples/core/iograph.cc
 * @details
 *
 * FIXME
 *
 * */
