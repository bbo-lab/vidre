#include "benchmark.hh"

#include <vidre/exception.hh>

#define REPEAT (10000000UL)

volatile int good = 10;
volatile int bad = -1;

__attribute__((noinline)) void error_handler(int ec [[maybe_unused]]) {
    //
}

__attribute__((noinline)) int good_value() {
    return good;
}

__attribute__((noinline)) int bad_value() {
    return bad;
}

__attribute__((noinline)) int exception_value() {
    throw bad;
    return good;
}

int main(int argc, const char **argv) {
    // warm-up
    {
        for (size_t i = 0; i < REPEAT; ++i) {
            error_handler(bad);
        }
    }

    // error-handler call
    benchmark::timer t_eh;
    {
        t_eh.start();
        for (size_t i = 0; i < REPEAT; ++i) {
            error_handler(bad);
        }
        t_eh.stop();

        std::cout << t_eh.duration<benchmark::nanoseconds>() / REPEAT << " ns" << std::endl;
    }

    // good call
    benchmark::timer t_gc;
    {
        t_gc.start();
        for (size_t i = 0; i < REPEAT; ++i) {
            auto res = good_value();
            if (res < 0) [[unlikely]]
                error_handler(res);
        }
        t_gc.stop();

        std::cout << t_gc.duration<benchmark::nanoseconds>() / REPEAT << " ns" << std::endl;
    }

    // bad call
    benchmark::timer t_bc;
    {
        t_bc.start();
        for (size_t i = 0; i < REPEAT; ++i) {
            auto res = good_value();
            if (res < 0) [[unlikely]]
                error_handler(res);
        }
        t_bc.stop();

        std::cout << t_bc.duration<benchmark::nanoseconds>() / REPEAT << " ns" << std::endl;
    }

    // exception
    benchmark::timer t_ex;
    {
        t_ex.start();
        for (size_t i = 0; i < REPEAT; ++i) {
            try {
                auto res [[maybe_unused]] = exception_value();
            } catch (int x) {
                error_handler(x);
            }
        }
        t_ex.stop();

        std::cout << t_ex.duration<benchmark::nanoseconds>() / REPEAT << " ns" << std::endl;
    }

    return 0;
}