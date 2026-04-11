/* VidRe
 * Copyright (C) 2025-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_OS_MEMORY_HH
#define VIDRE_CORE_OS_MEMORY_HH
#pragma once

/**@defgroup api-core-os-memory System memory
 * @ingroup api-core-os
 * @details
 * @fileinfo
 *
 * */

#include <vidre/core/config.hh>

#if defined(_WIN32  ) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

namespace vidre {

    namespace os {

        /**@addtogroup api-core-os-memory
         * */
        ///@{

        /** Physical system memory overview.
         *
         * <b>Example</b>:
         * @include examples/core/os.cc
         *
         * */
        struct VIDRE_CORE_VISIBILITY memory {
            std::size_t total = 0;      ///< Total physical system memory.
            std::size_t available = 0;  ///< Memory size available for allocation.
            std::size_t used = 0;       ///< Used memory (including buffers).

            /** Get system memory info.
             * */
            static memory get() noexcept;
        };

        /** Get system page size.
         * */
        inline std::size_t pagesize() noexcept {
#if defined(_SC_PAGESIZE)
            auto retval = sysconf(_SC_PAGESIZE);
            return retval;
#elif defined(_WIN32)
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            return si.dwPageSize;
#else
            return 0;  // Unknown platform
#endif
        }

        ///@}

    }  // namespace os

}  // namespace vidre

#endif  // VIDRE_CORE_OS_MEMORY_HH