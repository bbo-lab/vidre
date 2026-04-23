/* VidRe
 * Copyright (C) 2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
 * Author(s):
 *      Jürgen "George" Sawinski¹
 *
 * Affiliations:
 * ¹ MPI f. Neurobiol. of Behavior — caesar, Bonn, Germany
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_UTIL_EVENTS_HH
#define VIDRE_CORE_UTIL_EVENTS_HH
#pragma once

/**@defgroup api-core-util-events Event handling
 * @ingroup api-core-util
 * @details
 * @fileinfo
 *
 * @{
 * @defgroup api-core-util-events-details Implementation details
 * @}
 * @todo see signalfd
 * */

#include <vidre/core/config.hh>
#include <vidre/exception.hh>
#include <vidre/util/status.hh>

#if defined(__linux__)
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#elif defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <atomic>
#include <chrono>
#include <thread>
#include <utility>

#include <cassert>

namespace vidre {

    namespace detail {

        /**@addtogroup api-core-util-events-details
         * */
        ///@{

        /** Trivial event notification primitive based on Linux's eventfd.
         * @note This is only available on Linux, and is emulated on other platforms.
         * */
        struct VIDRE_CORE_VISIBILITY event {
            using value_type = uint64_t;  ///< Type of the notification count.

            static constexpr value_type max_count = 0xfffffffffffffffeULL;  ///< Maximum notification count.

            // clang-format off
            static constexpr int success = static_cast<int>(std::errc{});                                    ///< Notification was successful.
            static constexpr int try_again = static_cast<int>(std::errc::resource_unavailable_try_again);    ///< Notification failed and should be retried.
            static constexpr int timed_out = static_cast<int>(std::errc::timed_out);                         ///< Wait timed out.
            static constexpr int terminated = static_cast<int>(std::errc::broken_pipe);                      ///< Termination signal received (not handled here).
            // clang-format on

            using result_type = std::pair<int, value_type>;  ///< Type of the wait result (status code and notification count).

            event() = default;
            event(const event &) = delete;
            event &operator=(const event &) = delete;
            event(event &&other) noexcept : handle(std::exchange(other.handle, -1)), quit(other.quit.load()) {}
            event &operator=(event &&other) noexcept {
                if (this != &other) {
                    close();
                    handle = std::exchange(other.handle, -1);
                    quit = other.quit.load();
                }
                return *this;
            }

#if defined(__linux__) or defined(DOXYGEN)
            /** Construct event handler.
             * @param initval The initial value of the event counter (default: 0).
             * @throw @c exception::runtime with @c error::system information if the event handler could not be created.
             * */
            event(value_type initval) {
                open(initval);
            }

            ~event() {
                if (valid())
                    ::close(handle);
            }

            int handle = -1;                ///< Internal handle (e.g. file descriptor on Linux).
            std::atomic<bool> quit{false};  ///< Flag to indicate termination.

            /** Check if valid.
             * */
            constexpr bool valid() const noexcept {
                return handle != -1;
            }

            /** Initialize event handler.
             * */
            void open(value_type initval) {
                handle = ::eventfd(initval, EFD_NONBLOCK | EFD_CLOEXEC);
                if (handle == -1)
                    throw exception::runtime() << error::system() << error::message("failed to create eventfd");
            }

            /** Close the event handler.
             * */
            void close() noexcept {
                if (valid()) {
                    ::close(handle);
                    handle = -1;
                }
            }

            /** Notify receiver of the "terminate" condition.
             * @param time The time to wait for the consumer to acknowledge the
             *             termination signal (milliseconds, default: 100ms).
             * */
            void terminate(std::chrono::milliseconds timeout) {
                quit = true;

                // clear the eventfd counter to block any waiter
                value_type count;
                read(handle, &count, sizeof(count));

                // wake up any waiter to let them check the termination flag
                if (notify(max_count) != success)
                    goto termination_signal_not_acknowledged;

                // wait until we can write again (i.e. the waiter has acknowledged the termination signal)
                {
                    struct pollfd pfd = {handle, POLLOUT, 0};
                    int ret = poll(&pfd, 1, static_cast<int>(timeout.count()));
                    if (ret > 0)
                        return;  // acknowledged
                }

            termination_signal_not_acknowledged:
                throw vidre::exception::runtime() << error::system() << error::message("failed to notify termination");
            }

            /** Notify the event, i.e. wake up the consumer.
             * @param count The number of notifications to send (default: 1).
             * @return Status indicating whether the notification was successful
             *         (i.e. count notifications were sent) or not (i.e. the counter is full).
             * @throw @c exception::runtime with @c error::system information if the notification
             *        failed due to a system error.
             * @note This call fails if @c max_count would overflow.
             * */
            int notify(value_type count = 1) {
                ssize_t ret = write(handle, &count, sizeof(count));
                if (ret != sizeof(count)) {
                    switch (errno) {
                        case EAGAIN:
                            return try_again;
                        default:
                            throw vidre::exception::runtime() << error::system();
                    }
                }
                return success;
            }

            /** Wait for the event to be notified.
             * @return The result of the wait operation, including the status and the notification count.
             * */
            result_type wait() {
                value_type count;
                ssize_t ret = read(handle, &count, sizeof(count));
                while ((ret != sizeof(count)) and (errno == EAGAIN)) {
                    if (quit)
                        return {terminated, 0};
                    std::this_thread::yield();
                    ret = read(handle, &count, sizeof(count));
                }
                if (ret != sizeof(count)) {
                    switch (errno) {
                        case EAGAIN:
                            return {try_again, 0};
                        default:
                            throw vidre::exception::runtime() << error::system();
                    }
                }
                return {quit ? terminated : success, count};
            }

