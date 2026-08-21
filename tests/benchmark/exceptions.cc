#include <vidre/exception.hh>
#include <vidre/util/chrono.hh>

using namespace vidre;

#include <iostream>

#define REPEAT (10000000UL)

volatile int good = 10;
volatile int bad = -1;

__vidre_noinline__ void error_handler(int ec [[maybe_unused]]) {
    //
}

__vidre_noinline__ int good_value() {
    return good;
}

__vidre_noinline__ int bad_value() {
    return bad;
}

__vidre_noinline__ int exception_value() {
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
    chrono::timer t_eh;
    {
        t_eh.start();
        for (size_t i = 0; i < REPEAT; ++i) {
            error_handler(bad);
        }
        t_eh.stop();

        std::cout << t_eh.duration<chrono::timer::nanoseconds>() / REPEAT << " ns" << std::endl;
    }

    // good call
    chrono::timer t_gc;
    {
        t_gc.start();
        for (size_t i = 0; i < REPEAT; ++i) {
            auto res = good_value();
            if (res < 0) [[unlikely]]
                error_handler(res);
        }
        t_gc.stop();

        std::cout << t_gc.duration<chrono::timer::nanoseconds>() / REPEAT << " ns" << std::endl;
    }

    // bad call
    chrono::timer t_bc;
    {
        t_bc.start();
        for (size_t i = 0; i < REPEAT; ++i) {
            auto res = good_value();
            if (res < 0) [[unlikely]]
                error_handler(res);
        }
        t_bc.stop();

        std::cout << t_bc.duration<chrono::timer::nanoseconds>() / REPEAT << " ns" << std::endl;
    }

    // exception
    chrono::timer t_ex;
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

        std::cout << t_ex.duration<chrono::timer::nanoseconds>() / REPEAT << " ns" << std::endl;
    }

    return 0;
}