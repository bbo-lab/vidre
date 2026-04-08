#include <vidre/storage/raw.hh>
#include <vidre/storage/io.hh>
#include <vidre/video/common.hh>
#include <vidre/log.hh>
#include "benchmark.hh"

using namespace vidre;

#define REPEAT 10000000
#define SET_BUFFERING storage::filesink::mode_type::none

// #define REPEAT 1000000 // considerably slower...
// #define SET_BUFFERING storage::filesink::mode_type::unbuffered

namespace raw = vidre::storage::raw;
namespace raw = vidre::storage::raw;

struct __attribute__((packed, aligned(8))) index_type {
    uint64_t index;
    uint64_t dummy;
};

struct VIDRE_CORE_PACKED_VISIBILITY static_header {
#define MAKE_STATIC_TLV(NAME, TYPE) \
    raw::tag NAME##_tag{};          \
    TYPE NAME {}

    raw::tag grp{{raw::major::meta, raw::meta::group}, 128 - 16};
    MAKE_STATIC_TLV(index, index_type);
    MAKE_STATIC_TLV(geometry, video::geometry);
    MAKE_STATIC_TLV(timestamp, video::timestamp);
    MAKE_STATIC_TLV(computertime, video::timestamp);

    std::size_t put(storage::sink &f) const volatile noexcept {
        return f.put(const_cast<static_header *>(this), 1);
    }
};


void test_static() {
    benchmark::timer t;

    volatile static_header hdr{};

    storage::filesink f;
    f.open("static.tmp", storage::filesink::synchronous | SET_BUFFERING);
    if (not f.valid())
        log::fatal() << "Could not open file: " << std::strerror(errno) << log::endl;

    t.start();
    for (int i = 0; i < REPEAT; ++i) {
        hdr.put(f);
        hdr.index.index = hdr.index.index + 1;
    }
    t.stop();

    auto speed = double(sizeof(hdr) * REPEAT) / t.duration();
    log::notice() << "Static performance: " << std::format("{:.3f} MiB/s", speed / double(MiB(1))) <<  //
        std::format(" ({:.1} ns/call)", (1e9 / speed)) << log::endl;
}

struct VIDRE_CORE_VISIBILITY myindex : public virtual raw::entry {
    uint64_t index{};
    uint64_t dummy{};

    myindex() noexcept : raw::entry(raw::major::undefined, raw::value::undefined, sizeof(uint64_t)) {}

    virtual std::size_t put(storage::sink &f) const noexcept {
        return raw::entry::put(f) + f.put(&index, 1);
    }
};

struct VIDRE_CORE_VISIBILITY mygeometry : public virtual raw::entry {
    video::geometry geometry{};

    mygeometry() noexcept : raw::entry(raw::major::undefined, raw::value::undefined, sizeof(video::geometry)) {}

    virtual std::size_t put(storage::sink &f) const noexcept {
        return raw::entry::put(f) + f.put(&geometry, 1);
    }
};

struct VIDRE_CORE_VISIBILITY mytimestamp : public virtual raw::entry {
    video::timestamp timestamp{};

    mytimestamp() noexcept : raw::entry(raw::major::undefined, raw::value::undefined, sizeof(video::timestamp)) {}

    virtual std::size_t put(storage::sink &f) const noexcept {
        return raw::entry::put(f) + f.put(&timestamp, 1);
    }
};

void test_dynamic() {
    benchmark::timer t;
    raw::group hdr;

    auto index = hdr.append(new myindex());
    hdr.append(new mygeometry());
    hdr.append(new mytimestamp());
    auto computertime = hdr.append(new mytimestamp());

    storage::filesink f;
    f.open("dynamic.tmp", storage::filesink::synchronous | SET_BUFFERING);
    if (not f.valid())
        log::fatal() << "Could not open file: " << std::strerror(errno) << log::endl;

    t.start();
    for (int i = 0; i < REPEAT; ++i) {
        hdr.put(f);
        dynamic_cast<myindex *>(index.get())->index++;
    }
    t.stop();

    auto speed = double(sizeof(hdr) * REPEAT) / t.duration();
    log::notice() << "Dynamic performance: " << std::format("{:.3f} MiB/s", speed / double(MiB(1)))  //
                  << std::format(" ({:.1} ns/call)", (1e9 / speed)) << log::endl;
}

int main(int argc, const char **argv) {
    test_static();
    test_dynamic();
    return 0;
}