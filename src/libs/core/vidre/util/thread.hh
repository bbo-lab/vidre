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
#ifndef VIDRE_CORE_UTIL_THREAD_HH
#define VIDRE_CORE_UTIL_THREAD_HH
#pragma once

/**@defgroup api-core-util-thread Threading utilities
 * @ingroup api-core-util
 * @details
 * @fileinfo
 *
 * @{
 * @defgroup api-core-util-thread-details Implementation details
 * @}
 * @todo see signalfd
 * */

#include <vidre/core/config.hh>
#include <vidre/exception.hh>
#include <vidre/log.hh>
#include <vidre/util/status.hh>

#if defined(_WIN32)
#include <windows.h>
#define VIDRE_USE_PTHREAD 0
#else
// FIXME should check if pthread is used.
#define VIDRE_USE_PTHREAD 1
#include <pthread.h>
#include <unistd.h>
#endif

#include <atomic>
#include <thread>
#include <utility>
#include <cerrno>
#include <cstring>

namespace vidre {

    /**@addtogroup api-core-util-thread
     * */
    ///@{

    using thread = std::thread;

    ///@}

    namespace detail {

        /**@addtogroup api-core-util-thread-details
         * */
        ///@{

#if VIDRE_USE_PTHREAD
        /** Scheduling settings.
         * */
        struct VIDRE_CORE_VISIBILITY schedule {
            /** Scheduling policies.
             * @todo need to find some commonality between Windows and Linux scheduling policies, or at least provide a common
             * interface.
             * */
            enum policy_type : int {
                system = SCHED_OTHER,      ///< Default Linux time-sharing scheduling policy
                fifo = SCHED_FIFO,         ///< First-in, first-out real-time scheduling policy
                round_robin = SCHED_RR,    ///< Round-robin real-time scheduling policy
                batch = SCHED_BATCH,       ///< Scheduling policy for "batch" style execution
                isochronous = SCHED_ISO,   ///< Scheduling policy for "isochronous" style execution
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
            status priority(int priority) {
                int policy;
                sched_param sch_params;

                // get current scheduling policy and parameters
                if (pthread_getschedparam(handle, &policy, &sch_params) != 0) {
                    log::debug() << "Failed to get thread priority: " << std::strerror(errno) << log::endl;
                    return status::bad;
                }

                auto min_prio = min_priority(static_cast<policy_type>(policy));
                auto max_prio = max_priority(static_cast<policy_type>(policy));

                if ((priority < min_prio) || (priority > max_prio)) {
                    log::debug() << "Invalid priority value: " << priority << ". Valid range for policy " << static_cast<int>(policy) << " is ["
                                 << min_prio << ", " << max_prio << "]" << log::endl;
                    return status::bad;
                }

                // set new priority
                sch_params.sched_priority = priority;
                if (pthread_setschedparam(handle, static_cast<int>(policy), &sch_params) != 0) {
                    log::debug() << "Failed to set thread priority: " << std::strerror(errno) << log::endl;
                    return status::bad;
                }

                return status::good;
            }

            status priority(policy_type policy, int priority) {
                auto min_prio = min_priority(static_cast<policy_type>(policy));
                auto max_prio = max_priority(static_cast<policy_type>(policy));

                if ((priority < min_prio) || (priority > max_prio)) {
                    log::debug() << "Invalid priority value: " << priority << ". Valid range for policy " << static_cast<int>(policy) << " is ["
                                 << min_prio << ", " << max_prio << "]" << log::endl;
                    return status::bad;
                }

                // set new priority
                sched_param sch_params;
                sch_params.sched_priority = priority;
                if (pthread_setschedparam(handle, static_cast<int>(policy), &sch_params) != 0) {
                    log::debug() << "Failed to set thread priority: " << std::strerror(errno) << log::endl;
                    return status::bad;
                }

                return status::good;
            }
        };
#else
        // FIXME
#endif

        ///@}

    }  // namespace detail

    /**@addtogroup api-core-util-thread
     * */
    ///@{

    /** Scheduling settings.
     * */
    class VIDRE_CORE_VISIBILITY schedule {
    public:
        /** Scheduling policies.
         * @todo need to find some commonality between Windows and Linux scheduling policies, or at least provide a common
         * interface.
         * */
        enum class policy : int {
            system = to_underlying(detail::schedule::system),            ///< Default Linux time-sharing scheduling policy
            fifo = to_underlying(detail::schedule::fifo),                ///< First-in, first-out real-time scheduling policy
            round_robin = to_underlying(detail::schedule::round_robin),  ///< Round-robin real-time scheduling policy
            batch = to_underlying(detail::schedule::batch),              ///< Scheduling policy for "batch" style execution
            isochronous = to_underlying(detail::schedule::isochronous),  ///< Scheduling policy for "isochronous" style execution
            idle = to_underlying(detail::schedule::idle),  ///< Scheduling policy for running a thread only when the system is idle
            deadline = to_underlying(detail::schedule::deadline)  ///< Scheduling policy for threads with specific deadlines
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
        status priority(int priority) {
            return impl.priority(priority);
        }

        status priority(policy policy_, int priority) {
            return impl.priority(static_cast<detail::schedule::policy_type>(policy_), priority);
        }
    };

    ///@}

}  // namespace vidre

#endif  //  VIDRE_CORE_UTIL_THREAD_HH