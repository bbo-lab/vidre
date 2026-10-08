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
#ifndef VIDRE_CORE_IO_PACKET_FACTORY_HH
#define VIDRE_CORE_IO_PACKET_FACTORY_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/util/endian.hh>
#include <vidre/io/packet/base.hh>

#include <ankerl/unordered_dense.h>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-details
         * */
        ///@{

        /** Factory class for creating packet instances.
         * FIXME
         * */
        struct VIDRE_CORE_VISIBILITY packet::factory {
            /** Function type for creating packet instances.
             * @param header The header of the packet to create.
             * @return A shared pointer to the created packet instance.
             * */
            using make_type = shared<io::packet> (*)(packet::header);

            /** Type alias for the map storing packet creation functions.
             **/
            using map_type = ankerl::unordered_dense::map<io::packet::id, make_type, std::hash<io::packet::id>>;

            /** Get internal registry.
             * @return A reference to the internal map storing packet creation functions.
             * */
            static auto &registry() {
                static map_type map;
                return map;
            }

            /** Add a packet type to the factory.
             * @tparam T The packet type to add.
             * @param id The identifier for the packet type.
             * @throws exception::logic if the packet type is already registered.
             * */
            template <typename T>
            static void add(io::packet::id id) {
                auto [it, inserted] = registry().emplace(id, [](packet::header hdr) -> shared<io::packet> {
                    return make_shared<T>(hdr);
                });

                if (!inserted)
                    throw exception::logic() << "packet type already registered";
            }

            /** Create a packet instance by its (normalized) header.
             * @param hdr The header to pass to the packet constructor.
             * @return A shared pointer to the created packet instance.
             * @throws exception::logic if the packet type is not registered.
             * */
            static shared<io::packet> make(packet::header hdr) {
                auto it = registry().find(hdr.type);
                if (it == registry().end())
                    throw exception::logic() << "packet type not registered";
                return it->second(hdr);
            }

            /** Create packet from memory view.
             * @param view The memory view containing the packet data.
             * @param from The endian format of the memory view (default: native).
             * @return A shared pointer to the created packet instance.
             * @throws exception::logic if the packet type is not registered.
             * */
            template <typename Packet = io::packet>
            requires std::derived_from<Packet, io::packet>
            static shared<Packet> make(memory::view view, std::endian from = std::endian::native) {
                packet::header hdr;
                memory::copy(view, hdr);

                if (from != std::endian::native) {
                    hdr.type.major = endian::byteswap(hdr.type.major, from);
                    hdr.type.minor = endian::byteswap(hdr.type.minor, from);
                    hdr.stream = endian::byteswap(hdr.stream, from);
                    hdr.length = endian::byteswap(hdr.length, from);
                }

                auto pkt = factory::make(hdr);
                pkt->absorb(view, from);
                return std::dynamic_pointer_cast<Packet>(pkt);
            }
        };

        ///@}

        /**@addtogroup api-core-io-packet
         * */
        ///@{

        /** Registration of packet types.
         * This helper struct simplifies the registration of packet types with the factory.
         * It supports registering individual minor types as well as ranges of minor types.
         *
         * @todo Maybe it's worth (considering the typing effort) to take this class out of "factory" (and call it "registry")...?
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
            template <packet_code Major, packet_code... Minor>
            registry(Major major, Minor... minors) {
                (                                                                                            //
                    packet::factory::add<T>({static_cast<uint16_t>(major), static_cast<uint16_t>(minors)}),  //
                    ...                                                                                      //
                );
            }

            ///@internal

            /** Helper struct representing a range of minor packet types.
             * */
            struct range_type {
                uint16_t first{};
                uint16_t last{};

                range_type() = default;
                template <packet_code Minor>
                range_type(Minor first_, Minor last_) : first(static_cast<uint16_t>(first_)), last(static_cast<uint16_t>(last_)) {}
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
            template <packet_code Major>
            registry(Major major, range_type range) {
                const auto first = range.first;
                const auto last = range.last;

                if (first > last)
                    throw exception::logic() << "invalid packet registration range";

                for (uint32_t minor = first; minor <= last; ++minor) {
                    packet::factory::add<T>({static_cast<uint16_t>(major), static_cast<uint16_t>(minor)});
                }
            }
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_PACKET_FACTORY_HH