            /** Wait for the event to be notified, with a timeout.
             * @param timeout The maximum time to wait for the event.
             * @return The number of notifications received, or std::nullopt if the wait timed out.
             * */
            result_type wait(std::chrono::milliseconds timeout) {
                struct pollfd pfd = {handle, POLLIN, 0};
                int ret = poll(&pfd, 1, static_cast<int>(timeout.count()));
                if (ret == 0)
                    return {timed_out, 0};  // timed out
                else if (ret < 0) [[unlikely]] {
                    throw vidre::exception::runtime() << error::system();
                }
                return wait();
            }

#elif defined(__APPLE__)
            FIXME
#elif defined(_WIN32)
            FIXME
#endif
        };

        ///@}

    }  // namespace detail

    /**@addtogroup api-core-util-events
     * */
    ///@{

    /** Event signalling primitive.
     *
     * This class provides a simple mechanism for threads to signal events to
     * each other using the Linux eventfd mechanism (emulated on other platforms).
     * It allows one thread (the "producer") to notify another thread (the "consumer")
     * about the occurrence of an event. The consumer will receive the notification
     * count, which indicates how many times the event was signaled since the last wait.
     *
     * @note This class is designed to be used in a single-producer, single-consumer
     *       scenario. It does not support multiple producers or consumers, and it
     *       does not provide any synchronization mechanisms for concurrent access.
     *
     *       Also, the object of this class should be shared between the producer
     *       and consumer threads (e.g. via a shared pointer).
     *
     * */
    class VIDRE_CORE_VISIBILITY event {
    public:
        using value_type = detail::event::value_type;  ///< Type of the notification count.

        /** Notification state.
         * */
        enum state : int {
            success = detail::event::success,       ///< Notification was successful.
            error = detail::event::try_again,       ///< Notification failed and should be retried.
            timed_out = detail::event::timed_out,   ///< Wait timed out.
            terminated = detail::event::terminated  ///< Termination signal received (not handled here).
        };

        /** Wait result.
         * */
        struct VIDRE_CORE_VISIBILITY result {
            state status;
            value_type count{};

            operator bool() const noexcept {
                return status == state::success;
            }

            bool operator!() const noexcept {
                return status != state::success;
            }

            bool operator==(state s) const noexcept {
                return status == s;
            }

            template <class T>
            requires std::is_unsigned_v<T>
            bool operator==(T v) const noexcept {
                return (status == state::success) and (count == static_cast<value_type>(v));
            }
        };

    public:
        event() = default;

        event(const event &) = delete;
        event &operator=(const event &) = delete;
        event(event &&) noexcept = default;
        event &operator=(event &&) noexcept = default;

        ~event() {}

    protected:
        detail::event impl_{0};  ///< Platform-specific event implementation.

    public:
        /** Notify receiver of the "terminate" condition.
         * @param time The time to wait for the consumer to acknowledge the
         *             termination signal (milliseconds, default: 100ms).
         * @throw @c exception::runtime with @c error::system information if the termination signal could not be sent.
         * */
        void terminate(std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) {
            impl_.terminate(timeout);
        }

        /** Notify the event, i.e. wake up any waiter.
         * @param count The number of notifications to send.
         * @return Status indicating whether the notification was successful
         *         (i.e. count notifications were sent) or not (i.e. the counter is full).
         * @throw @c exception::runtime with @c error::system information if the notification
         *        failed due to a system error.
         * @note This call fails if @c max_count would overflow.
         * */
        state notify(uint64_t count = 1) {
            assert(count > 0);
            return static_cast<state>(impl_.notify(count));
        }

        /** Wait for the event to be notified.
         * @return @c result containing the status and the notification count.
         * @throw @c exception::runtime with @c error::system information if the wait operation failed due to a system error.
         * This method blocks indefinitely until the event is notified or a
         * termination signal is received.
         * */
        result wait() {
            auto retval = impl_.wait();
            return {static_cast<state>(retval.first), retval.second};
        }

        /** Wait for the event to be notified, with a timeout.
         * @param timeout The maximum time to wait for the event.
         * @throw @c exception::runtime with @c error::system information if the wait operation failed due to a system error.
         * @return @c result containing the status and the notification count.
         * 
         * Example:
         * @code
         * bool running = true;
         * while (running) {
         *      event::result res = ev.wait(std::chrono::milliseconds(100));
         *      switch (res.status) {
         *         case event::state::success:
         *             // handle event, res.count contains the notification count
         *             break;
         *         case event::state::timed_out:
         *             // handle timeout
         *             continue;
         *         case event::state::terminated:
         *             running = false;  // handle termination
         *             continue;
         *         default:
         *             // never reached (exceptions are thrown otherwise)
         *             break;
         *      }
         * }
         * @endcode 
         * */
        result wait(std::chrono::milliseconds timeout) {
            auto retval = impl_.wait(timeout);
            return {static_cast<state>(retval.first), retval.second};
        }
    };

    ///@}

}  // namespace vidre

#endif  //  VIDRE_CORE_UTIL_EVENTS_HH