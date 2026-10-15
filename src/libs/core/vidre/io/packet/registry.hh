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
#ifndef VIDRE_CORE_IO_PACKET_REGISTRY_HH
#define VIDRE_CORE_IO_PACKET_REGISTRY_HH
#pragma once

/**@addtogroup api-core-io-packet-registry
 * @brief Packet registry and decoding for creating packet instances.
 * @details
 * @fileinfo
 */

#include <vidre/core/config.hh>
#include <vidre/util/endian.hh>
#include <vidre/io/packet/base.hh>

#include <ankerl/unordered_dense.h>
#include <array>

namespace vidre {

    namespace io {

        namespace details {

            /**@addtogroup api-core-io-details
             * */
            ///@{

            /** The packet factory. */
            struct factory {
                using make_type = shared<io::packet> (*)(packet::header);

                struct bucket_type {
                    std::vector<make_type> construct{};
                    std::array<uint8_t, 256> index = [] {
                        std::array<uint8_t, 256> a;
                        a.fill(0xFF);
                        return a;
                    }();
                };

                static std::vector<bucket_type> bucket;
                static std::array<uint8_t, 256> index;

                static void add(uint8_t major, uint8_t minor_from, uint8_t minor_to, make_type make);

                /** Add a packet type to the factory.
                 * @tparam T The packet type to add.
                 * @param id The identifier for the packet type.
                 * @throws exception::logic if the packet type is already registered.
                 * */
                template <typename T, compatible_id Major, compatible_id MinorFrom, compatible_id MinorTo>
                static void add(Major major, MinorFrom minor_from, MinorTo minor_to) {
                    add(packet::id::cast(major), packet::id::cast(minor_from), packet::id::cast(minor_to), [](packet::header hdr) -> shared<io::packet> {
                        return packet::make<T>(hdr);
                    });
                }

                /** Create a packet instance by its (normalized) header.
                 * @param hdr The header to pass to the packet constructor.
                 * @return A shared pointer to the created packet instance.
                 * @throws exception::logic if the packet type is not registered.
                 * */
                static shared<io::packet> make(packet::header hdr);
            };

            ///@}

        }  // namespace details

        /**@addtogroup api-core-io-packet-registry
         * */
        ///@{

        /** Registration of packet types.
         * This helper struct simplifies the registration of packet types with the factory.
         * It supports registering individual minor types as well as ranges of minor types.
         *
         * @snippet example/core/packetio.c simple-packet-registry
         * @see `io::packet`
         * */
        template <typename T>
        struct packet::registry {
            /** Register a list of minor packet types for a given major type.
             * @tparam Major The major packet type.
             * @tparam Minor The minor packet types.
             * @param major The major packet type value.
             * @param minors The minor packet type values.
             *
             * Example:
             * @code
             * packet::registry<MyPacketType> reg(MAJOR_TYPE, MINOR_TYPE_1, MINOR_TYPE_2);
             * @endcode
             * */
            template <compatible_id Major, compatible_id... Minor>
            registry(Major major, Minor... minors) {
                (                                                     //
                    details::factory::add<T>(major, minors, minors),  //
                    ...                                               //
                );
            }

            ///@internal

            /** Helper struct representing a range of minor packet types.
             * */
            struct range_type {
                uint8_t first{};
                uint8_t last{};

                range_type() = default;
                template <compatible_id Minor>
                range_type(Minor first_, Minor last_) : first(packet::id::cast(first_)), last(packet::id::cast(last_)) {}
            };

            ///@endinternal

            /** Register a range of minor packet types for a given major type.
             * @tparam Major The major packet type.
             * @param major The major packet type value.
             * @param range The range of minor packet type values.
             *
             * Example:
             * @code
             * packet::registry<MyPacketType> reg(MAJOR_TYPE, {MINOR_START, MINOR_END});
             * @endcode
             * */
            template <compatible_id Major>
            registry(Major major, range_type range) {
                details::factory::add<T>(major, range.first, range.last);
            }
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_PACKET_REGISTRY_HH
