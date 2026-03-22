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
#ifndef VIDRE_CORE_UTIL_DEVELOP_HH
#define VIDRE_CORE_UTIL_DEVELOP_HH
#pragma once

#include <vidre/core/config.hh>

namespace vidre {

#if VIDRE_DEVELOP or defined DOXYGEN
    namespace develop {

        /**@defgroup api-core-util-develop Development utilities.
         * @ingroup api-core-util
         * @details
         * @fileinfo
         * 
         * The development utilities are only available for development
         * (see VIDRE_DEVELOP setting in CMake). For releases, empty stubs are
         * present.
         * 
         * @see @ref examples/core/develop.cc
         * 
         * */
        ///@{

        /** Check if we're executed from inside the development environment.
         * */
        bool inbuild() noexcept;

        ///@}

    }  // namespace develop
#else
    namespace develop {

        consteval inline bool inbuild() noexcept {
            return false;
        }

    }  // namespace develop
#endif

}  // namespace vidre

#endif  // VIDRE_CORE_UTIL_DEVELOP_HH
