#include "benchmark.hh"
#define REPEAT 10'000'000UL

#include <vidre/log.hh>

using namespace vidre;

__benchmark_noinline__ uint32_t empty(uint32_t &i, uint32_t m) {
    i = (i + 1);
    return i;
}

__benchmark_noinline__ uint32_t modulo(uint32_t &i, uint32_t m) {
    i = (i + 1) % m;
    return i;
}

__benchmark_noinline__ uint32_t ifalg(uint32_t &i, uint32_t m) {
    i = (i + 1);
    if (i >= m)
        i = 0;
    return i;
}

__benchmark_noinline__ uint32_t mulalg(uint32_t &i, uint32_t m) {
    i = (i + 1) * (i < (m-1));
    return i;
}

int main(int argc, const char **argv) {
    benchmark::timer t;

    uint32_t res = 0;

    // 0. overhead of empty loop
    t.start();
    {
        uint32_t q = 0;
        for (uint32_t i = 0; i < REPEAT; ++i) {
            res = empty(q,0);
            benchmark::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "empty: " <<  t.duration<benchmark::nanoseconds>() / double(REPEAT) << " ns/call" << log::endl;

    // 1. modulo 137
    t.start();
    {
        uint32_t q = 0;
        for (uint32_t i = 0; i < REPEAT; ++i) {
            res = modulo(q, 137);
            benchmark::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "modulo 137: " << (t.duration<benchmark::nanoseconds>() / double(REPEAT)) << " ns/call"
                  << log::endl;

    // 2. if 137
    t.start();
    {
        uint32_t q = 0;
        for (uint32_t i = 0; i < REPEAT; ++i) {
            res = ifalg(q, 137);
            benchmark::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "if 137: " << (t.duration<benchmark::nanoseconds>() / double(REPEAT)) << " ns/call" << log::endl;

    // 3. modulo inline 137
    t.start();
    {
        uint32_t q = 0;
        for (uint32_t i = 0; i < REPEAT; ++i) {
            q = (q + 1) % 137;
            res = q;
            benchmark::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "modulo inline 137: " << (t.duration<benchmark::nanoseconds>() / double(REPEAT)) << " ns/call" << log::endl;

    // 3. mul cmp 137
    t.start();
    {
        uint32_t q = 0;
        for (uint32_t i = 0; i < REPEAT; ++i) {
            res = mulalg(q, 137);
            benchmark::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "mul cmp 137: " << (t.duration<benchmark::nanoseconds>() / double(REPEAT)) << " ns/call" << log::endl;

    return 0;
}