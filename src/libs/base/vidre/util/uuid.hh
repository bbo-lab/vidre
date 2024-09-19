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

/**@defgroup api-base-util-uuid UUID
 * @ingroup api-base-util
 * @details
 * @fileinfo
 *
 * TODO
 *
 * */
///@{

#include <uuid/uuid.h>
#include <string>

namespace vidre {

    /** UUIDs.
     * @see
     * https://en.wikipedia.org/wiki/Universally_unique_identifier
     * */
    class VIDRE_BASE_VISIBILITY UUID {
    public:
        /** UUID variant.
         * */
        enum class Variant {
            NCS = UUID_VARIANT_NCS,
            DCE = UUID_VARIANT_DCE,
            Microsoft = UUID_VARIANT_MICROSOFT,
            Other = UUID_VARIANT_OTHER
        };

        /** DCE variant type.
         * */
        enum class Type {
            NIL = UUID_TYPE_DCE_NIL,
            Time = UUID_TYPE_DCE_TIME,
            Security = UUID_TYPE_DCE_SECURITY,
            MD5 = UUID_TYPE_DCE_MD5,
            Random = UUID_TYPE_DCE_RANDOM,
            SHA1 = UUID_TYPE_DCE_SHA1
        };

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

}  // namespace vidre

///@}

#endif  // VIDRE_BASE_CONFIG_HH
