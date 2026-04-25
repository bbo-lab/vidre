#ifndef VIDRE_BENCHMARK_HH
#define VIDRE_BENCHMARK_HH
#pragma once

#include <iostream>
#include <chrono>

#define kiB(x) (x * 1024UL)
#define MiB(x) (x * 1024UL * 1024UL)
#define GiB(x) (x * 1024UL * 1024UL * 1024UL)

namespace benchmark {
    using clock = std::chrono::steady_clock;
    using timepoint = clock::time_point;

    using nanoseconds = std::chrono::duration<double, std::nano>;
    using microseconds = std::chrono::duration<double, std::micro>;
    using milliseconds = std::chrono::duration<double, std::milli>;
    using seconds = std::chrono::duration<double>;

    struct timer {
        timer() = default;

        timepoint start_{};
        timepoint stop_{};

        void start() {
            start_ = clock::now();
        }
        void stop() {
            stop_ = clock::now();
        }

        template <class Duration = seconds>
        double elapsed() {
            return Duration(clock::now() - start_).count();
        }

        template <class Duration = seconds>
        double duration() {
            return Duration(stop_ - start_).count();
        }

        template <class Duration = seconds>
        double percall(uint64_t calls) {
            return Duration(stop_ - start_).count() / double(calls);
        }
    };

    template <typename T>
    void do_not_optimize(T const &val) {
        asm volatile("" : : "r,m"(val) : "memory");
    }

#define __benchmark_noinline__ __attribute__((noinline))

}  // namespace benchmark

#endif  // VIDRE_BENCHMARK_HH
