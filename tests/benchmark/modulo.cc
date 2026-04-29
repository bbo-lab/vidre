#define REPEAT 1'000'000'000UL
#define MODULO 137

#include <vidre/util/chrono.hh>
#include <vidre/util/debug.hh>
#include <vidre/cpp/format.hh>
#include <vidre/log.hh>

using namespace vidre;

__vidre_noinline__ int increment_only(int &i, int m) {
    i = (i + 1);
    return i;
}

__vidre_noinline__ int using_modulo(int &i, int m) {
    i = (i + 1) % m;
    return i;
}

__vidre_noinline__ int using_if(int &i, int m) {
    i = (i + 1);
    if (i >= m) 
        i = 0;
    return i;
}

__vidre_noinline__ int using_unlikely(int &i, int m) {
    i = (i + 1);
    if (i >= m) [[unlikely]]
        i = 0;
    return i;
}

__vidre_noinline__ int using_cmpmul(int &i, int m) {
    i = (i + 1) * (i < (m - 1));
    return i;
}

int main(int argc, const char **argv) {
    chrono::timer t;

    // 0.
    t.start();
    {
        int res = 0;
        for (unsigned i = 0; i < REPEAT; ++i) {
            res = increment_only(res, 0);
            debug::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "increment only:          " << std::format("{:.3f} ns/call", t.duration<chrono::timer::nanoseconds>(REPEAT))
                  << log::endl;

    // 1.
    t.start();
    {
        int res = 0;
        for (unsigned i = 0; i < REPEAT; ++i) {
            res = using_modulo(res, MODULO);
            debug::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "increment modulo:        " << std::format("{:.3f} ns/call", t.duration<chrono::timer::nanoseconds>(REPEAT))
                  << log::endl;

    // 2.
    t.start();
    {
        int res = 0;
        for (unsigned i = 0; i < REPEAT; ++i) {
            res = (res + 1) % MODULO;
            debug::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "increment modulo inline: " << std::format("{:.3f} ns/call", t.duration<chrono::timer::nanoseconds>(REPEAT))
                  << log::endl;

    // 3.
    t.start();
    {
        int res = 0;
        for (unsigned i = 0; i < REPEAT; ++i) {
            res = using_if(res, MODULO);
            debug::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "limit using if:          " << std::format("{:.3f} ns/call", t.duration<chrono::timer::nanoseconds>(REPEAT))
                  << log::endl;

    // 4.
    t.start();
    {
        int res = 0;
        for (unsigned i = 0; i < REPEAT; ++i) {
            res = using_unlikely(res, MODULO);
            debug::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "limit using unlikely:    " << std::format("{:.3f} ns/call", t.duration<chrono::timer::nanoseconds>(REPEAT))
                  << log::endl;

    // 5.
    t.start();
    {
        int res = 0;
        for (unsigned i = 0; i < REPEAT; ++i) {
            res = using_cmpmul(res, MODULO);
            debug::do_not_optimize(res);
        }
    }
    t.stop();
    log::notice() << "limit using cmpmul:      " << std::format("{:.3f} ns/call", t.duration<chrono::timer::nanoseconds>(REPEAT))
                  << log::endl;

    return 0;
}