#include <vidre/io.hh>
#include <vidre/error.hh>
#include <vidre/io.hh>

#include <iostream>

using namespace vidre;

int main() {
    uint8_t data[9];

    {
        io::cache cache(16, 16);

        // cache handler task
        auto reader = task::make(
            [](shared<task> self, io::cache *cache) {
                task::scope scope(self);
                while (not scope.isterminated()) {
                    // wait for notification(s)
                    auto ev = cache->wait();

                    // handle errors
                    if (ev == status::bad) {
                        switch (ev.error()) {
                            case std::errc::broken_pipe:
                                break;
                            default:
                                std::cout << "Error condition: " << std::make_error_code(ev.error()).message() << std::endl;
                        }

                        break;
                    }

                    // process notifications (with delay)
                    for (event::value_type i = 0; i < *ev; ++i) {
                        cache->flush();
                        std::this_thread::sleep_for(std::chrono::milliseconds(140));
                    }
                }

                std::cout << "Reading data finished." << std::endl;
            },
            &cache);

        // write data to cache
        for (int i = 0; i < 32; ++i) {
            if (cache.put(data) < 9) {
                std::cerr << "Failed to write data." << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "Writing data finished." << std::endl;

        // wait for thread to exit
        std::cout << "Joining thread..." << std::endl;
        reader->join();
    }

    // done
    std::cout << "End." << std::endl;

    return 0;
};

/**@example examples/core/cache.cc
 * @details
 * This file demonstrates the use of the IO cache.
 *
 * This example covers the following topics:
 * - writing data to the cache,
 * - reading data from the cache, and,
 * - handling cache events.
 * */
