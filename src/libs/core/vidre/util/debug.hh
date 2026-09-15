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
#ifndef VIDRE_CORE_UTIL_DEBUG_HH
#define VIDRE_CORE_UTIL_DEBUG_HH
#pragma once

/**@defgroup api-core-util-debug Debug utilities
 * @ingroup api-core-util
 * @details
 * @fileinfo
 *
 * @{
 * @namespace vidre::debug Debug utilities.
 * @}
 * */

#include <vidre/core/config.hh>

namespace vidre {

    namespace debug {

        /**@addtogroup api-core-util-debug
         * */
        ///@{

        /** Prevents the compiler from optimizing away a variable.
         * @tparam T Type of the variable.
         * @param val The variable to prevent optimization on.
         * */
        template <typename T>
        void do_not_optimize(T const &val) {
            asm volatile("" : : "r,m"(val) : "memory");
        }

        ///@}

    }  // namespace debug

}  // namespace vidre

#endif  //  VIDRE_CORE_UTIL_DEBUG_HH
