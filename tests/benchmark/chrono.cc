/*
Measure the speed of the monotonic clock and the time taken to perform a large
number of iterations of a simple operation (e.g., incrementing a counter) to
evaluate the performance of the chrono library in C++.
*/

#include <vidre/util/chrono.hh>
#include <vidre/log.hh>
#include <vidre/util/debug.hh>

#define REPEAT 10'000'000
using namespace vidre;

int main(int argc, const char **argv) {
    chrono::timer t;

    t.start();
    for (int i = 0; i < REPEAT; ++i) {
        chrono::monotonic::time_point now = chrono::monotonic::now();
        debug::do_not_optimize(now);
    }
    t.stop();
    double duration = t.duration<chrono::timer::nanoseconds>() / double(REPEAT);
    log::notice() << "monotonic clock: " << duration << " ns/call" << log::endl;

    return 0;
}
