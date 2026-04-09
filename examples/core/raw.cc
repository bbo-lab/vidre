#include <vidre/storage/raw.hh>
#include <vidre/video.hh>
#include <vidre/log.hh>

#include <iostream>

using namespace vidre;
namespace raw = vidre::storage::raw;

//! [fixed-elements]
namespace chunk {
    using index = raw::element::fixed<uint64_t, raw::major::video, raw::video::index>;
    using geometry = raw::element::fixed<video::geometry, raw::major::video, raw::video::geometry>;
    using timestamp = raw::element::fixed<video::timestamp, raw::major::video, raw::video::timestamp>;

    // fixed-size chunk header
    struct __vidre_packed__ header {
        chunk::index index;
        chunk::geometry geometry;
        chunk::timestamp timestamp;
        chunk::timestamp computertime;

        storage::sink::offset_type put(storage::sink &f) const noexcept {
            return f.put(this,1);
        }
    };
}  // namespace header
//! [fixed-elements]

int main(int argc, const char **argv) {
    log::io::stream::threshold(0, log::level::trace);
    log::trace::pattern("*");

    log::notice() << "sizeof(chunk::header) = " << sizeof(chunk::header) << log::endl;

    return 0;
}

/**@example examples/core/raw.cc
 * @brief Raw storage format usage example.
 * */
