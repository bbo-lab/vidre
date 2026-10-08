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
#ifndef VIDRE_CORE_IO_PACKET_VALUE_HH
#define VIDRE_CORE_IO_PACKET_VALUE_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/io/packet/base.hh>
#include <vidre/util/chrono.hh>
#include <vidre/util/uuid.hh>

#include <variant>
#include <string>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-packet-object
         * */
        ///@{

        /** Represents a value packet in the VidRe framework.
         *
         * This class encapsulates various types of values, including plain data types
         * such as boolean, integer, natural, number, string, binary, UUID, ISO date,
         * timestamp, and UTC time. It provides mechanisms for emitting and absorbing
         * these values to and from memory views.
         *
         * @note Except for the nil and binary type, all other types have a defined memory
         *       representation. In case of the binary type, the data is represented as a
         *       memory view, therefore, not owned.
         *
         * @todo Need to check if non-owning binary type handling is correct.
         * */
        class VIDRE_CORE_VISIBILITY packet::value : public io::packet {
        public:
            /** Plain data types.
             * */
            enum class minor : uint16_t {
                // === 0x0000-0x000F reserved
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
                utctime = 0x0022,    ///< UTC time.
            };

            using value_type = std::variant<std::monostate,     // nil
                                            bool,               // boolean
                                            int64_t,            // integer
                                            uint64_t,           // natural
                                            double,             // number
                                            std::string,        // string
                                            memory::view,       // binary
                                            uuid,               // uuid
                                            chrono::isodate,    // isodate
                                            chrono::timestamp,  // timestamp
                                            chrono::utcstamp    // utctime
                                            >;

            ///@todo JSON and Lua have a "nil" - this needs to be represented in the framework

        public:
            value(const value &) = delete;
            value &operator=(const value &) = delete;
            value(value &&) = default;
            value &operator=(value &&) = default;
            value(const header &hdr_) noexcept : packet{hdr_} {}

            virtual ~value() = default;

            value(uint16_t stream = 0)  //
                : packet{packet::major::value, minor::nil, stream, 0}, value_{} {}
            value(bool v, uint16_t stream = 0) : packet{packet::major::value, minor::boolean, stream, sizeof(int64_t)}, value_{v} {}
            template <typename T>
            requires (std::is_integral_v<T> && std::is_signed_v<T>)
            value(T v, uint16_t stream = 0)
                : packet{packet::major::value, minor::integer, stream, sizeof(int64_t)}, value_{static_cast<int64_t>(v)} {}
            template <typename T>
            requires (std::is_integral_v<T> && std::is_unsigned_v<T>)
            value(T v, uint16_t stream = 0)
                : packet{packet::major::value, minor::natural, stream, sizeof(uint64_t)}, value_{static_cast<uint64_t>(v)} {}

            template <typename T>
            requires std::is_floating_point_v<T>
            value(T v, uint16_t stream = 0)  //
                : packet{packet::major::value, minor::number, stream, sizeof(double)}, value_{static_cast<double>(v)} {}

            value(const std::string &v, uint16_t stream = 0)
                : packet{packet::major::value, minor::string, stream, static_cast<uint32_t>(v.size())}, value_{v} {}
            value(const char *v, uint16_t stream = 0)
                : packet{packet::major::value, minor::string, stream, static_cast<uint32_t>(std::strlen(v))}, value_{v} {}

            value(const memory::view &v, uint16_t stream = 0)
                : packet{packet::major::value, minor::binary, stream, static_cast<uint32_t>(v.size())}, value_{v} {}

            value(const uuid &v, uint16_t stream = 0)
                : packet{packet::major::value, minor::uuid, stream, sizeof(uuid)}, value_{v} {}

            value(const chrono::isodate &v, uint16_t stream = 0)
                : packet{packet::major::value, minor::isodate, stream, sizeof(chrono::isodate)}, value_{v} {}

            value(const chrono::timestamp &v, uint16_t stream = 0)
                : packet{packet::major::value, minor::timestamp, stream, sizeof(chrono::timestamp)}, value_{v} {}

            value(const chrono::utcstamp &v, uint16_t stream = 0)
                : packet{packet::major::value, minor::utctime, stream, sizeof(chrono::utcstamp)}, value_{v} {}
            value(const chrono::clock::time_point &v, uint16_t stream = 0)
                : packet{packet::major::value, minor::utctime, stream, sizeof(chrono::utcstamp)}, value_{chrono::utcstamp(v)} {}

        protected:
            value_type value_;

        public:
            void update() noexcept override;
            result emit(io::sink &f) override;
            void absorb(memory::view view, std::endian endian = std::endian::native) override;
            settings::json to_json() const override;

        public:
            /** Get stored value.
             * @tparam T The type to retrieve the value as.
             * @return The stored value cast to the specified type.
             * */
            template <typename T>
            T get() const {
                return std::get<T>(value_);
            }
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_PACKET_VALUE_HH
