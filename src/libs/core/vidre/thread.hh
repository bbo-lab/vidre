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
#ifndef VIDRE_CORE_UTIL_THREAD_HH
#define VIDRE_CORE_UTIL_THREAD_HH
#pragma once

/**@addtogroup api-core-thread
 * @details
 * @fileinfo
 * 
 * VidRe in general uses the C++ standard library's `std::thread` for threading.
 * Here, we provide some utilities for working with threads, such as setting
 * thread scheduling policies and priorities if supported by the underlying platform.
 *
 * @{
 * @defgroup api-core-thread-types Types
 * @defgroup api-core-thread-schedule Scheduling
 * @defgroup api-core-thread-synchronization Synchronization
 * @defgroup api-core-thread-details Implementation details
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/thread/types.hh>
#include <vidre/thread/task.hh>
#include <vidre/thread/schedule.hh>

#endif  //  VIDRE_CORE_UTIL_THREAD_HH
