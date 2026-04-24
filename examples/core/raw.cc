#include <vidre/storage/raw.hh>
#include <vidre/video.hh>
#include <vidre/log.hh>

#include <format>
#include <iostream>
#include <locale>

#include "benchmark/benchmark.hh"

using namespace vidre;
namespace raw = vidre::storage::raw;

#define REPEAT 10000000

namespace fixed {
    //! [fixed-elements]
    using index = raw::element::fixed<uint64_t, raw::major::video, raw::video::index>;
    using geometry = raw::element::fixed<video::geometry, raw::major::video, raw::video::geometry>;
    using timestamp = raw::element::fixed<video::timestamp, raw::major::video, raw::video::timestamp>;

    // fixed-size chunk header
    struct __vidre_packed__ header {
        fixed::index index;
        fixed::geometry geometry;
        fixed::timestamp timestamp;
        fixed::timestamp computertime;

        storage::offset_type put(storage::sink &f) const noexcept {
            return f.put(this, 1);
        }
    };
    //! [fixed-elements]
}  // namespace fixed

namespace variable {
    //! [dynamic-elements]
    struct __vidre_packed__ index : public virtual raw::element::dynamic {
        uint64_t counter{};

        index() noexcept : raw::element::dynamic(raw::major::video, raw::video::index, sizeof(raw::prefix) + sizeof(counter)) {}
        virtual ~index() {}

        offset_type put(storage::sink &f) const noexcept override {
            return dynamic::put(f) + f.put(&counter, 1);
        }
    };

    struct __vidre_packed__ geometry : public virtual raw::element::dynamic {
        video::geometry content{};

        geometry() noexcept
            : raw::element::dynamic(raw::major::video, raw::video::geometry, sizeof(raw::prefix) + sizeof(content)) {}
        virtual ~geometry() {}

        offset_type put(storage::sink &f) const noexcept override {
            return dynamic::put(f) + f.put(&content, 1);
        }
    };

    struct __vidre_packed__ timestamp : public virtual raw::element::dynamic {
        video::timestamp content{};

        timestamp() noexcept
            : raw::element::dynamic(raw::major::video, raw::video::timestamp, sizeof(raw::prefix) + sizeof(content)) {}
        virtual ~timestamp() {}

        offset_type put(storage::sink &f) const noexcept override {
            return dynamic::put(f) + f.put(&content, 1);
        }
    };

    struct header : public virtual raw::element::sequence {
        variable::index *index{};
        variable::geometry *geometry{};
        variable::timestamp *camera{};
        variable::timestamp *computer{};

        header() noexcept : raw::element::sequence() {
            index = add<variable::index>();
            geometry = add<variable::geometry>();
            camera = add<variable::timestamp>();
            computer = add<variable::timestamp>();
        }

        virtual ~header() {}
    };
    //! [dynamic-elements]
}  // namespace variable

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");

    std::locale::global(std::locale(""));

    log::info() << std::format("Running raw storage benchmark with {:L} iterations.", REPEAT) << log::endl << log::endl;
    benchmark::timer t;

    // write fixed elements
    {
        fixed::header hdr;
        storage::file::sink f;
        f.open(PROJECT_BINARY_DIR "/fixed.tmp", storage::file::sink::direct);
        if (not f.valid())
            log::fatal() << "Could not open file: " << std::strerror(errno) << log::endl;

        t.start();
        for (int i = 0; i < REPEAT; ++i) {
            hdr.index.content = i;
            hdr.geometry.content = {{uint32_t(i), uint32_t(i)}, {uint32_t(i), uint32_t(i)}};
            hdr.timestamp.content.value = uint64_t(i);
            // hdr.computertime.content.now();
            hdr.put(f);
        }
        t.stop();

        log::info() << "Fixed elements" << log::endl;
        log::info() << "         duration: " << std::format("{:.3f} ms", t.duration<benchmark::milliseconds>()) << log::endl;
        log::info() << "       throughput: "
                    << std::format("{:.3f}", (REPEAT * sizeof(hdr)) / t.duration<benchmark::seconds>() / (1024.0 * 1024.0))
                    << " MiB/s" << log::endl;
        log::info() << "    call overhead: " << std::format("{:.3f} ns", t.duration<benchmark::nanoseconds>() / REPEAT)
                    << log::endl;
    }

    // write dynamic elements
    {
        variable::header hdr;
        storage::file::sink f;
        f.open(PROJECT_BINARY_DIR "/dynamic.tmp", storage::file::sink::direct);
        if (not f.valid())
            log::fatal() << "Could not open file: " << std::strerror(errno) << log::endl;

        t.start();
        for (int i = 0; i < REPEAT; ++i) {
            hdr.index->counter = i;
            hdr.geometry->content = {{uint32_t(i), uint32_t(i)}, {uint32_t(i), uint32_t(i)}};
            hdr.camera->content.value = uint64_t(i);
            // hdr.computertime->content.now();
            hdr.put(f);
        }
        t.stop();

        log::info() << log::endl;
        log::info() << "Dynamic elements" << log::endl;
        log::info() << "         duration: " << std::format("{:.3f} ms", t.duration<benchmark::milliseconds>()) << log::endl;
        log::info() << "       throughput: "
                    << std::format("{:.3f}", (REPEAT * sizeof(hdr)) / t.duration<benchmark::seconds>() / (1024.0 * 1024.0))
                    << " MiB/s" << log::endl;
        log::info() << "    call overhead: " << std::format("{:.3f} ns", t.duration<benchmark::nanoseconds>() / REPEAT)
                    << log::endl;
    }

    // check consistency
    {
        storage::file::io fix, var;

        fix.open(PROJECT_BINARY_DIR "/fixed.tmp", storage::file::io::readonly);
        var.open(PROJECT_BINARY_DIR "/dynamic.tmp", storage::file::io::readonly);

        // different size?
        if (fix.size() != var.size())
            log::fatal() << "File size mismatch: " << fix.size() << " vs. " << var.size() << log::endl;

        // different content?
        t.start();
        uint8_t fbuf[4096], vbuf[4096];
        storage::file::io::offset_type pos = 0;
        while (true) {
            auto fn = fix.read(fbuf, sizeof(fbuf));
            auto vn = var.read(vbuf, sizeof(vbuf));

            if (fn != vn)
                log::fatal() << "Read size mismatch: " << fn << " vs. " << vn << log::endl;
            if (std::memcmp(fbuf, vbuf, fn) != 0)
                log::fatal() << "File content mismatch at offset " << pos << log::endl;
            if (fn == 0)
                break;
            pos += fn;
        }
        t.stop();

        log::info() << log::endl;
        log::info() << "Checked " << pos << " bytes ("
                    << std::format("{:.3f} MiB/s", double(pos) / t.duration<benchmark::seconds>() / (1024.0 * 1024.0))
                    << ")." << log::endl;
        log::info() << "Files are consistent." << log::endl;
    }

    // remove temporary files
    storage::file::remove(PROJECT_BINARY_DIR "/fixed.tmp");
    storage::file::remove(PROJECT_BINARY_DIR "/dynamic.tmp");

    return 0;
}

/**@example examples/core/raw.cc
 * @details
 * Raw storage format usage example.
 * */
