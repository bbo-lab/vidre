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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_SYNC_EVENTS_HH
#define VIDRE_CORE_SYNC_EVENTS_HH
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
#include <vidre/error.hh>

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

#if not defined(__linux__)
#include <condition_variable>
#include <mutex>
#endif

#include <chrono>
#include <atomic>
#include <thread>
#include <cassert>

namespace vidre {

    namespace detail {

        /**@addtogroup api-core-util-events-details
         * */
        ///@{

        /** Trivial event notification primitive.
         *
         * @note This struct, under Linux, abstracts the eventfd mechanism. On
         *       other platforms, the behavior is simulated.
         *
         * @todo cancel() may block forever if the notifier is allowed to send signals...
         * */
        struct VIDRE_CORE_VISIBILITY event {
            using value_type = uint64_t;             ///< Type of the notification count.
            using result_type = result<value_type>;  ///< Type of the wait result (status code and notification count).

            static constexpr value_type max_count = 0xfffffffffffffffeULL;  ///< Maximum notification count.

            event() = default;
            event(const event &) = delete;
            event &operator=(const event &) = delete;
            event(event &&) = delete;
            event &operator=(event &&) = delete;

#if defined(__linux__) or defined(DOXYGEN)
            /** Construct event handler.
             * @param initval The initial value of the event counter (default: 0).
             * @throw @c exception::runtime with @c error::system information if the event handler could not be created.
             * */
            event(value_type initval) {
                open();
                if (auto result = notify(initval); result == status::bad) {
                    close();
                    throw exception::runtime() << error::message("failed to initialize event handler");
                }
            }

            ~event() {
                if (valid())
                    ::close(handle);
            }

            int handle = -1;                      ///< Internal handle (e.g. file descriptor on Linux).
            std::atomic_bool cancelled_ = false;  ///< Flag indicating whether the event has been cancelled.

            /** Check if valid.
             * */
            constexpr bool valid() const noexcept {
                return handle != -1;
            }

            /** Initialize event handler.
             * */
            void open() {
                handle = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
                if (handle == -1)
                    throw exception::runtime()                               //
                        << error::message("failed to create event handler")  //
                        << error::system();
            }

            /** Close the event handler.
             * */
            void close() noexcept {
                if (valid()) {
                    ::close(handle);
                    handle = -1;
                }
            }

            /** Check if the event queue is empty.
             * @returns `true` if the event queue is empty, `false` otherwise.
             * */
            bool empty() const noexcept {
                struct pollfd pfd = {handle, POLLIN, 0};
                int ret = ::poll(&pfd, 1, 0);

                if (ret <= 0)
                    return true;

                return !(pfd.revents & POLLIN);
            }

            /** Wait for the event to be notified.
             * @returns notification count or error.
             * */
            result_type read() {
                value_type count;
                ssize_t ret = ::read(handle, &count, sizeof(count));

                if (ret != sizeof(count))
                    return unexpected(static_cast<std::errc>(errno));
                else if (cancelled_.load(std::memory_order_acquire))
                    return unexpected(std::errc::operation_canceled);
                else
                    return count;
            }

            // API required:

            /** Cancel the event.
             *
             * This function waits until the event queue is empty before cancelling.
             * Any current waiter is woken and current or future calls to wait()
             * return `std::errc::operation_canceled`.
             *
             * @note Cancellation is persistent.
             */
            void cancel() noexcept {
                // wait for queue to be empty (if necessary) before cancelling
                while (!empty())
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));

                // set cancelled flag and notify any waiters
                if (cancelled_.exchange(true, std::memory_order_acq_rel))
                    return;

                notify();
            }

            /** Reset event to its initial state.
             * */
            void reset() noexcept {
                if (!empty())
                    ::read(handle, nullptr, 0);

                cancelled_.store(false, std::memory_order_release);
            }

            /** Notify the event, i.e. wake up the consumer.
             * @param count The number of notifications to send (default: 1).
             * @return The @a count or error information.
             * @note This call fails if @c max_count would overflow.
             * */
            result_type notify(value_type count = 1) {
                ssize_t ret = ::write(handle, &count, sizeof(count));
                if (ret != sizeof(count))
                    return unexpected(static_cast<std::errc>(errno));
                else
                    return count;
            }

            /** Wait for the event to be notified.
             * @returns notification count or error.
             * */
            result_type wait() {
                return wait(std::chrono::milliseconds(-1));
            }

