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
#ifndef VIDRE_CORE_UTIL_DEVELOP_HH
#define VIDRE_CORE_UTIL_DEVELOP_HH
#pragma once

/**@defgroup api-core-util-cpp C++ utilities.
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>

#include <type_traits>
#include <climits>

namespace vidre {

    /**@addtogroup api-core-util-cpp
     * */
    ///@{

    /** Get size of an array at compile time.
     *
     * This is the equivalent of
     * @code
     * sizeof(arr) / sizeof(arr[0]);
     * @endcode
     *
     */
    template <typename T, std::size_t N>
    constexpr std::size_t size_of(T (&)[N]) noexcept {
        return N;
    }

    /** Get number of bits in a type.
     * */
    template<typename T>
    struct bits_of {
        static constexpr std::size_t value = CHAR_BIT * sizeof(T);
    };

    /** Get value (such as enum class) as underlying value.
     * */
    template <typename E>
    constexpr auto to_underlying(E e) noexcept {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_UTIL_DEVELOP_HH
