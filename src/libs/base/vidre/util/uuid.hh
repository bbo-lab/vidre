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
// @file
#pragma once
#ifndef VIDRE_BASE_CONFIG_HH
#define VIDRE_BASE_CONFIG_HH

#include <vidre/config.hh>

#include <uuid/uuid.h>
#include <string>
#include <stdexcept>

namespace vidre {
    /**@defgroup api-base-util-uuid UUID
     * @ingroup api-base-util
     * @details
     * @fileinfo
     * @see
     * https://en.wikipedia.org/wiki/Universally_unique_identifier
     * */
    ///@{

    /** UUIDs.
     *
     * This class wraps the [uuid](https://www.man7.org/linux/man-pages/man3/uuid.3.html) library.
     *
     * */
    class VIDRE_BASE_VISIBILITY UUID {
    public:
        /** UUID variant.
         * */
        enum class Variant {
            NCS = UUID_VARIANT_NCS,              ///< Apollo NCS variant (backwards compatibility).
            DCE = UUID_VARIANT_DCE,              ///< DCE 1.1 UUIDs (see [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)).
            Microsoft = UUID_VARIANT_MICROSOFT,  ///< Microsoft COM/DCOM variant (compatibility).
            Other = UUID_VARIANT_OTHER           ///< Reserved.
        };

        /** DCE variant type.
         * @see
         * https://datatracker.ietf.org/doc/html/rfc4122
         * */
        enum class Type {
            Time = UUID_TYPE_DCE_TIME,          ///< Time-/MAC-based.
            Security = UUID_TYPE_DCE_SECURITY,  ///< Security version, with embedded POSIX UIDs.
            MD5 = UUID_TYPE_DCE_MD5,            ///< Name-based with MD5 hashing.
            Random = UUID_TYPE_DCE_RANDOM,      ///< Random or pseudo-random.
            SHA1 = UUID_TYPE_DCE_SHA1           ///< Name-based with SHA1 hashing.
        };

        /** Generate random or time-based UUID.
         * This function generates a random UUID if a high-quality random
         * source is available, a time-/MAC-based UUID otherwise.
         * @see
         *      UUID::makeTime() <br>
         *      UUID::makeTimeSafe()
         * */
        static UUID make() noexcept {
            UUID result;
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
         * > [...]
         * > If neither of these two synchronization mechanisms can be used, it
         * > is theoretically possible that two concurrently running processes
         * > obtain the same UUID(s).
         * [man uuid_generate](https://www.man7.org/linux/man-pages/man3/uuid_generate.3.html)
         *
         * @see
         *      UUID::makeTimeSafe()
         * */
        static UUID makeTime() noexcept {
            UUID result;
            uuid_generate_time(result.uuid_);
            return result;
        }

        /** Safely create a time- and MAC-based UUID.
         *
         * This function uses synchronization mechanisms to avoid that identical
         * UUIDs are generated on a local machine.
         *
         * @throws std::runtime_error() if synchronization fails
         * @see
         *      UUID::makeTime()
         * */
        static UUID makeTimeSafe() {
            UUID result;
            if (not uuid_generate_time_safe(result.uuid_))
                throw std::runtime_error("synchronization failed");
            return result;
        }

        /** Create a random or pseudo-random UUID.
         * */
        static UUID makeRandom() noexcept {
            UUID result;
            uuid_generate_random(result.uuid_);
            return result;
        }

        /** Create a name-based UUID with MD5 hashing.
         * */
        static UUID makeMD5(const UUID ns, const std::string &name) noexcept {
            UUID result;
            uuid_generate_md5(result.uuid_, ns.uuid_, name.data(), name.size());
            return result;
        }

        /** Create a name-based UUID with SHA-1 hashing.
         * */
        static UUID makeSHA1(const UUID ns, const std::string &name) noexcept {
            UUID result;
            uuid_generate_sha1(result.uuid_, ns.uuid_, name.data(), name.size());
            return result;
        }

    public:
        UUID() = default;
        UUID(const std::string &s);

    protected:
        uuid_t uuid_{};

    protected:
        void parse(const std::string &s);

    public:
        /** Get variant.
         * */
        operator Variant() const noexcept {
            return static_cast<Variant>(uuid_variant(uuid_));
        }

        /** Get DCE type.
         * */
        operator Type() const noexcept {
            return static_cast<Type>(uuid_type(uuid_));
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
        bool operator==(const UUID &other) const noexcept {
            return uuid_compare(uuid_, other.uuid_) == 0;
        }
        bool operator==(const std::string &s) const noexcept {
            return std::string(*this) == s;
        }
    };

    ///@}
}  // namespace vidre

#endif  // VIDRE_BASE_CONFIG_HH
