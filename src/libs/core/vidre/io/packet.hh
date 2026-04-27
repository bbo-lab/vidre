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
#ifndef VIDRE_CORE_IO_PACKET_HH
#define VIDRE_CORE_IO_PACKET_HH
#pragma once

/**@addtogroup api-core-io-packet
 * @brief Packet-based I/O.
 * @details
 * @fileinfo
 *
 * Packet-based I/O is designed for handling variable-sized data units, such as video frames or audio
 * samples, which may not fit well into a stream-based model. It provides a structured way to read and
 * write packets of data, each with its own header and payload. This allows for more efficient processing
 * and better support for multiplexed data streams.
 *
 * @{
 * @namespace vidre::io::packet Packet-based I/O types.
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/io/types.hh>

#include <functional>

namespace vidre {

    namespace io {

        namespace detail {

            /**@addtogroup api-core-io-details
             * */
            ///@{

            /** Require IEEE 754 floating point types.
             * The internal packet system only works at this moment, if the standard 
             * floating point data types conform to the IEEE 754 standard. 
             * 
             * If, though, raw file storage has a system description in the header, 
             * this requirement could be relaxed in the future.
             * 
             * @todo decide how to implement non-IEEE 754 (an "endianness" field could be extended with "strange" system types...?)
             * */
            static_assert(std::numeric_limits<float>::is_iec559);
            static_assert(sizeof(float) == sizeof(uint32_t));
            static_assert(sizeof(double) == sizeof(uint64_t));

            ///@}

        }  // namespace detail

        namespace packet {

            /**@addtogroup api-core-io-packet
             * */
            ///@{

            /** Major type of a packet.
             * */
            enum class major : uint16_t {
                undefined = 0x0000,

                log = 0x0001,   ///< Log messages (see @ref vidre::log).
                data = 0x0002,  ///< Arbitrary data (see @ref FIXME).

                // standard types
                video = 0x0010,  ///< Video data (see @ref FIXME).
                audio = 0x0020,  ///< Audio data (see @ref FIXME).
                // FIXME...

                // user types
                user = 0x8000,  ///< User-defined packet types (see @ref FIXME).
            };

            /** Data types.
             * */
            enum class data : uint16_t {
                group = 0x0000,  ///< Grouped packet (see @ref FIXME).

                binary = 0x0008,  ///< Binary data (see @ref FIXME).

                nil = 0x0010,      ///< Nil value (no data).
                boolean = 0x0011,  ///< Boolean value (true/false).
                integer = 0x0012,  ///< 64-bit signed integer.
                natural = 0x0013,  ///< 64-bit unsigned integer.
                number = 0x0014,   ///< 64-bit floating-point value (double).
                string = 0x0015,   ///< UTF-8 sequence (zero-terminated).

                isodate = 0x0020,    ///< ISO 8601 date ("YYYYMMDDTHHMMSS.SSSZ\0\0\0").
                timestamp = 0x0021,  ///< Timestamp.
                localtime = 0x0022,  ///< Local time (timestamp with timezone information).
            };

            /** Minor type of a packet.
             * */
            enum class minor : uint16_t {
                group = 0x0000,  ///< Grouped packet.
            };

            /** Packet header.
             * */
            struct __vidre_packed__(4) VIDRE_CORE_VISIBILITY header {
                uint16_t major{};  ///< Major packet type.
                uint16_t minor{};  ///< Sub-type.

                /** Hash function (FNV).
                 * @note: The stream ID is ignored in the hash calculation,
                 *        as it is not part of the packet type.
                 *
                 * */
                constexpr std::size_t hash() const noexcept {
#if VIDRE_HOST_BITS(64)
                    std::size_t result = 0xcbf29ce484222325;
                    result = (result * 0x100000001B3) ^ major;
                    result = (result * 0x100000001B3) ^ minor;
#elif VIDRE_HOST_BITS(32)
                    std::size_t result = 0x811C9DC5;
                    result = (result * 0x01000193) ^ major;
                    result = (result * 0x01000193) ^ minor;
#endif
                    return result;
                }
            };

            /** Base class for stream packets.
             * */
            struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY base {
                header hdr{};       ///< Packet header.
                uint32_t stream{};  ///< Stream ID (e.g., for multiplexed streams).

                offset_type put(io::sink &f) const noexcept {
                    return f.put({sizeof(base), reinterpret_cast<memory::const_pointer>(this)});
                }

                offset_type get(io::source &s) noexcept {
                    return s.get({sizeof(base), reinterpret_cast<memory::pointer>(this)});
                }
            };

            ///@}

        }  // namespace packet

    }  // namespace io

}  // namespace vidre

///@cond
namespace std {
    template <>
    struct hash<vidre::io::packet::header> {
        size_t operator()(const vidre::io::packet::header &h) const noexcept {
            return h.hash();
        }
    };
}  // namespace std
///@endcond

#endif  // VIDRE_CORE_IO_TYPES_HH
