#include <vidre/storage/file.hh>
#include <vidre/cpp/util.hh>

#include <cstring>
#include <chrono>
#include <iostream>

#define MiB      (1024UL * 1024UL)
#define GiB      (1024UL * 1024UL * 1024UL)
#define FILESIZE (40UL * GiB)

using namespace vidre;
using namespace storage;

int main(int argc, char **argv) {
    uint8_t buf[MiB] = {};
    memset(buf, 255, size_of(buf));

    auto f = file::io();
    f.open("hdspeed.dat", file::io::writeonly | file::io::create | file::io::truncate | file::io::synchronous);
    // f.open("hdspeed.dat", file::io::writeonly | file::io::create | file::io::truncate);

    auto tstart = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < (FILESIZE / size_of(buf)); ++i) {
        f.write(buf, size_of(buf));

        auto ttmp = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> tdur = ttmp - tstart;

        std::cout << std::format("\r{:9.3f}MiB/s    {:3.0f}%     ",  //
                                 double(i * size_of(buf)) / double(MiB) / tdur.count(),
                                 100.0 * double(i) / double((FILESIZE / size_of(buf))));
    }
    auto tstop = std::chrono::high_resolution_clock::now();
    f.close();

    std::chrono::duration<double> tdur = tstop - tstart;
    std::cout << std::format("\r{:9.3f}MiB/s                     ",  //
                             double(FILESIZE) / double(MiB) / tdur.count())
              << std::endl;

    storage::file::remove("hdspeed.dat");

    return 0;
}
