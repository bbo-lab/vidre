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
#ifndef VIDRE_CORE_THREAD_TASK_HH
#define VIDRE_CORE_THREAD_TASK_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/cpp/shared.hh>
#include <vidre/thread/types.hh>
#include <vidre/thread/schedule.hh>

#include <utility>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <mutex>

namespace vidre {

    /**@addtogroup api-core-thread-types
     * */
    ///@{

    /** A task (thread) class.
     *
     * This class provides an extension of the standard `std::thread` with some
     * additional convenience methods, such as setting scheduling policies and
     * managing the thread's running state.
     *
     * */
    class VIDRE_CORE_VISIBILITY task {
    public:
        /** Create a shared task instance.
         * @tparam F The type of the callable object (function, lambda, etc.) to be executed in the thread.
         *         @note The first argument of the callable object must be a `task::shared` pointer to the
         *               task instance itself which is automatically passed by the `make()` method. This
         *               allows the callable to manage its own lifetime and access the task's state.
         * @tparam Args The types of the arguments to be passed to the callable object.
         * @param f The callable object to be executed in the thread.
         * @param args The arguments to be passed to the callable object.
         * @returns A shared pointer to the created task instance.
         * */
        template <typename F, typename... Args>
        requires std::is_invocable_v<F, shared<task>, Args...>
        static shared<task> make(F &&f, Args &&...args) {
            auto task_ptr = std::make_shared<task>();

            task_ptr->impl_ = std::thread([task_ptr, f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable {
                f(task_ptr, args...);
            });

            return task_ptr;
        }

        /** Create a shared task instance (for member function).
         * @tparam Class The class type of the member function to be executed in the thread.
         * @tparam F The type of the member function to be executed in the thread.
         * @tparam Args The types of the arguments to be passed to the member function.
         * @param instance Pointer to the instance of the class on which the member function will be called.
         * @param f The member function to be executed in the thread.
         * @param args The arguments to be passed to the member function.
         * @returns A shared pointer to the created task instance.
         * @note The first argument of the member function must be a `task::shared` pointer to the task instance itself which is
         * automatically passed by the `make()` method. This allows the member function to manage its own lifetime and access the
         * task's state.
         * */
        template <typename Class, typename F, typename... Args>
        requires std::is_invocable_v<F, Class *, shared<task>, Args...>
        // @todo make this standard argument aware, maybe
        static shared<task> make(Class *instance, F &&f, Args &&...args) {
            return make([instance, f = std::forward<F>(f), ... args = std::forward<Args>(args)](shared<task> self) mutable {
                (instance->*f)(self, std::forward<Args>(args)...);
            });
        }

        /** Scoped task state.
         *
         * This helper struct is intended to be used within task handlers to
         * manage the task's running state and to provide a convenient method
         * for checking termination requests.
         *
         * @note The scope object does not manage life-time of the task but only
         *       the running state within its scope. It is recommened, therefor,
         *       (see example below) to use `shared<task>` when creating the
         *       scope object.
         *
         * Typical usage:
         * @code
         * void handler(shared<task> self, shared<task> receiver) {
         *     task::scope thr(self);
         *     while (not thr.isterminated()) {
         *         // do work
         *     }
         * }
         * @endcode
         *
         * */
        struct VIDRE_CORE_VISIBILITY scope {
            scope() = delete;

            /** Constructor signalling running state.
             * */
            scope(shared<task> t) : task_(t.get()) {
                if (task_)
                    task_->running_ = true;
            }

            /** Destructor ending running state.
             * */
            ~scope() {
                if (task_)
                    task_->running_ = false;
            }

            /** Check if thread should terminate.
             * */
            bool isterminated() const noexcept {
                return task_ && task_->terminate_;
            }

        protected:
            task *task_{};  ///< Pointer to the associated thread, or @c nullptr if not associated.
        };

        /** Type alias for a termination handler.
         *
         * This is a callable object that will be invoked when the thread is signaled to terminate.
         * */
        using terminate_handler = std::function<void()>;

    public:
        task() noexcept = default;

        task(const task &) = delete;
        task &operator=(const task &) = delete;
        task(task &&other) noexcept = delete;
        task &operator=(task &&other) noexcept = delete;

        /** Destructor.
         * The destructor automatically joins the underlying thread if necessary.
         * */
        virtual ~task() {
            if (impl_.joinable())
                impl_.join();
        }

    protected:
        template <class F, class... Args>
        explicit task(F &&f, Args &&...args) : impl_(std::forward<F>(f), std::forward<Args>(args)...) {}

    protected:
        std::thread impl_;                    ///< The underlying thread implementation.
        std::atomic_bool running_ = false;    ///< Flag indicating whether the thread is running.
        std::atomic_bool terminate_ = false;  ///< Flag indicating whether the thread should terminate.
        std::mutex terminate_mutex_{};        ///< Mutex to protect access to termination handlers.
        std::vector<terminate_handler>
            terminate_handlers_{};  ///< List of termination handlers to be invoked when the thread is signaled to terminate.

    public:
        /** Check if thread is running.
         * @returns @c true if the thread is running, @c false otherwise.
         * @note This method only checks the running flag and does not guarantee that the thread is actively executing.
         *       In addition, it does not provide any synchronization guarantees with respect to other threads.
         * */
        bool isrunning() const noexcept {
            return running_;
        }

        /** Check if thread is expected to be terminated.
         * @returns @c true if the thread is expected to be terminated, @c false otherwise.
         *
         * Typical usage is in the event loop:
         * @code
         * while (not isterminated()) {
         *     // Perform thread tasks here
         * }
         * @endcode
         *
         * @see task::terminate()
         * */
        bool isterminated() const noexcept {
            return terminate_;
        }

        /** Signal the thread to terminate.
         *
         * This method only sets the termination flag. It can be overridden in derived classes
         * to implement custom termination logic.
         *
         * @note Termination is cooperative. Derived implementations should ensure that
         *       blocking operations can be interrupted or woken when terminate() is called.
         *
         * @see task::isterminated()
         * @see task::on_terminate()
         * @see task::scope
         * */
        virtual void terminate() {
            std::vector<terminate_handler> handlers;

            {
                std::lock_guard lock(terminate_mutex_);

                if (terminate_)
                    return;

                terminate_ = true;
                handlers.swap(terminate_handlers_);
            }

            for (auto &handler : handlers)
                handler();
        }

        /** Register a termination handler.
         *
         * The @a handler will be invoked when the thread is signaled to terminate.
         *
         * This allows blocking operations or other task-specific activities to be notified of a termination request.
         * For example, `event::wait()` blocks indefinitely until the event is notified. The `event` class, however,
         * provides a `cancel()` method that can be used to interrupt the wait operation:
         * @code
         * shared<event> event_;
         * shared<task> t;
         * t->on_terminate([ev = event_]() {
         *     ev->cancel();
         * });
         * @endcode
         *
         * @param handler The termination handler to be registered.
         * */
        void on_terminate(terminate_handler handler) {
            {
                std::lock_guard lock(terminate_mutex_);

                if (not terminate_) {
                    terminate_handlers_.emplace_back(std::move(handler));
                    return;
                }
            }

            // Termination already requested.
            handler();
        }

        /** Join the thread.
         *
         * This method joins the thread, i.e., it executes and waits for the thread to finish execution.
         * @returns ``status::bad`` if the thread was not joinable, ``status::good`` if the thread was successfully joined.
         *
         * @note The destructor automatically joins the thread if it is still joinable.
         *
         * @see isjoinable()
         * @see detach()
         * */
        status join() {
            if (not impl_.joinable())
                return status::bad;
            impl_.join();
            return status::ok;
        }

        /** Check if task is joinable.
         * @returns @c true if the task is joinable, @c false otherwise.
         *
         * @see join()
         * @see detach()
         * */
        bool isjoinable() const noexcept {
            return impl_.joinable();
        }

        /** Detach the thread.
         * This method detaches the thread, allowing it to run independently.
         * @returns ``status::bad`` if the thread was not joinable, ``status::good`` if the thread was successfully detached.
         * @see join()
         * */
        status detach() noexcept {
            if (not impl_.joinable())
                return status::bad;
            impl_.detach();
            return status::ok;
        }

        /** Set scheduling priority.
         * @param priority Priority value as a float (0.0 to 1.0, means minimum to maximum priority).
         * @param policy Scheduling policy (default: FIFO).
         * @return ``status::bad`` if the scheduling operation failed, ``status::good`` otherwise.
         * */
        status priority(float priority = 0.5f, schedule::policy policy = schedule::policy::fifo) {
            return schedule(impl_).priority(priority, policy);
        }

        /** Set deadline scheduling.
         * @param period The period of the thread.
         * @param runtime The runtime of the thread.
         * @param deadline The deadline of the thread.
         * @return ``status::bad`` if the scheduling operation failed, ``status::good`` otherwise.
         * */
        status set_deadline(std::chrono::nanoseconds period,   //
                            std::chrono::nanoseconds runtime,  //
                            std::chrono::nanoseconds deadline) {
            return schedule(impl_).set_deadline(period, runtime, deadline);
        }
    };

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_THREAD_TASK_HH
