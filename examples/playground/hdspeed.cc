#include <vidre/io/file.hh>
#include <vidre/cpp/util.hh>

#include <cstring>
#include <chrono>
#include <iostream>

#define kiB      (1024UL)
#define MiB      (1024UL * kiB)
#define GiB      (1024UL * MiB)
#define FILESIZE (40UL * GiB)

using namespace vidre;

int main(int argc, char **argv) {
    uint8_t buf[MiB] = {};
    memset(buf, 255, size_of(buf));

    auto f = io::file();
    f.open("hdspeed.dat", io::file::access::writeonly, io::file::disposition::create | io::file::disposition::truncate,
           io::file::hint::synchronous|io::file::hint::direct);
    // f.open("hdspeed.dat", io::file::access::writeonly, io::file::disposition::create | io::file::disposition::truncate);

    auto tstart = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < (FILESIZE / size_of(buf)); ++i) {
        f.write({size_of(buf), buf});

        auto ttmp = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> tdur = ttmp - tstart;

        std::cout << std::format("\r{:9.3f}MiB/s    {:3.0f}%     ",  //
                                 double(i * size_of(buf)) / double(MiB) / tdur.count(),
                                 100.0 * double(i) / (double(FILESIZE) / double(size_of(buf))));
    }
    auto tstop = std::chrono::high_resolution_clock::now();
    f.close();

    std::chrono::duration<double> tdur = tstop - tstart;
    std::cout << std::format("\r{:9.3f}MiB/s                     ",  //
                             double(FILESIZE) / double(MiB) / tdur.count())
              << std::endl;

    std::filesystem::remove("hdspeed.dat");

    return 0;
}
