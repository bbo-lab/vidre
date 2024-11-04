/* VidRe
 * Copyright (C) 2024 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_UTIL_UUID_HH
#define VIDRE_CORE_UTIL_UUID_HH
#pragma once

#include <vidre/core/config.hh>

#include <uuid/uuid.h>
#include <string>
#include <stdexcept>

namespace vidre {
    /**@defgroup api-core-util-uuid UUID
     * @ingroup api-core-util
     * @details
     * @fileinfo
     * 
     * <b>Example</b>:
     * @include examples/core/uuid.cc
     * 
     * @see
     * https://en.wikipedia.org/wiki/Universally_unique_identifier
     * */
    ///@{

    /** UUIDs.
     *
     * This class wraps the [uuid](https://www.man7.org/linux/man-pages/man3/uuid.3.html) library.
     * 
     * @see
     *      [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)
     * */
    class VIDRE_CORE_VISIBILITY uuid {
    public:
        /** UUID variant.
         * @see
         *      [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)
         * */
        enum class variant {
            NCS = UUID_VARIANT_NCS,              ///< Apollo NCS variant (backwards compatibility).
            DCE = UUID_VARIANT_DCE,              ///< DCE 1.1 UUIDs (see [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)).
            Microsoft = UUID_VARIANT_MICROSOFT,  ///< Microsoft COM/DCOM variant (compatibility).
            Other = UUID_VARIANT_OTHER           ///< Reserved.
        };

        /** DCE variant type.
         * @see
         *      [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)
         * */
        enum class type {
            NIL = UUID_TYPE_DCE_NIL,            ///< "NIL" (refers to the ["Nil UUID"](https://en.wikipedia.org/wiki/Universally_unique_identifier#Nil_UUID)).
            Time = UUID_TYPE_DCE_TIME,          ///< Time-/MAC-based.
            Security = UUID_TYPE_DCE_SECURITY,  ///< Security version, with embedded POSIX UIDs.
            MD5 = UUID_TYPE_DCE_MD5,            ///< Name-based with MD5 hashing.
            Random = UUID_TYPE_DCE_RANDOM,      ///< Random or pseudo-random.
            SHA1 = UUID_TYPE_DCE_SHA1           ///< Name-based with SHA1 hashing.
        };

        /** Generate random or time-based UUID.
         * 
         * This function generates a random UUID if a high-quality random
         * source is available, a time-/MAC-based UUID otherwise.
         * 
         * @see
         *      uuid::makeTime() <br/>
         *      uuid::makeTimeSafe() <br/>
         *      uuid::makeRandom()
         * */
        static uuid make() noexcept {
            uuid result;
            uuid_generate(result.uuid_);
            return result;
        }

        /** Create a time- and MAC-based UUID.
         *
         * > This function forces the use of the alternative algorithm which uses
         * > the current time and the local ethernet MAC address (if available).
         * > This algorithm used to be the default one used to generate UUIDs,
         * > but because of the use of the ethernet MAC address, it can leak
         * > information about when and where the UUID was generated.
         * (see [uuid_generate_time man-page](https://www.man7.org/linux/man-pages/man3/uuid_generate.3.html))
         * 
         * Note, however, that uniqueness of UUIDs is not guaranteed.
         *
         * @see
         *      uuid::makeTimeSafe()
         * */
        static uuid makeTime() noexcept {
            uuid result;
            uuid_generate_time(result.uuid_);
            return result;
        }

        /** Safely create a time- and MAC-based UUID.
         *
         * @throws std::runtime_error() if synchronization fails
         * 
         * This function uses synchronization mechanisms to ensure uniqueness 
         * of time-based UUIDs
         *
         * @see
         *      uuid::makeTime()
         * */
        static uuid makeTimeSafe() {
            uuid result;
            if (not uuid_generate_time_safe(result.uuid_))
                throw std::runtime_error("synchronization failed");
            return result;
        }

        /** Create a random or pseudo-random UUID.
         * */
        static uuid makeRandom() noexcept {
            uuid result;
            uuid_generate_random(result.uuid_);
            return result;
        }

        /** Create a name-based UUID with MD5 hashing.
         * */
        static uuid makeMD5(const uuid ns, const std::string &name) noexcept {
            uuid result;
            uuid_generate_md5(result.uuid_, ns.uuid_, name.data(), name.size());
            return result;
        }

        /** Create a name-based UUID with SHA-1 hashing.
         * */
        static uuid makeSHA1(const uuid ns, const std::string &name) noexcept {
            uuid result;
            uuid_generate_sha1(result.uuid_, ns.uuid_, name.data(), name.size());
            return result;
        }

    public:
        uuid() = default;
        uuid(const std::string &s);

    protected:
        uuid_t uuid_{};

    protected:
        void parse(const std::string &s);

    public:
        /** Implicit variant conversion.
         * 
         * Usage:
         * @code
         * void fn(const uuid &uuid) {
         *     if (uuid == uuid::Variant::DCE) {
         *         // ...
         *     }
         * }
         * @endcode
         * */
        operator variant() const noexcept {
            return static_cast<variant>(uuid_variant(uuid_));
        }

        /** Implicit DCE type conversion.
         * @see
         *     `uuid::operator Variant()`
         * */
        operator type() const noexcept {
            return static_cast<type>(uuid_type(uuid_));
        }

        /** Convert to string.
         * @param lowercase Output with lower-case letters (default: false).
         * */
        std::string tostring(bool lowercase = false) const noexcept;

        /** Coerce to string.
         * */
        operator std::string() const noexcept {
            return tostring();
        }

        /** Equality.
         * */
        bool operator==(const uuid &other) const noexcept {
            return uuid_compare(uuid_, other.uuid_) == 0;
        }
        bool operator==(const std::string &s) const noexcept {
            return std::string(*this) == s;
        }
    };

    ///@}
}  // namespace vidre

#endif  // VIDRE_CORE_UTIL_UUID_HH
