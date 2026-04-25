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
#ifndef VIDRE_CORE_THREAD_TYPES_HH
#define VIDRE_CORE_THREAD_TYPES_HH
#pragma once

/**@addtogroup api-core-thread-types
 * @details
 * @fileinfo
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/exception.hh>
#include <vidre/util/status.hh>
#include <vidre/cpp/util.hh>

///@cond
#if defined(_WIN32)
#include <windows.h>
#define VIDRE_USE_THREAD_PTHREAD 0
#define VIDRE_USE_THREAD_WINAPI  1
#else
#define VIDRE_USE_THREAD_PTHREAD 1
#define VIDRE_USE_THREAD_WINAPI  0
#include <pthread.h>
#include <unistd.h>
#include <sys/resource.h>
#endif
///@endcond

#include <atomic>
#include <thread>
#include <utility>

namespace vidre {

    /**@addtogroup api-core-thread-types
     * */
    ///@{

#if defined(DOXYGEN)
    /** Indicates whether pthreads are used.
     * */
#define VIDRE_USE_THREAD_PTHREAD <0|1>

    /** Indicates whether Windows API threads are used.
     * */
#define VIDRE_USE_THREAD_WINAPI <0|1>
#endif

    /** Thread type.
     * */
    using thread = std::thread;

    ///@}

}  // namespace vidre

#endif  //  VIDRE_CORE_THREAD_TYPES_HH