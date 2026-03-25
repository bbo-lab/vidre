/* VidRe
 * Copyright (C) 2024-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_UTIL_SYSINFO_HH
#define VIDRE_CORE_UTIL_SYSINFO_HH
#pragma once

/**@defgroup api-core-util-sysinfo System information
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>

#include <filesystem>

namespace vidre {

    namespace sysinfo {

        /**@addtogroup api-core-util-sysinfo
         * */
        ///@{

        /** Physical system memory overview.
         *
         * <b>Example</b>:
         * @include examples/core/sysinfo.cc
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

        /** Get user-local configuration directory.
         * */
        std::filesystem::path configdir() noexcept;

        /** Get system temporary directory.
         * */
        std::filesystem::path tmpdir() noexcept;

        /** Get user-local cache directory.
         * */
        std::filesystem::path cachedir() noexcept;

        ///@}

    }  // namespace sysinfo

}  // namespace vidre

#endif  // VIDRE_CORE_UTIL_SYSINFO_HH
