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
#ifndef VIDRE_CORE_IO_PACKET_BASE_HH
#define VIDRE_CORE_IO_PACKET_BASE_HH
#pragma once

/**@addtogroup api-core-io-packet-base
 * @brief Basic packet types.
 * @details
 * @fileinfo
 */

#include <vidre/core/config.hh>
#include <vidre/cpp/shared.hh>
#include <vidre/util/uuid.hh>
#include <vidre/util/chrono.hh>
#include <vidre/io/types.hh>
#include <vidre/settings/json.hh>

#include <functional>
#include <bit>

#include <ankerl/unordered_dense.h>

namespace vidre {

    namespace io {

        // forward declarations
        class manifest;

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

        /**@addtogroup api-core-io-packet-base
         * */
        ///@{

        /** Default byte alignment.
         * */
        inline constexpr size_t alignment = 8;

        /** Major/minor packet code concept.
         * */
        template <typename T>
        concept packet_code =
            (std::unsigned_integral<std::remove_cvref_t<T>> && !std::same_as<std::remove_cvref_t<T>, bool> &&
             sizeof(std::remove_cvref_t<T>) <= sizeof(uint16_t)) ||
            (std::is_enum_v<std::remove_cvref_t<T>> && std::unsigned_integral<std::underlying_type_t<std::remove_cvref_t<T>>> &&
             sizeof(std::underlying_type_t<std::remove_cvref_t<T>>) <= sizeof(uint16_t));

        /** Stream packets.
         * This reflects the basic type of an IO packet ([TLV
         * encoded](https://en.wikipedia.org/wiki/Type%E2%80%93length%E2%80%93value)),
         * used for dispatching to the appropriate handler. The actual content
         * of the packet is determined by the major and minor types, and may be
         * further specified by the stream ID (e.g., for multiplexed streams).
         * */
        class VIDRE_CORE_VISIBILITY packet {
        public:
            /** Major type of a packet.
             * */
            enum class major : uint16_t {
                undefined = 0x0000,

                // basic types (usually representable also in JSON format)
                value = 0x0001,     ///< Plain data (see `io::packet::value`).
                sequence = 0x0002,  ///< Sequence of values (see `io::packet::sequence`).
                object = 0x0003,    ///< Structured data (see `io::packet::object`).

                manifest = 0x000F,  ///< Manifest data (see `io::manifest`).

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

            /** Packet ID.
             * */
            struct __vidre_packed__(4) VIDRE_CORE_VISIBILITY id {
                uint16_t major{};  ///< Major packet type.
                uint16_t minor{};  ///< Sub-type.

                constexpr id() = default;
                template <packet_code Major, packet_code Minor>
                constexpr id(Major major_, Minor minor_)
                    : major(static_cast<uint16_t>(major_)), minor(static_cast<uint16_t>(minor_)) {}

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

                constexpr bool operator==(const id &other) const noexcept {
                    return major == other.major && minor == other.minor;
                }
            };

            /** The packet header.
             * */
            struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY header {
                id type{};          ///< Packet ID.
                uint32_t stream{};  ///< Stream ID (e.g., for multiplexed streams).
                uint64_t length{};  ///< Size of the packet payload (excluding the header).
            };

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

        public:
            packet() = default;
            packet(const packet &) = delete;
            packet &operator=(const packet &) = delete;
            packet(packet &&) = default;
            packet &operator=(packet &&) = default;

            packet(id type_, uint32_t stream = 0, uint64_t length = 0) noexcept : hdr{type_, stream, length} {}
            template <packet_code Major, packet_code Minor>
            packet(Major major, Minor minor, uint32_t stream = 0, uint64_t length = 0) noexcept
                : hdr{{major, minor}, stream, length} {}

            packet(const header &hdr_) noexcept : hdr{hdr_} {}
            virtual ~packet() = default;

        protected:
            header hdr{};  ///< Packet header.

        public:
            /** Get size of serialized packet data (aligned).
             * @return The size of the packet data, aligned to the required byte boundary.
             * */
            constexpr size_t size() const noexcept {
                return sizeof(header) + ((hdr.length + alignment - 1) & ~(alignment - 1));
            }

            /** Hash of the packet ID.
             * @note: The stream ID is ignored in the hash calculation,
             *        as it is not part of the packet type.
             * This method is intended for use with the packet factory only.
             * */
            constexpr std::size_t hash() const noexcept {
                return hdr.type.hash();
            }

            /** Get the packet header.
             * @return The packet header.
             * */
            constexpr const header &getheader() const noexcept {
                return hdr;
            }

            /** Update packet length (if applicable).
             * This method is intended to be overridden by derived packet types
             * that need to update their length based on their content or other
             * factors. The default implementation does nothing.
             *
             * @note This method is automatically called before writing the packet
             *       to a sink in the base class implementation of `emit()`.
             * */
            virtual void update() noexcept {}

            /** Write the packet to a sink.
             * @param f The sink to write to.
             * @return The result of the write operation, including any error status and the number of bytes written.
             *
             * Inheriting classes must override this method to provide the specific
             * behavior for writing their packet data to the sink, for example:
             * @code
             * io::result log::packet::emit(io::sink &f) override {
             *   return io::packet::emit(f)
             *          + f.put(timestamp)
             *          + f.put({message.size(), reinterpret_cast<const uint8_t *>(message.data())})
             *          + pad(f);
             * }
             * @endcode
             *
             * */
            virtual result emit(io::sink &f);

            /** Helper for padding.
             * @note This method is to be called when the packet payload possibly
             *       does not align to the required byte boundary (see `alignment`)
             *       as packet writing finalization step.
             * */
            result pad(io::sink &f) const;

            /** "Absorb" the packet from memory.
             * @param view The memory view containing the packet data.
             * */
            virtual void absorb(memory::view view, std::endian endian = std::endian::native);

            /** Convert to JSON (if applicable).
             * */
            virtual settings::json to_json() const {
                return {};
            }

        public:
            class queue;

            struct factory;

            class value;
            class sequence;
            class object;

            template <typename T>
            struct registry;

        private:
            friend struct factory;
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

#endif  // VIDRE_CORE_IO_PACKET_BASE_HH
