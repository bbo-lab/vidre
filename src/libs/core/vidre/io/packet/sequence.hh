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
#ifndef VIDRE_CORE_IO_PACKET_SEQUENCE_HH
#define VIDRE_CORE_IO_PACKET_SEQUENCE_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/io/packet/base.hh>

namespace vidre {

    namespace io {

         /**@addtogroup api-core-io-packet-object
         * @{
         * */
        /** Sequence of packets.
         * */
        class VIDRE_CORE_VISIBILITY packet::sequence : public packet {
        public:
            using list_type = std::vector<shared<packet>>;

        public:
            sequence(const sequence &) = delete;
            sequence &operator=(const sequence &) = delete;
            sequence(sequence &&) = default;
            sequence &operator=(sequence &&) = default;
            sequence(const header &hdr_) noexcept : packet{hdr_} {}

            virtual ~sequence() = default;

            sequence(uint16_t stream = 0) noexcept : io::packet{packet::major::sequence, packet::minor::group, stream, 0} {}

        protected:
            list_type packets_{};  ///< Sequence of packets contained within this sequence.

        public:
            void update() noexcept override;
            result emit(io::sink &f) override;
            void absorb(memory::view view, std::endian endian = std::endian::native) override;
            settings::json to_json() const override;

        public:
            /** Get the sequence of packets contained within this sequence.
             * */
            const list_type &packets() const noexcept {
                return packets_;
            }

            /** Get the packet cound contained within this sequence.
             * */
            std::size_t count() const noexcept {
                return packets_.size();
            }

            /** Add packet.
             * */
            void add(shared<packet> pkt) {
                packets_.push_back(std::move(pkt));
            }
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_PACKET_SEQUENCE_HH
