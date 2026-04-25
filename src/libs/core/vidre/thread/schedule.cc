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

    status schedule::priority(int priority, policy_type policy_) noexcept {
        if (handle == 0)
            return status::bad;

        struct sched_param param = {};
        param.sched_priority = 0;

        switch (policy_) {
            case policy_type::system:
            case policy_type::batch:
                // For SCHED_OTHER/SCHED_BATCH, priority value is actually the nice level
                // Convert: our priority value is already the desired nice level (-20 to 19)
                if (handle != pthread_self()) {
                    log::debug() << "Warning: nice value can only be set for the current thread.";
                    return status::bad;
                }

                if (priority < -20)
                    priority = -20;
                if (priority > 19)
                    priority = 19;

                if (setpriority(PRIO_PROCESS, syscall(SYS_gettid), priority) != 0) {
                    log::debug() << "Failed to set nice value: " << std::strerror(errno);
                }

                // Also set the scheduling policy itself
                if (pthread_setschedparam(handle, policy_, &param) != 0) {
                    log::debug() << "Failed to set scheduling policy: " << std::strerror(errno);
                    return status::bad;
                }
                break;

            case policy_type::fifo:
            case policy_type::round_robin: {
                int min_prio = sched_get_priority_min(policy_);
                int max_prio = sched_get_priority_max(policy_);

                if (priority < min_prio)
                    priority = min_prio;
                if (priority > max_prio)
                    priority = max_prio;

                param.sched_priority = priority;

                if (pthread_setschedparam(handle, policy_, &param) != 0) {
                    return status::bad;
                }
            } break;

            case policy_type::idle:
                // SCHED_IDLE doesn't use priority values
                if (pthread_setschedparam(handle, policy_, &param) != 0) {
                    return status::bad;
                }
                break;

            default:
                return status::bad;
        }

        return status::good;
    }
#endif

}  // namespace vidre::detail