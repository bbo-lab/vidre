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
 * @{
 * @namespace vidre::io::packet Packet-based I/O types.
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/io/types.hh>

#include <functional>

namespace vidre {

    namespace io {

        namespace packet {

            /**@addtogroup api-core-io-packet
             * */
            ///@{

            /** Major type of a packet.
             * */
            enum class major : uint16_t {
                undefined = 0x0000,

                // range 0x0001 - 0x000F are reserved for storage::raw

                // standard types
                video = 0x0010,  ///< Video data (see @ref FIXME).
                audio = 0x0020,  ///< Audio data (see @ref FIXME).
                // FIXME...

                // user types
                user = 0x8000,  ///< User-defined packet types (see @ref FIXME).
            };

            /** Sub-type "video" minor types.
             * */
            enum class video : uint16_t {
                undefined = 0x0000,  ///< Video format is undefined, relevant information is embedded in the stream config.

                packed = 0x0001,  ///< Data is packed (flag).
                mask = 0xFFFE,    ///< Mask for extracting format.

                // standard monochrome formats
                mono8 = 0x0100,
                mono8_packed = 0x0101,
                mono10 = 0x0110,
                mono10_packed = 0x0111,
                mono12 = 0x0120,
                mono12_packed = 0x0121,
                mono14 = 0x0130,
                mono14_packed = 0x0131,
                mono16 = 0x0140,

                // standard Bayer formats
                bggr8 = 0x0200,
                // FIXME...

                // standard RGB formats
                rgba32 = 0x0300,
            };

            ///@cond
            video operator|(video lhs, video rhs) noexcept {
                return static_cast<video>(to_underlying(lhs) | to_underlying(rhs));
            }
            video &operator|=(video &lhs, video rhs) noexcept {
                lhs = lhs | rhs;
                return lhs;
            }
            video operator&(video lhs, video rhs) noexcept {
                return static_cast<video>(to_underlying(lhs) & to_underlying(rhs));
            }
            video &operator&=(video &lhs, video rhs) noexcept {
                lhs = lhs & rhs;
                return lhs;
            }
            video operator~(video v) noexcept {
                return static_cast<video>(~to_underlying(v));
            }
            ///@endcond

            /** Sub-type "audio" minor types.
             * */
            enum class audio : uint16_t {
                undefined = 0x0000,

                bits = 0x000F,      ///< Mask for extracting bits per sample.
                type = 0x00F0,      ///< Mask for extracting audio type.
                encoding = 0xFF00,  ///< Mask for extracting audio encoding.

                // standard audio types
                mono = 0x0010,
                stereo = 0x0020,
                multichannel = 0x0030,

                // standard audio encodings
                // FIXME...
            };

            ///@cond
            audio operator|(audio lhs, audio rhs) noexcept {
                return static_cast<audio>(to_underlying(lhs) | to_underlying(rhs));
            }
            audio &operator|=(audio &lhs, audio rhs) noexcept {
                lhs = lhs | rhs;
                return lhs;
            }
            audio operator&(audio lhs, audio rhs) noexcept {
                return static_cast<audio>(to_underlying(lhs) & to_underlying(rhs));
            }
            audio &operator&=(audio &lhs, audio rhs) noexcept {
                lhs = lhs & rhs;
                return lhs;
            }
            audio operator~(audio v) noexcept {
                return static_cast<audio>(~to_underlying(v));
            }
            ///@endcond

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
            };

#if 0
            /** Data packet base type.
             * */
            struct VIDRE_CORE_VISIBILITY packet {
                vidre::uuid uuid{};  ///< Packet type (e.g., video, audio, metadata).
                uint32_t stream{};   ///< Stream identifier (e.g., for multiplexed streams).
                // /** Function type for putting a packet-based streams into a sink.
                //  * */
                // using put_type = void (*)(const io::packet &source, io::sink &sink);

                // put_type putter{};   ///< Function pointer for putting this packet into a sink.

                // public:
                //     using selector_type = put_type (*)(const io::packet &source, const io::packet &sink);
                //     using map_type = ankerl::unordered_dense::map<vidre::uuid, selector_type>;

                // protected:
                //     static map_type registry;  ///< Static map for registering packet types and their put functions.

                // public:
                //     /** Register a "selector" function.
                //      * */
                //     static void enregister(const vidre::uuid &uuid, selector_type func) {
                //         auto it = registry.find(uuid);
                //         if (it != registry.end())
                //             throw vidre::exception() << error::message("packet type already registered");

                //         registry[uuid] = func;
                //     }

                //     /** Retrieve selector function.
                //      * */
                //     static selector_type retrieve(const vidre::uuid &uuid) {
                //         auto it = registry.find(uuid);
                //         if (it == registry.end())
                //             throw vidre::exception() << error::message("packet type not registered");

                //         return it->second;
                //     }
            };
#endif

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
