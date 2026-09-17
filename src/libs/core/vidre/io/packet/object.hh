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
#ifndef VIDRE_CORE_IO_PACKET_OBJECT_HH
#define VIDRE_CORE_IO_PACKET_OBJECT_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/io/packet/base.hh>
#include <vidre/io/packet/value.hh>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-packet
         * */
        ///@{

        /** Object (i.e. key-value pair)
         * */
        class VIDRE_CORE_VISIBILITY packet::object : public virtual packet {
        public:
            using key_type = shared<packet::value>;
            using value_type = shared<packet::value>;

        public:
            object(const object &) = delete;
            object &operator=(const object &) = delete;
            object(object &&) = default;
            object &operator=(object &&) = default;
            object(const header &hdr_) noexcept : packet{hdr_} {}

            virtual ~object() = default;

            object(uint16_t stream = 0) noexcept : io::packet{packet::major::object, packet::minor::group, stream, 0} {}
            object(packet::value &&k, packet::value &&v, uint16_t stream = 0) noexcept
                : io::packet{packet::major::object, packet::minor::group, stream, 0} {
                key_ = make_shared<packet::value>(std::move(k));
                value_ = make_shared<packet::value>(std::move(v));
            }

        protected:
            key_type key_;
            value_type value_;

        public:
            void update() noexcept override;
            result emit(io::sink &f) override;
            void absorb(memory::view view, std::endian endian = std::endian::native) override;
            settings::json to_json() const override;

        public:
            shared<packet> key() const noexcept {
                return key_;
            }
            shared<packet> value() const noexcept {
                return value_;
            }
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_PACKET_OBJECT_HH
