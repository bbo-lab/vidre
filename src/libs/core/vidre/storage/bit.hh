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
#ifndef VIDRE_CORE_STORAGE_BIT_HH
#define VIDRE_CORE_STORAGE_BIT_HH
#pragma once

/**@defgroup api-core-storage-bit Bit mangling (such as packing).
 * @ingroup api-core-storage
 * @fileinfo
 * @details
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/storage/io.hh>

#include <bit>

namespace vidre {

    namespace storage {

        /**@addtogroup api-core-storage-bit
         * */
        ///@{

        /** Get output size fo pack (or unpack).
         * @tparam From     The input bit-width.
         * @tparam To       The output bit-width.
         */
        template <std::size_t From, std::size_t To>
        constexpr inline view::size_type pack_size(view::size_type bytes_in) {
            if constexpr (From <= To)
                return (bytes_in * To) / From;
            else
                return (bytes_in * To + From - 1) / From;
        }

        /** Pack (or unpack) data.
         * @tparam From     The input bit-width.
         * @tparam To       The output bit-width.
         * @tparam ByteSwap Flag, if bytes have to be swapped (such as with non-native endianness).
         */
        template <std::size_t From, std::size_t To, bool ByteSwap = false>
        view::size_type pack(const view &input, view output);

        template <>
        view::size_type pack<16, 10, false>(const view &input, view output);

        template <>
        view::size_type pack<10, 16, false>(const view &input, view output);

        template <>
        view::size_type pack<16, 12, false>(const view &input, view output);

        template <>
        view::size_type pack<12, 16, false>(const view &input, view output);

        ///@}

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_IO_HH
