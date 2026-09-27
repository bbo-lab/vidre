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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
///@file
#ifndef VIDRE_CORE_CPP_UTIL_HH
#define VIDRE_CORE_CPP_UTIL_HH
#pragma once

/**@defgroup api-core-cpp-util Utilities
 * @ingroup api-core-cpp
 * @details
 * @fileinfo
 *
 * @todo symbol demangling support
 * */

#include <vidre/config.hh>

#include <type_traits>
#include <string_view>
#include <climits>
#include <cstddef>

namespace vidre {

    /**@addtogroup api-core-cpp-util
     * */
    ///@{

    /** Get size of an array of any type.
     * */
    template <typename T, std::size_t N>
    constexpr std::size_t size_of(const T (&)[N]) noexcept {
        return N;
    }

    /** Get number of bits in a type.
     * */
    template <typename T>
    struct bits_of {
        static constexpr std::size_t value = CHAR_BIT * sizeof(T);
    };

    /** Get value (such as enum class) as underlying value.
     * */
    template <typename E>
    constexpr auto to_underlying(E e) noexcept {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    /** Compile-time fixed-size string.
     *
     * Stores a string literal, including its terminating NUL character, in a
     * structural type suitable for use as a non-type template parameter (NTTP).
     *
     * The template size @p N denotes the complete storage size, including the
     * terminating NUL character. Consequently, size() returns @c N-1.
     *
     * @tparam N Storage size in characters, including the terminating NUL.
     *
     * @code
     * template <fixed_string Name>
     * void foo();
     *
     * foo<"example">();
     * @endcode
     */
    template <std::size_t N>
    struct fixed_string {
        char data[N];  ///< Character storage, including the terminating NUL.

        /** Construct from a string literal.
         *
         * @param str String literal to copy, including its terminating NUL.
         */
        constexpr fixed_string(const char (&str)[N]) noexcept {
            for (std::size_t i = 0; i < N; ++i)
                data[i] = str[i];
        }

        /** Return the string as a NUL-terminated C string.
         *
         * @returns Pointer to the internal character storage.
         */
        constexpr const char *c_str() const noexcept {
            return data;
        }

        /** Return a view of the string.
         *
         * The terminating NUL character is not part of the returned view.
         *
         * @returns String view containing the stored string.
         */
        constexpr std::string_view view() const noexcept {
            return {data, N - 1};
        }

        /** Return the length of the string.
         *
         * @returns Number of characters excluding the terminating NUL.
         */
        constexpr std::size_t size() const noexcept {
            return N - 1;
        }

        /** Implicit conversion to a string view.
         *
         * @returns String view containing the stored string, excluding the
         *          terminating NUL.
         */
        constexpr operator std::string_view() const noexcept {
            return view();
        }
    };

    /** Deduction guide for fixed strings.
     *
     * Deduces the storage size, including the terminating NUL, directly from a
     * string literal.
     */
    template <std::size_t N>
    fixed_string(const char (&)[N]) -> fixed_string<N>;

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_CPP_UTIL_HH
