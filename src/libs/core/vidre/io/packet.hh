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
#include <vidre/util/uuid.hh>
#include <vidre/util/chrono.hh>

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

            /** Byte alignment.
             * */
            inline constexpr size_t alignment = 8;

            /** Major type of a packet.
             * */
            enum class major : uint16_t {
                undefined = 0x0000,

                log = 0x0001,    ///< Log messages (see @ref vidre::log).
                plain = 0x0002,  ///< Plain data (see @ref FIXME).

                // standard types
                video = 0x0010,  ///< Video data (see @ref FIXME).
                audio = 0x0020,  ///< Audio data (see @ref FIXME).
                // FIXME...

                // user types
                user = 0x8000,  ///< User-defined packet types (see @ref FIXME).
            };

            /** Data types.
             * */
            enum class plain : uint16_t {
                group = 0x0000,  ///< Grouped packet (see @ref FIXME).

                nil = 0x0001,      ///< Nil value (no data).
                boolean = 0x0002,  ///< Boolean value (true/false).
                integer = 0x0003,  ///< 64-bit signed integer.
                natural = 0x0004,  ///< 64-bit unsigned integer.
                number = 0x0005,   ///< 64-bit floating-point value (double).
                string = 0x0006,   ///< UTF-8 sequence (not! zero-terminated).

                binary = 0x0008,  ///< Binary data (see @ref FIXME).

                isodate = 0x000A,    ///< ISO 8601 date ("YYYYMMDDTHHMMSS.SSSZ\0\0\0").
                timestamp = 0x000B,  ///< Timestamp.
                localtime = 0x000C,  ///< Local time (timestamp with timezone information).

                uuid = 0x000F,  ///< UUID (128-bit universally unique identifier).
            };

            /** Minor type of a packet.
             * */
            enum class minor : uint16_t {
                group = 0x0000,  ///< Grouped packet.
            };

            /** Packet ID.
             * */
            struct __vidre_packed__(4) VIDRE_CORE_VISIBILITY id {
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

            /** Stream packets.
             * This reflects the basic type of a packet ([TLV
             * encoded](https://en.wikipedia.org/wiki/Type%E2%80%93length%E2%80%93value)),
             * which is used for dispatching to the appropriate handler. The
             * actual content of the packet is determined by the major and minor
             * types, and may be further specified by the stream ID (e.g., for
             * multiplexed streams).
             * */
            struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY basic {
                id type{};          ///< Packet ID.
                uint32_t stream{};  ///< Stream ID (e.g., for multiplexed streams).
                uint64_t length{};  ///< Size of the packet payload (excluding the header).

                constexpr std::size_t hash() const noexcept {
                    return type.hash();
                }

                basic() = default;
                basic(const basic &) = delete;
                basic &operator=(const basic &) = delete;
                basic(basic &&) = default;
                basic &operator=(basic &&) = default;

                basic(id type_, uint32_t stream = 0, uint64_t length = 0) noexcept : type{type_}, stream(stream), length(length) {}
                basic(uint16_t major, uint16_t minor, uint32_t stream = 0, uint64_t length = 0) noexcept
                    : type{major, minor}, stream(stream), length(length) {}
                virtual ~basic() = default;

                /** Get storage size (aligned).
                 * */
                constexpr size_t size() const noexcept {
                    return (length + alignment - 1) & ~(alignment - 1);
                }

                /** Update packet length (if applicable).
                 * */
                virtual void update() noexcept {}

                /** Write the packet to a sink.
                 * @param f The sink to write to.
                 * @return The number of bytes written.
                 * @internal
                 * This method, even though virtual, should always be inlined by
                 * the compiler, allowing for efficient packet writing without
                 * the overhead of virtual function calls when the compiler can
                 * determine the packet type at compile time.
                 * @endinternal
                 * */
                virtual offset_type put(io::sink &f) const {
                    return f.put({sizeof(basic), reinterpret_cast<memory::const_pointer>(this)});
                }

                /** Helper for padding.
                 * */
                offset_type pad(io::sink &f) const {
                    size_t padding = size() - length;
                    if (padding > 0) {
                        static const uint8_t padbuf[alignment] = {};
                        return f.put({padding, padbuf});
                    }
                    return 0;
                }

                /* FIXME how to handle packet-based loading?

                There should be some internal mapping of T <-> reader. The
                reader should return "basic *" (freshly allocated) or "T*"?

                -> requires some form of registry...

                Note: virtual doesn't cut it here, we do not know what type of
                packe we want to read at compile time...

                */
                offset_type get(io::source &s) noexcept {
                    return s.get({sizeof(basic), reinterpret_cast<memory::pointer>(this)});
                }
            };

            ///@}

        }  // namespace packet

    }  // namespace io

}  // namespace vidre

///@cond
namespace std {
    template <>
    struct hash<vidre::io::packet::id> {
        size_t operator()(const vidre::io::packet::id &h) const noexcept {
            return h.hash();
        }
    };
    template <>
    struct hash<vidre::io::packet::basic> {
        size_t operator()(const vidre::io::packet::basic &b) const noexcept {
            return b.hash();
        }
    };
}  // namespace std
///@endcond

#endif  // VIDRE_CORE_IO_TYPES_HH
