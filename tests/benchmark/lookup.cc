/*
For dealing with data packets (see vidre::io::packet) we need to establish a
fast way to lookup decoder functions for a given packet type. This benchmark
is intended to test the performance of different lookup strategies, such as
hash maps, sorted vectors with binary search, or perfect hashing.
*/

#include <vidre/util/uuid.hh>
#include <vidre/util/chrono.hh>
#include <vidre/io/types.hh>
#include <vidre/log.hh>

#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>

#include <ankerl/unordered_dense.h>

#include <dlfcn.h>

#define REPEAT 1'000'000

using namespace vidre;

int main(int argc, const char **argv) {
    chrono::timer t;
    volatile uint64_t count = 0;

    // 0. prepare test data
    using function_type = void (*)(io::sink &, const io::packet::basic &);

    // uuid<->function map
    std::vector<std::pair<uuid, function_type>> data;
    for (uint32_t i = 0; i < 1024; ++i) {
        data.emplace_back(uuid::makeRandom(),
                          reinterpret_cast<function_type>(dlsym(RTLD_NEXT, uuid::makeRandom().tostring().c_str())));
    }

    // number of elements to lookup
    int N[] = {1, 5, 11, 16, 43, 64, 127};

    // 1. get overhead of empty loop
    t.start();
    for (int n : N) {
        int q = 0;
        for (int i = 0; i < REPEAT; ++i) {
            count += uint64_t(data[q].second);

            q = (q + 1);
            q = q * (q < n);
        }
    }
    t.stop();
    double overhead = t.duration<chrono::timer::nanoseconds>() / double(REPEAT * size_of(N));
    log::notice() << "overhead: " << overhead << " ns/call" << log::endl;

#if 1
    // 2. linear search
    log::notice() << "LINEAR SEARCH" << log::endl;
    for (int n : N) {
        log::notice() << std::format("    {:>4} elements:", n);
        t.start();
        {
            int q = 0;
            for (int i = 0; i < REPEAT; ++i) {
                auto key = data[q].first;

                auto it = std::find_if(data.begin(), data.end(), [&](const auto &p) {
                    return p.first == key;
                });
                count += uint64_t(it->second);

                q = (q + 1);
                q = q * (q < n);
            }
        }
        t.stop();
        double duration = t.duration<chrono::timer::nanoseconds>() / double(REPEAT);
        log::notice() << std::format("{:>9.3f} ns/call\n", duration - overhead);
    }
#endif

#if 2
    // 3. map
    {
        using map_type = std::map<uuid, function_type>;

        log::notice() << "STD::MAP" << log::endl;
        for (int n : N) {
            log::notice() << std::format("    {:>4} elements:", n);

            map_type map;
            for (int i = 0; i < n; ++i) {
                map[data[i].first] = data[i].second;
            }

            t.start();
            {
                int q = 0;
                for (int i = 0; i < REPEAT; ++i) {
                    auto key = data[q].first;

                    auto it = map.find(key);
                    count += uint64_t(it->second);

                    q = (q + 1);
                    q = q * (q < n);
                }
            }
            t.stop();
            double duration = t.duration<chrono::timer::nanoseconds>(REPEAT);
            log::notice() << std::format("{:>9.3f} ns/call\n", duration - overhead);
        }
    }
#endif

    // 4. unordered_map
    {
        using map_type = std::unordered_map<uuid, function_type>;

        log::notice() << "STD::UNORDERED_MAP" << log::endl;
        for (int n : N) {
            log::notice() << std::format("    {:>4} elements:", n);

            map_type map;
            for (int i = 0; i < n; ++i) {
                map[data[i].first] = data[i].second;
            }

            t.start();
            {
                int q = 0;
                for (int i = 0; i < REPEAT; ++i) {
                    auto key = data[q].first;

                    auto it = map.find(key);
                    count += uint64_t(it->second);

                    q = (q + 1);
                    q = q * (q < n);
                }
            }
            t.stop();
            double duration = t.duration<chrono::timer::nanoseconds>(REPEAT);
            log::notice() << std::format("{:>9.3f} ns/call\n", duration - overhead);
        }
    }

    // 5.  ankerl::unordered_dense::segmented_map
    {
        using map_type = ankerl::unordered_dense::segmented_map<uuid, function_type>;

        log::notice() << "ANKERL::UNORDERED_DENSE:SEGMENTED_MAP" << log::endl;
        for (int n : N) {
            log::notice() << std::format("    {:>4} elements:", n);

            map_type map;
            for (int i = 0; i < n; ++i) {
                map[data[i].first] = data[i].second;
            }

            t.start();
            {
                int q = 0;
                for (int i = 0; i < REPEAT; ++i) {
                    auto key = data[q].first;

                    auto it = map.find(key);
                    count += uint64_t(it->second);

                    q = (q + 1);
                    q = q * (q < n);
                }
            }
            t.stop();
            double duration = t.duration<chrono::timer::nanoseconds>(REPEAT);
            log::notice() << std::format("{:>9.3f} ns/call\n", duration - overhead);
        }
    }

    // 6.  ankerl::unordered_dense::map
    {
        using map_type = ankerl::unordered_dense::map<uuid, function_type>;

        log::notice() << "ANKERL::UNORDERED_DENSE:MAP" << log::endl;
        for (int n : N) {
            log::notice() << std::format("    {:>4} elements:", n);

            map_type map;
            for (int i = 0; i < n; ++i) {
                map[data[i].first] = data[i].second;
            }

            t.start();
            {
                int q = 0;
                for (int i = 0; i < REPEAT; ++i) {
                    auto key = data[q].first;

                    auto it = map.find(key);
                    count += uint64_t(it->second);

                    q = (q + 1);
                    q = q * (q < n);
                }
            }
            t.stop();
            double duration = t.duration<chrono::timer::nanoseconds>(REPEAT);
            log::notice() << std::format("{:>9.3f} ns/call\n", duration - overhead);
        }
    }

    return 0;
}
