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
#ifndef VIDRE_CORE_THREAD_SCHEDULE_HH
#define VIDRE_CORE_THREAD_SCHEDULE_HH
#pragma once

/**@addtogroup api-core-thread-schedule
 * @details
 * @fileinfo
 *
 * @todo link to "linux.md" 
 * */

#include <vidre/core/config.hh>
#include <vidre/thread/types.hh>

namespace vidre {

    namespace detail {

        /**@addtogroup api-core-thread-details
         * */
        ///@{

#if VIDRE_USE_THREAD_PTHREAD
        /** Scheduling settings.
         * */
        struct VIDRE_CORE_VISIBILITY schedule {
            /** Scheduling policies.
             * */
            enum policy_type : int {
                invalid = -1,              ///< Invalid scheduling policy.

                system = SCHED_OTHER,      ///< Default Linux time-sharing scheduling policy
                batch = SCHED_BATCH,       ///< Scheduling policy for "batch" style execution

                fifo = SCHED_FIFO,         ///< First-in, first-out real-time scheduling policy
                round_robin = SCHED_RR,    ///< Round-robin real-time scheduling policy

                idle = SCHED_IDLE,         ///< Scheduling policy for running a thread only when the system is idle
                deadline = SCHED_DEADLINE  ///< Scheduling policy for threads with specific deadlines
            };

            static constexpr policy_type default_policy = policy_type::system;

            schedule(thread &t) noexcept : handle(t.native_handle()) {}

            schedule() = delete;
            schedule(const schedule &) = delete;
            schedule &operator=(const schedule &) = delete;
            schedule(schedule &&) noexcept = delete;
            schedule &operator=(schedule &&) noexcept = delete;

            pthread_t handle{};

            /** Get minimum priority for a policy.
             * @param policy Scheduling policy.
             * @return Minimum priority value for the given policy.
             * */
            static int min_priority(policy_type policy_) noexcept {
                switch (policy_) {
                    case policy_type::system:
                    case policy_type::batch:
                        // For SCHED_OTHER/SCHED_BATCH, "priority" is really nice value
                        // Min priority = 19 (lowest priority, highest nice)
                        return 19;

                    case policy_type::fifo:
                    case policy_type::round_robin:
                        return sched_get_priority_min(static_cast<int>(policy_));

                    case policy_type::deadline:
                        // SCHED_DEADLINE uses runtime/deadline/period, not traditional priority
                        // Return 0 as placeholder - actual priority is determined by parameters
                        return 0;

                    case policy_type::idle:
                        // SCHED_IDLE has no priority values - always runs at lowest priority
                        return 0;

                    default:
                        return -1;
                }
            }

            /** Get maximum priority for a policy.
             * @param policy Scheduling policy.
             * @return Maximum priority value for the given policy.
             * */
            static int max_priority(policy_type policy_) noexcept {
                switch (policy_) {
                    case policy_type::system:
                    case policy_type::batch:
                        // For SCHED_OTHER/SCHED_BATCH, max priority = -20 (highest priority, lowest nice)
                        return -20;

                    case policy_type::fifo:
                    case policy_type::round_robin:
                        return sched_get_priority_max(static_cast<int>(policy_));

                    case policy_type::deadline:
                        // SCHED_DEADLINE returns 0 as placeholder - use sched_setattr for real config
                        return 0;

                    case policy_type::idle:
                        return 0;

                    default:
                        return -1;
                }
            }

            /** Get normalized priority for a policy.
             * @param normalized Normalized priority value (0.0 to 1.0).
             * @param policy Scheduling policy.
             * @return Priority value corresponding to the normalized input.
             * */
            static int normalized_priority(float normalized, policy_type policy_) noexcept {
                if (normalized < 0.0f)
                    normalized = 0.0f;
                if (normalized > 1.0f)
                    normalized = 1.0f;

                int min_prio = min_priority(policy_);
                int max_prio = max_priority(policy_);

                if (min_prio == max_prio)
                    return min_prio;

                switch (policy_) {
                    case policy_type::system:
                    case policy_type::batch:
                        // Nice values: lower = higher priority
                        // Normalized 1.0 (highest) -> max_prio (-20)
                        // Normalized 0.0 (lowest)  -> min_prio (19)
                        return static_cast<int>(max_prio - (normalized * (max_prio - min_prio)));

                    case policy_type::fifo:
                    case policy_type::round_robin:
                        // SCHED_FIFO/RR: higher = higher priority
                        // Normalized 1.0 -> max_prio
                        // Normalized 0.0 -> min_prio
                        return static_cast<int>(min_prio + (normalized * (max_prio - min_prio)));

                    default:
                        return min_prio;
                }
            }

            /** Set priority with specific policy.
             * @param policy Scheduling policy to set for the thread.
             * @param priority Priority value to set for the thread.
             * @return Status indicating success or failure.
             * */
            status priority(int priority, policy_type policy_ = default_policy) noexcept;

