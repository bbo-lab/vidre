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

        storage::sink::offset_type put(storage::sink &f) const noexcept {
            return f.put(this, 1);
        }
    };
    //! [fixed-elements]
}  // namespace fixed

namespace dynamic {
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
        video::geometry value{};

        geometry() noexcept : raw::element::dynamic(raw::major::video, raw::video::geometry, sizeof(raw::prefix) + sizeof(value)) {}
        virtual ~geometry() {}

        offset_type put(storage::sink &f) const noexcept override {
            return dynamic::put(f) + f.put(&value, 1);
        }
    };

    struct __vidre_packed__ timestamp : public virtual raw::element::dynamic {
        video::timestamp value{};

        timestamp() noexcept
            : raw::element::dynamic(raw::major::video, raw::video::timestamp, sizeof(raw::prefix) + sizeof(value)) {}
        virtual ~timestamp() {}

        offset_type put(storage::sink &f) const noexcept override {
            return dynamic::put(f) + f.put(&value, 1);
        }
    };

    struct header : public virtual raw::element::sequence {
        raw::element::shared counter_;
        raw::element::shared geometry_;
        raw::element::shared camera_;
        raw::element::shared computer_;

        header() noexcept : raw::element::sequence() {
            counter_ = append(new ::dynamic::index());
            geometry_ = append(new ::dynamic::geometry());
            camera_ = append(new ::dynamic::timestamp());
            computer_ = append(new ::dynamic::timestamp());
        }

        virtual ~header() {}

        ::dynamic::index *index() const noexcept {
            return dynamic_cast<::dynamic::index *>(counter_.get());
        }

        ::dynamic::geometry *geometry() const noexcept {
            return dynamic_cast<::dynamic::geometry *>(geometry_.get());
        }

        ::dynamic::timestamp *camera() const noexcept {
            return dynamic_cast<::dynamic::timestamp *>(camera_.get());
        }

        ::dynamic::timestamp *computertime() const noexcept {
            return dynamic_cast<::dynamic::timestamp *>(computer_.get());
        }
    };
    //! [dynamic-elements]
}  // namespace dynamic

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");

    std::locale::global(std::locale(""));

    log::info() << std::format("Running raw storage benchmark with {:L} iterations.", REPEAT) << log::endl << log::endl;
    benchmark::timer t;

    // write fixed elements
    {
        fixed::header hdr;
        storage::filesink f;
        f.open(PROJECT_BINARY_DIR "/fixed.tmp", storage::filesink::synchronous);
        if (not f.valid())
            log::fatal() << "Could not open file: " << std::strerror(errno) << log::endl;

        t.start();
        for (int i = 0; i < REPEAT; ++i) {
            hdr.index.value = i;
            hdr.geometry.value = {{uint32_t(i), uint32_t(i)}, {uint32_t(i), uint32_t(i)}};
            hdr.timestamp.value.value = uint64_t(i);
            // hdr.computertime.value.now();
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

        storage::file::remove(PROJECT_BINARY_DIR "/fixed.tmp");
    }

    // write dynamic elements
    {
        dynamic::header hdr;
        storage::filesink f;
        f.open(PROJECT_BINARY_DIR "/dynamic.tmp", storage::filesink::synchronous);
        if (not f.valid())
            log::fatal() << "Could not open file: " << std::strerror(errno) << log::endl;

        t.start();
        for (int i = 0; i < REPEAT; ++i) {
            hdr.index()->counter = i;
            hdr.geometry()->value = {{uint32_t(i), uint32_t(i)}, {uint32_t(i), uint32_t(i)}};
            hdr.camera()->value.value = uint64_t(i);
            // hdr.computertime()->value.now();
            hdr.put(f);
        }
        t.stop();

        log::info() << "Dynamic elements" << log::endl;
        log::info() << "         duration: " << std::format("{:.3f} ms", t.duration<benchmark::milliseconds>()) << log::endl;
        log::info() << "       throughput: "
                    << std::format("{:.3f}", (REPEAT * sizeof(hdr)) / t.duration<benchmark::seconds>() / (1024.0 * 1024.0))
                    << " MiB/s" << log::endl;
        log::info() << "    call overhead: " << std::format("{:.3f} ns", t.duration<benchmark::nanoseconds>() / REPEAT)
                    << log::endl;

        storage::file::remove(PROJECT_BINARY_DIR "/dynamic.tmp");
    }

    return 0;
}

/**@example examples/core/raw.cc
 * @brief Raw storage format usage example.
 * */
