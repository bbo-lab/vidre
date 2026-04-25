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
#include "schedule.hh"

#include "../log.hh"
#include <cstring>
#include <cerrno>

namespace vidre::detail {

#if VIDRE_USE_THREAD_PTHREAD

    status schedule::priority(int priority) noexcept {
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
            log::debug() << "Invalid priority value: " << priority << ". Valid range for policy " << static_cast<int>(policy)
                         << " is [" << min_prio << ", " << max_prio << "]" << log::endl;
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

    status schedule::priority(policy_type policy, int priority) noexcept {
        auto min_prio = min_priority(static_cast<policy_type>(policy));
        auto max_prio = max_priority(static_cast<policy_type>(policy));

        if ((priority < min_prio) || (priority > max_prio)) {
            log::debug() << "Invalid priority value: " << priority << ". Valid range for policy " << static_cast<int>(policy)
                         << " is [" << min_prio << ", " << max_prio << "]" << log::endl;
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
#endif

}  // namespace vidre::detail