            /** Wait for the event to be notified, with a timeout.
             * @param timeout The maximum time to wait for the event.
             * @return The notification count or error information. If the timeout expires before a notification is received,
             * returns 0.
             * */
            result_type wait(std::chrono::milliseconds timeout) {
                struct pollfd pfd = {handle, POLLIN, 0};
                int ret = ::poll(&pfd, 1, static_cast<int>(timeout.count()));

                if (ret < 0)
                    return unexpected(static_cast<std::errc>(errno));
                else if (ret == 0)
                    return value_type(0);

                if (pfd.revents & POLLIN)
                    return read();

                if (pfd.revents & POLLNVAL)
                    return unexpected(std::errc::bad_file_descriptor);

                return unexpected(std::errc::io_error);
            }

#else

            event(value_type initval) : count_(initval) {
                if (initval > max_count)
                    throw exception::logic() << "invalid initial event count";
            }

            ~event() = default;

        protected:
            std::mutex mutex_;
            std::condition_variable condition_;
            value_type count_ = 0;
            bool cancelled_ = false;

        public:
            result_type notify(value_type count = 1) {
                {
                    std::lock_guard lock(mutex_);

                    if (cancelled_)
                        return unexpected(std::errc::operation_canceled);

                    if (count > max_count - count_)
                        return unexpected(std::errc::value_too_large);

                    count_ += count;
                }

                condition_.notify_one();
                return count;
            }

            result_type wait() {
                std::unique_lock lock(mutex_);

                condition_.wait(lock, [&] {
                    return count_ != 0 || cancelled_;
                });

                if (cancelled_)
                    return unexpected(std::errc::operation_canceled);

                value_type count = count_;
                count_ = 0;

                condition_.notify_all();  // important for cancel()
                return count;
            }

            result_type wait(std::chrono::milliseconds timeout) {
                std::unique_lock lock(mutex_);

                if (!condition_.wait_for(lock, timeout, [&] {
                        return count_ != 0 || cancelled_;
                    }))
                    return value_type{0};

                if (cancelled_)
                    return unexpected(std::errc::operation_canceled);

                value_type count = count_;
                count_ = 0;

                condition_.notify_all();  // wake cancel() waiting for count_ == 0
                return count;
            }

            void cancel() noexcept {
                std::unique_lock lock(mutex_);

                condition_.wait(lock, [&] {
                    return count_ == 0;
                });

                if (cancelled_)
                    return;

                cancelled_ = true;

                lock.unlock();
                condition_.notify_all();
            }
#endif
        };

        ///@}

    }  // namespace detail

    /**@addtogroup api-core-thread-synchronization
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
     * @note This class is designed to be used in for single-producer, single-consumer
     *       use, otherwise the behavior is undefined.
     *
     * @see @ref examples/core/event.cc
     * */
    class VIDRE_CORE_VISIBILITY event {
    public:
#if defined(DOXYGEN)
        using value_type = uint64_t;             ///< Type of the notification count.
        using result_type = result<value_type>;  ///< Type of the wait result (status code and notification count).
#else
        using value_type = detail::event::value_type;
        using result_type = detail::event::result_type;
#endif

        static constexpr value_type max_count = detail::event::max_count;  ///< Maximum notification count.

    public:
        event() = default;

        event(const event &) = delete;
        event &operator=(const event &) = delete;
        event(event &&) noexcept = delete;
        event &operator=(event &&) noexcept = delete;

    protected:
        detail::event impl_{0};  ///< Platform-specific event implementation.

    public:
        /** Increment event counter and wakeup the waiter.
         * @param count The number of notifications to send.
         * @return Status indicating whether the notification was successful
         *         (i.e. count notifications were sent) or not (i.e. the counter is full).
         * */
        result_type notify(uint64_t count = 1) {
            return impl_.notify(count);
        }

        /** Cancel the event.
         *
         * This function waits until the event queue is empty before cancelling.
         * Any current waiter is woken and current or future calls to wait()
         * return `std::errc::operation_canceled`.
         *
         * @note Cancellation is persistent.
         */
        void cancel() noexcept {
            impl_.cancel();
        }

        /** Reset event to its initial state.
         * */
        void reset() noexcept {
            impl_.reset();
        }

        /** Wait for the event to be notified.
         * @return the result containing either an error or the notification count.
         * */
        [[nodiscard]] result_type wait() {
            return impl_.wait();
        }

        /** Wait for the event to be notified, with a timeout.
         * @param timeout The maximum time to wait for the event (may be zero for non-blocking wait).
         * @return the result containing either an error or the notification count.
         *
         * @note The behavior for a negative @a timeout is unspecified.
         *
         * @see `wait()`
         * */
        [[nodiscard]] result_type wait(std::chrono::milliseconds timeout) {
            return impl_.wait(timeout);
        }
    };

    ///@}

}  // namespace vidre

#endif  //  VIDRE_CORE_SYNC_EVENTS_HH