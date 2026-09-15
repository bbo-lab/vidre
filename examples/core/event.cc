#include <vidre/sync/events.hh>
#include <vidre/util/chrono.hh>
#include <vidre/thread.hh>
#include <vidre/log.hh>

using namespace vidre;

#define RUN_FOR_S 10

static event ev{};

void consumer() {
    uint64_t count = 0;
    bool running = true;
    while (running) {
        auto res = ev.wait(std::chrono::milliseconds(1));
        if (res == status::ok) {
            if (*res > 0)
                count += *res;
            else
                break;
        }
        else {
            log::error() << "Consumer: failed to wait for event (" << std::make_error_code(res.error()).message() << ")\n";
            break;
        }
    }

    log::notice() << "Consumer: received " << count << " events" << log::endl;
}

void producer() {
    uint64_t count = 0;
    chrono::timer timer;

    timer.start();
    while (true) {
        if (timer.elapsed<chrono::timer::seconds>() > RUN_FOR_S)
            break;

        if (ev.notify(1) != status::ok) {
            log::error() << "Producer: failed to notify event" << log::endl;
            break;
        }
        count++;
    }
    timer.stop();

    log::notice() << "Producer: sent " << count << " notifications" << log::endl
                  << "          (" << std::format("{:.3f} ns/notification", timer.duration<chrono::timer::nanoseconds>(count))
                  << ")" << log::endl;
}

int main(int, const char **) {
    log::notice() << "Running for " << RUN_FOR_S << " s...\n";

    thread prod(producer);
    thread cons(consumer);

    prod.join();
    cons.join();

    return 0;
}

/**@example examples/core/event.cc
 * @details
 * Measures the performance of event notifications between a producer and a consumer thread,
 * and also demonstrates usage of the `event` class.
 * */