            /** Set deadline priority.
             * @param period The period of the thread (in nanoseconds).
             * @param runtime The runtime of the thread (in nanoseconds).
             * @param deadline The deadline of the thread (in nanoseconds).
             * @return Status indicating success or failure.
             *
             * @todo Explanation...
             * */
            status set_deadline(uint64_t period, uint64_t runtime, uint64_t deadline) noexcept {
                // FIXME https://chat.deepseek.com/a/chat/s/d4d5a090-2f26-468a-870f-1c2ec1934dea
                //
                // Period 1                Period 2                Period 3
                // t0          t+6ms      t+10ms     t+16ms     t+20ms
                // │           │          │          │          │
                // ▼           ▼          ▼          ▼          ▼
                // ├─────┬─────┼─────┬────┼─────┬────┼─────┬────┤
                // │     │     │     │    │     │    │     │    │
                // └──run┘     └──run┘    └──run┘    └──run┘
                //      │           │          │
                //      └─deadline  └─deadline └─deadline

                return status::bad;  
            }
        };
#elif VIDRE_USE_THREAD_WINAPI
        // Windows API implementation would go here 
        // FIXME
#else
        // Fallback implementation for unsupported platforms
        // FIXME
#endif

        ///@}

    }  // namespace detail

    /**@addtogroup api-core-thread-schedule
     * */
    ///@{

    /** Scheduling settings.
     *
     * Scheduling is a highly platform-specific topic, and the available policies
     * and priority levels can vary widely between different operating systems.
     * The `schedule` class provides a common interface for setting thread
     * scheduling policies and priorities, while the underlying implementation
     * in `detail::schedule` handles the platform-specific details.
     *
     * This interface is oriented towards POSIX-compliant systems, specifically
     * Linux, and may not be fully functional on all platforms. On other
     * platforms, a best-fit approach is taken, and some features may be unavailable
     * or behave differently.
     *
     * Even on Linux, the behavior of scheduling policies and priorities can be
     * influenced by system configuration, user permissions, and other factors.
     * For example, setting real-time priorities typically requires elevated
     * privileges. Also, the default system scheduling policy (SCHED_OTHER) does
     * not support explicit priority levels, therefore the "niceness" of the
     * process is adjusted instead, which is a different mechanism for influencing
     * scheduling behavior.
     *
     * */
    class VIDRE_CORE_VISIBILITY schedule {
    public:
        /** Scheduling policies.
         * */
        enum class policy : int {
            invalid = -1,  ///< Invalid scheduling policy.

            system = to_underlying(detail::schedule::system),  ///< Default system time-sharing scheduling policy.

            fifo = to_underlying(detail::schedule::fifo),                ///< First-in, first-out real-time scheduling policy.
            round_robin = to_underlying(detail::schedule::round_robin),  ///< Round-robin real-time scheduling policy.

            batch = to_underlying(detail::schedule::batch),  ///< Scheduling policy for "batch" style execution.
            idle = to_underlying(detail::schedule::idle),  ///< Scheduling policy for running a thread only when the system is idle.
            deadline = to_underlying(detail::schedule::deadline)  ///< Scheduling policy for threads with specific deadlines.
        };

        /** Default system scheduling policy.
         * */
        static constexpr policy default_policy = static_cast<policy>(detail::schedule::default_policy);

        schedule(thread &t) noexcept : impl(t) {}

        schedule() = delete;
        schedule(const schedule &) = delete;
        schedule &operator=(const schedule &) = delete;
        schedule(schedule &&) noexcept = delete;
        schedule &operator=(schedule &&) noexcept = delete;

    protected:
        detail::schedule impl;

    public:
        /** Get minimum priority for a policy.
         * @param policy Scheduling policy.
         * @return Minimum priority value for the given policy.
         * */
        int min_priority(policy policy_ = default_policy) const noexcept {
            return detail::schedule::min_priority(static_cast<detail::schedule::policy_type>(policy_));
        }

        /** Get maximum priority for a policy.
         * @param policy Scheduling policy.
         * @return Maximum priority value for the given policy.
         * */
        int max_priority(policy policy_ = default_policy) const noexcept {
            return detail::schedule::max_priority(static_cast<detail::schedule::policy_type>(policy_));
        }

        /** Set thread priority.
         * @param priority Priority value.
         * @param policy Scheduling policy.
         * @return Status of the operation.
         * */
        status priority(int priority, policy policy_ = policy::system) {
            return impl.priority(priority, static_cast<detail::schedule::policy_type>(policy_));
        }

        /** Set thread priority using a floating-point value.
         * @param priority Priority value as a float (0.0 to 1.0, means minimum to maximum priority).
         * @param policy Scheduling policy.
         * @return Status of the operation.
         * */
        status priority(float priority, policy policy_ = policy::system) {
            int prio = detail::schedule::normalized_priority(priority, static_cast<detail::schedule::policy_type>(policy_));
            return impl.priority(prio, static_cast<detail::schedule::policy_type>(policy_));
        }

        /** Set deadline priority.
         * @param period The period of the thread (in nanoseconds).
         * @param runtime The runtime of the thread (in nanoseconds).
         * @param deadline The deadline of the thread (in nanoseconds).
         * @return Status of the operation.
         * */
        status set_deadline(std::chrono::nanoseconds period, std::chrono::nanoseconds runtime, std::chrono::nanoseconds deadline) {
            return impl.set_deadline(period.count(), runtime.count(), deadline.count());
        }
    };

    ///@}

}  // namespace vidre

#endif  //  VIDRE_CORE_THREAD_SCHEDULE_HH
