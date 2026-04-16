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
#ifndef VIDRE_CORE_OS_PATHS_HH
#define VIDRE_CORE_OS_PATHS_HH
#pragma once

/**@defgroup api-core-os-path Paths
 * @ingroup api-core-os
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>

#include <filesystem>

namespace vidre {

    namespace os {

        /**@addtogroup api-core-os-path
         * */
        ///@{

        /** Get user-local configuration directory (for VidRe).
         * */
        std::filesystem::path configdir() noexcept;

        /** Get system temporary directory.
         * */
        std::filesystem::path tmpdir() noexcept;

        /** Get user-local cache directory (for VidRe).
         * */
        std::filesystem::path cachedir() noexcept;

        /** System installation direction of VidRe.
         * */
        std::filesystem::path installdir() noexcept;

        ///@}

    }  // namespace os

}  // namespace vidre

#endif  // VIDRE_CORE_OS_PATHS_HH