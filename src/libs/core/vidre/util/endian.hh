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
#ifndef VIDRE_CORE_UTIL_ENDIAN_HH
#define VIDRE_CORE_UTIL_ENDIAN_HH
#pragma once

/**@defgroup api-core-util-endian Endianness handling
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>

#include <bit>
#include <concepts>

namespace vidre {

    /** Endianness handling.
     * @ingroup api-core-util-endian
     * */
    namespace endian {

        /**@addtogroup api-core-util-endian
         * */
        ///@{

        /** Byteswap a value from a given endianness to the target endianness.
         * @tparam T The type of the value to byteswap.
         * @tparam Target The target endianness (default is native).
         * @param value The value to byteswap.
         * @param from The current endianness of the value.
         * @return The byteswapped value if the endianness differs from the target, otherwise the original value.
         */
        template <typename T, std::endian Target = std::endian::native>
        requires (not std::floating_point<T>)
        [[nodiscard]] inline constexpr T byteswap(T value, std::endian from) noexcept {
            if (from == Target)
                return value;
            else
                return std::byteswap(value);
        }

        template <std::floating_point T, std::endian Target = std::endian::native>
        [[nodiscard]] inline constexpr T byteswap(T value, std::endian from) noexcept {
            if (from == Target)
                return value;
            else {
                if constexpr (sizeof(T) == 4) {
                    auto bits = std::bit_cast<uint32_t>(value);
                    return std::bit_cast<T>(std::byteswap(bits));
                }
                else if constexpr (sizeof(T) == 8) {
                    auto bits = std::bit_cast<uint64_t>(value);
                    return std::bit_cast<T>(std::byteswap(bits));
                }
            }
        }

        /** Byteswap a value in place.
         * @tparam T The type of the value to byteswap.
         * @tparam Target The target endianness (default is native).
         * @param value The value to byteswap.
         * @param from The current endianness of the value.
         * */
        template <typename T, std::endian Target = std::endian::native>
        inline constexpr void byteswap_inplace(T &value, std::endian from) noexcept {
            if (from != Target)
                value = std::byteswap(value);
        }

        ///@}

    }  // namespace endian

}  // namespace vidre

#endif  //  VIDRE_CORE_UTIL_ENDIAN_HH
