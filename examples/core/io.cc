#include <vidre/io.hh>
#include <vidre/exception.hh>

#include <iostream>
#include "vidre/io/types/source.hh"

using namespace vidre;

#define kiB (1024ULL)
#define MiB (1024ULL * kiB)
#define GiB (1024ULL * MiB)

//! [create-dummy-file]
std::filesystem::path create_dummy_file() {
    std::filesystem::path p = "/tmp/vidre.example.io.dat";

    if (not std::filesystem::exists(p)) {
        // dummy "data"
        char buf[4 * kiB];
        for (io::size_type i = 0; i < 4 * kiB; ++i)
            buf[i] = i;

        // dummy file
        io::file dummy(p, io::file::access::writeonly, io::file::disposition::create | io::file::disposition::truncate);
        if (not dummy.valid())
            throw exception::runtime() << error::system();

        for (io::size_type i = 0; i < (MiB); ++i)
            dummy.write({4 * kiB, buf});
    }

    return p;
}
//! [create-dummy-file]

struct files {
    io::source::shared src;
    io::sink::shared dst;

    static files make() {
        std::filesystem::path data = create_dummy_file();

        auto src = io::source::make<io::file::source>(data);
        auto dst = io::sink::make<io::file::sink>("/tmp/vidre.example.io.copy", io::file::sink::truncate);

        if (not src->valid())
            throw exception::runtime() << error::system();
        if (not dst->valid())
            throw exception::runtime() << error::system();

        if (not io::endpoint::match(src, dst))
            throw exception::logic() << "incompatible source and sink";

        return {src, dst};
    }
};

//! [copy-source-to-sink-manual]
void copy_source_to_sink_manual() {
    auto files = files::make();

    while (dynamic_cast<io::source*>(files.src.get())->step(dynamic_cast<io::sink*>(files.dst.get())))
        ;
}
//! [copy-source-to-sink-manual]

//! [copy-source-to-sink-route]
void copy_source_to_sink_route() {
    auto files = files::make();

    io::flow::route(files.src, files.dst).execute();
}
//! [copy-source-to-sink-route]

int main(int argc, const char **argv) {
    try {
        copy_source_to_sink_manual();
        copy_source_to_sink_route();
    } catch (exception &e) {
        std::cerr << error::diagnostic(e).dump() << std::endl;
    }

    return 0;
}

/**@example examples/core/io.cc
 * @details
 *
 * FIXME
 *
 * */
