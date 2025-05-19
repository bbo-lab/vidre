/* VidRe
 * Copyright (C) 2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
///@file
#ifndef VIDRE_CORE_CPP_UTIL_HH
#define VIDRE_CORE_CPP_UTIL_HH
#pragma once

#include <vidre/config.hh>

/**@defgroup api-core-cpp-util Utilities
 * @ingroup api-core-cpp
 * @details
 * @fileinfo
 * */

#include <cstddef>

namespace vidre {

    namespace detail {

        // FIXME

    } //

    /**@addtogroup api-core-cpp-util
     * */
    ///@{

    /** Get size of an array of any type.
     * */
    template <typename T, std::size_t N>
    constexpr std::size_t size_of(const T (&)[N]) noexcept {
        return N;
    }

    /** Get demangled name of FIXME.
     * */

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_CPP_UTIL_HH
