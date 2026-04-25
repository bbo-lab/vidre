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
#ifndef VIDRE_CORE_THREAD_SCHEDULE_HH
#define VIDRE_CORE_THREAD_SCHEDULE_HH
#pragma once

/**@addtogroup api-core-thread-schedule
 * @details
 * @fileinfo
 *
 * @todo see signalfd
 * */

#include <vidre/core/config.hh>
#include <vidre/thread/types.hh>

namespace vidre {

    namespace detail {

        /**@addtogroup api-core-util-thread-details
         * */
        ///@{

#if VIDRE_USE_THREAD_PTHREAD
        /** Scheduling settings.
         * */
        struct VIDRE_CORE_VISIBILITY schedule {
            /** Scheduling policies.
             * @todo need to find some commonality between Windows and Linux scheduling policies, or at least provide a common
             * interface.
             * */
            enum policy_type : int {
                invalid = -1,              ///< Invalid scheduling policy.
                system = SCHED_OTHER,      ///< Default Linux time-sharing scheduling policy
                fifo = SCHED_FIFO,         ///< First-in, first-out real-time scheduling policy
                round_robin = SCHED_RR,    ///< Round-robin real-time scheduling policy
                batch = SCHED_BATCH,       ///< Scheduling policy for "batch" style execution
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
            static int min_priority(policy_type policy) {
                return sched_get_priority_min(static_cast<int>(policy));
            }

            /** Get maximum priority for a policy.
             * @param policy Scheduling policy.
             * @return Maximum priority value for the given policy.
             * */
            static int max_priority(policy_type policy) {
                return sched_get_priority_max(static_cast<int>(policy));
            }

            /** Set priority.
             * @param priority Priority value to set for the thread.
             * */
            status priority(int priority) noexcept;

            /** Set priority with specific policy.
             * @param policy Scheduling policy to set for the thread.
             * @param priority Priority value to set for the thread.
             * @return Status indicating success or failure.
             * */
            status priority(policy_type policy, int priority) noexcept;
        };
#else
        // FIXME
#endif

        ///@}

    }  // namespace detail

#if 0
    /**@addtogroup api-core-util-thread
     * */
    ///@{

    // @todo This needs quite a bit of work to make it look cross-platform...

    // /** Scheduling settings.
    //  *
    //  * Scheduling is a highly platform-specific topic, and the available policies
    //  * and priority levels can vary widely between different operating systems.
    //  * The `schedule` class provides a common interface for setting thread
    //  * scheduling policies and priorities, while the underlying implementation
    //  * in `detail::schedule` handles the platform-specific details.
    //  *
    //  * */
    // class VIDRE_CORE_VISIBILITY schedule {
    // public:
    //     /** Scheduling policies.
    //      * @todo need to find some commonality between Windows and Linux scheduling policies, or at least provide a common
    //      * interface.
    //      * */
    //     enum class policy : int {
    //         invalid = -1,                                                ///< Invalid scheduling policy.
    //         system = to_underlying(detail::schedule::system),            ///< Default Linux time-sharing scheduling policy.
    //         fifo = to_underlying(detail::schedule::fifo),                ///< First-in, first-out real-time scheduling policy.
    //         round_robin = to_underlying(detail::schedule::round_robin),  ///< Round-robin real-time scheduling policy.
    //         batch = to_underlying(detail::schedule::batch),              ///< Scheduling policy for "batch" style execution.
    //         idle = to_underlying(detail::schedule::idle),  ///< Scheduling policy for running a thread only when the system is idle.
    //         deadline = to_underlying(detail::schedule::deadline)  ///< Scheduling policy for threads with specific deadlines.
    //     };

    //     /** Default system scheduling policy.
    //      * */
    //     static constexpr policy default_policy = static_cast<policy>(detail::schedule::default_policy);

    //     schedule(thread &t) noexcept : impl(t) {}

    //     schedule() = delete;
    //     schedule(const schedule &) = delete;
    //     schedule &operator=(const schedule &) = delete;
    //     schedule(schedule &&) noexcept = delete;
    //     schedule &operator=(schedule &&) noexcept = delete;

    // protected:
    //     detail::schedule impl;

    // public:
    //     status priority(int priority) {
    //         return impl.priority(priority);
    //     }

    //     status priority(policy policy_, int priority) {
    //         return impl.priority(static_cast<detail::schedule::policy_type>(policy_), priority);
    //     }
    // };

    ///@}

#endif

}  // namespace vidre

#endif  //  VIDRE_CORE_THREAD_SCHEDULE_HH