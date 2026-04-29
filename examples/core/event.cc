#include <vidre/util/events.hh>
#include <vidre/util/chrono.hh>
#include <vidre/thread.hh>
#include <vidre/log.hh>
#include <thread>

using namespace vidre;

#define RUN_FOR_S 10

static event ev{};

void consumer() {
    uint64_t count = 0;
    bool running = true;
    while (running) {
        event::result res = ev.wait(std::chrono::milliseconds(1));
        switch (res.status) {
            case event::state::success:
                count += res.count;
                break;
            case event::state::timed_out:
                continue;
            case event::state::terminated:
                running = false;
                continue;
            default:
                log::error() << "Consumer: failed to wait for event" << log::endl;
                running = false;
                continue;
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

        if (ev.notify(1) != event::state::success) {
            log::error() << "Producer: failed to notify event" << log::endl;
            break;
        }
        count++;
    }
    timer.stop();

    log::notice() << "Producer: sent " << count << " notifications" << log::endl
                  << "          (" << std::format("{:.3f} ns/notification", timer.duration<chrono::timer::nanoseconds>(count)) << ")" << log::endl;
    ev.terminate();
}

int main(int, const char **) {
    thread prod(producer);
    thread cons(consumer);

    prod.join();
    cons.join();

    return 0;
}