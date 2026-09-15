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
#ifndef VIDRE_CORE_IO_PACKET_HH
#define VIDRE_CORE_IO_PACKET_HH
#pragma once

/**@addtogroup api-core-io-packet
 * @brief Packet-based I/O.
 * @details
 * @fileinfo
 *
 * Packet-based I/O is designed for handling variable-sized data units, such as video
 * frames or audio samples, which may not fit well into a continuous byte-stream model
 * (e.g., TCP or raw file I/O). It provides a structured way to read and write packets
 * of data, each with its own header and payload. This allows for more efficient
 * processing and better support for multiplexed data streams.
 *
 * Here, for packets, we adopt the [type-length-value](https://en.wikipedia.org/wiki/Type%E2%80%93length%E2%80%93value)
 * encoding, with some of the underlying logic inspired by [ASN.1](https://en.wikipedia.org/wiki/ASN.1):
 * - the type is subdivided into a
 *   * major (describing the overall use of the packet) and a
 *   * minor number (describing the structure or disposition of the major type),
 * - the length of the data packet, followed by
 * - the value, or content, respectively.
 *
 * A JSON document, might, for example, be represented by an `packet::major::object`,
 * consisting of a sequence of a key-value pairs, consisting of a `packet::plain::string`
 * and a value (such as a packet::plain::number).
 *
 * The framework implements structures preferrably in a way, that would allow to
 * store these byte-wise in a file (cf. `storage::raw::magic`) but also takes into
 * account the majorly dynamic nature of packets.
 *
 * @see @ref vidre::storage::raw
 * @{
 * @namespace vidre::io::packet Packet-based I/O types.
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/shared.hh>
#include <vidre/util/uuid.hh>
#include <vidre/util/chrono.hh>
#include <vidre/io/types/core.hh>

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
             * */
            static_assert(std::numeric_limits<float>::is_iec559);
            static_assert(sizeof(float) == sizeof(uint32_t));
            static_assert(sizeof(double) == sizeof(uint64_t));

            ///@}

        }  // namespace detail

        /**@addtogroup api-core-io-packet
         * */
        ///@{

        /** Default byte alignment.
         * */
        inline constexpr size_t alignment = 8;

        /** Stream packets.
         * This reflects the basic type of an IO packet ([TLV
         * encoded](https://en.wikipedia.org/wiki/Type%E2%80%93length%E2%80%93value)),
         * used for dispatching to the appropriate handler. The actual content
         * of the packet is determined by the major and minor types, and may be
         * further specified by the stream ID (e.g., for multiplexed streams).
         *
         * @todo need a stream ID registry (0 is reserved for unassigned streams, such as log)
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY packet {
            /** Major type of a packet.
             * */
            enum class major : uint16_t {
                undefined = 0x0000,

                plain = 0x0001,   ///< Plain data (see @ref FIXME).
                object = 0x0002,  ///< Structured data (see @ref FIXME).

                // standard types
                video = 0x0010,   ///< Video data (see @ref FIXME).
                audio = 0x0020,   ///< Audio data (see @ref FIXME).
                sensor = 0x0030,  ///< Sensor data, e.g. temperature (see @ref FIXME).

                log = 0x8080,  ///< Log message (see `log::packet`).

                // user types
                user = 0xF000,  ///< User-defined packet types (see @ref FIXME).
            };

            /** Minor type of a packet.
             * @note For every major type, the minor type '0x0000' is reserved for
             *       the group (one prominent exception is the log packet, which does
             *       not follow this scheme—[«Ausnahmen bestätigen die
             * Regel»](https://en.wikipedia.org/wiki/Exception_that_proves_the_rule)).
             * */
            enum class minor : uint16_t {
                group = 0x0000,  ///< Grouped packet.
            };

            /** Plain data types.
             * */
            enum class plain : uint16_t {
                sequence = 0x0000,   ///< Grouped packet (see @ref FIXME).
                                     // === 0x0001-0x000F reserved
                nil = 0x0010,        ///< Nil value (no data).
                boolean = 0x0011,    ///< Boolean value (true/false).
                integer = 0x0012,    ///< 64-bit signed integer.
                natural = 0x0013,    ///< 64-bit unsigned integer.
                number = 0x0014,     ///< 64-bit floating-point value (double).
                string = 0x0015,     ///< UTF-8 sequence (not! zero-terminated).
                binary = 0x0016,     ///< Binary data (see @ref FIXME).
                uuid = 0x0017,       ///< UUID (128-bit universally unique identifier).
                                     // === 0x0018-0x001F reserved for future POD types.
                isodate = 0x0020,    ///< ISO 8601 date ("YYYYMMDDTHHMMSS.SSSZ\0\0\0").
                timestamp = 0x0021,  ///< Timestamp.
                localtime = 0x0022,  ///< Local time (timestamp with timezone information).
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

            id type{};          ///< Packet ID.
            uint32_t stream{};  ///< Stream ID (e.g., for multiplexed streams).
            uint64_t length{};  ///< Size of the packet payload (excluding the header).

            constexpr std::size_t hash() const noexcept {
                return type.hash();
            }

            packet() = default;
            packet(const packet &) = delete;
            packet &operator=(const packet &) = delete;
            packet(packet &&) = default;
            packet &operator=(packet &&) = default;

            packet(id type_, uint32_t stream = 0, uint64_t length = 0) noexcept : type{type_}, stream(stream), length(length) {}
            packet(uint16_t major, uint16_t minor, uint32_t stream = 0, uint64_t length = 0) noexcept
                : type{major, minor}, stream(stream), length(length) {}
            virtual ~packet() = default;

            /** Create a shared pointer to a new packet.
             * This method simplifies the creation of shared pointers to packets,
             * forwarding the provided arguments to the constructor of the packet type.
             * @tparam T The type of the packet to create.
             * @tparam Args The types of the arguments to forward to the packet constructor.
             * @param args The arguments to forward to the packet constructor.
             * @return A shared pointer to the newly created packet.
             * */
            template <typename T, typename... Args>
            static auto make(Args &&...args) {
                return make_shared<T>(std::forward<Args>(args)...);
            }

            /** Create a shared pointer to a static packet.
             * This method creates a shared pointer to a packet that is
             * statically allocated, meaning it does not manage the
             * lifetime of the packet. The provided deleter does nothing,
             * ensuring that the static packet is not deleted when the
             * shared pointer goes out of scope.
             * @tparam T The type of the packet.
             * @param obj The static packet object.
             * @return A shared pointer to the static packet.
             * */
            template <typename T>
            static auto make_static(T &obj) {
                return shared<T>(&obj, foreign<T>{});
            }

            /** Get storage size (aligned).
             * @return The size of the packet, aligned to the required byte boundary.
             * */
            constexpr size_t size() const noexcept {
                return (length + alignment - 1) & ~(alignment - 1);
            }

            /** Update packet length (if applicable).
             * This method is intended to be overridden by derived packet types
             * that need to update their length based on their content or other
             * factors. The default implementation does nothing.
             * */
            virtual void update() noexcept {}

            /** Write the packet to a sink.
             * @param f The sink to write to.
             * @return The result of the write operation, including any error status and the number of bytes written.
             * @internal
             * This method, even though virtual, should always be inlined by
             * the compiler, allowing for efficient packet writing without
             * the overhead of virtual function calls when the compiler can
             * determine the packet type at compile time.
             * @endinternal
             * */
            virtual result put(io::sink &f) const;

            /** Helper for padding.
             * @note This method is to be called when the packet payload possibly
             *       does not align to the required byte boundary (see `alignment`)
             *       as packet writing finalization step.
             * */
            result pad(io::sink &f) const;

            /* FIXME how to handle packet-based loading?

            There should be some internal mapping of T <-> reader. The
            reader should return "basic *" (freshly allocated) or "T*"?

            -> requires some form of registry...

            Note: virtual doesn't cut it here, we do not know what type of
            packe we want to read at compile time...

            */
            result get(io::source &s) noexcept;
        };

        ///@}

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
    struct hash<vidre::io::packet> {
        size_t operator()(const vidre::io::packet &b) const noexcept {
            return b.hash();
        }
    };
}  // namespace std
///@endcond

#endif  // VIDRE_CORE_IO_TYPES_HH
