#include <vidre/io.hh>
#include <vidre/exception.hh>

#include <iostream>

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

//! [copy-source-to-sink]
void copy_source_to_sink() {
    std::filesystem::path data = create_dummy_file();

    auto src = io::source::make<io::file::source>(data);
    auto dst = io::sink::make<io::file::sink>("/tmp/vidre.example.io.copy", io::file::sink::truncate);

    if (not src->valid())
        throw exception::runtime() << error::system();
    if (not dst->valid())
        throw exception::runtime() << error::system();    

    connect(src, dst);
}
//! [copy-source-to-sink]

int main(int argc, const char **argv) {
    try {
        copy_source_to_sink();
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
