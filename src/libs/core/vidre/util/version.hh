/* VidRe
 * Copyright (C) 2025-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_UTIL_VERSION_HH
#define VIDRE_CORE_UTIL_VERSION_HH
#pragma once

/**@defgroup api-core-util-version Version handling
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>
#include <vidre/error.hh>

#include <compare>
#include <string_view>

/**@addtogroup api-core-util-version */
///@{

/** Macro for calculating an integral value from a version.
 * */
#define VIDRE_RELEASE_VALUE(MAJOR, MINOR, PATCH) (((MAJOR * 100) + MINOR) * 1000 + PATCH)

/** Integral version number.
 * @see VIDRE_RELEASE_VALUE
 * */
#define VIDRE_RELEASE VIDRE_RELEASE_VALUE(VIDRE_VERSION_MAJOR, VIDRE_VERSION_MINOR, VIDRE_VERSION_PATCH)

///@}

namespace vidre {

    /**@addtogroup api-core-util-version
     * */
    ///@{

    /** Version class for handling version information.
     * */
    class VIDRE_CORE_VISIBILITY version {
    public:
        /** Undefined version component value.
         * */
        static constexpr unsigned undefined = static_cast<unsigned>(-1);

        /** VidRe version.
         * */
        static version vidre;

    public:
        constexpr version() noexcept = default;
        constexpr version(uint32_t def[2]) noexcept : major_{def[0]}, minor_{def[1]} {}
        constexpr version(unsigned major, unsigned minor = undefined, unsigned patch = undefined) noexcept
            : major_{major}, minor_{minor}, patch_{patch} {}

        /** String constructor.
         * The string should be in the format "MAJOR.MINOR.PATCH", where MINOR and PATCH
         * are optional. If a component is missing, it will be set to `undefined`.
         *
         * If MINOR or PATCH are omitted, the version string must not contain any additional characters.
         *
         * Punctiuation and suffixes (e.g., "1.2.3-alpha") are allowed after the PATCH component,
         * but will be ignored.
         * */
        version(std::string_view str);

    protected:
        unsigned major_{undefined};
        unsigned minor_{undefined};
        unsigned patch_{undefined};

    protected:
        // Parses an unsigned integer from the string starting at `it` and advances `it` to the position after the number.
        unsigned parse(std::string_view::iterator &it, std::string_view::iterator end) noexcept {
            unsigned val = 0;

            if ((*it < '0') or (*it > '9'))
                return undefined;

            while (it != end) {
                char c = *it;
                if ((c < '0') or (c > '9'))
                    break;
                val = val * 10 + (c - '0');
                ++it;
            }

            return val;
        }

    public:
        /** Check if valid.
         * */
        constexpr bool valid() const {
            return major_ != undefined;
        }

        /** Get major version.
         * */
        constexpr unsigned major() const {
            return major_;
        }

        /** Get minor version.
         * */
        constexpr unsigned minor() const {
            return minor_;
        }

        /** Get patch version.
         * */
        constexpr unsigned patch() const {
            return patch_;
        }

        /** Check for equality.
         * */
        constexpr bool operator==(const version &other) const {
            if ((major_ == undefined) || (other.major_ == undefined))
                return false;
            else if (major_ != other.major_)
                return false;

            if ((minor_ == undefined) || (other.minor_ == undefined))
                return true;
            else if (minor_ != other.minor_)
                return false;

            if ((patch_ == undefined) || (other.patch_ == undefined))
                return true;

            return patch_ == other.patch_;
        }

        /** Check for inequality.
         * */
        constexpr bool operator!=(const version &other) const {
            if (major_ == undefined || other.major_ == undefined)
                return false;
            else if (major_ == other.major_)
                return false;

            if ((minor_ == undefined) || (other.minor_ == undefined))
                return true;
            else if (minor_ == other.minor_)
                return false;

            if ((patch_ == undefined) || (other.patch_ == undefined))
                return true;

            return patch_ != other.patch_;
        }

        /** Three-way comparison.
         *
         * The comparison is performed component-wise, starting with the major version,
         * then minor, and finally patch. If a component is `undefined` in either version,
         * it is considered less than any defined value for that component. If both components
         * are `undefined`, they are considered equal.
         *
         * @note If one of the versions is not valid (ie. the major version is undefined),
         *       the result is unspecified.
         * */
        constexpr std::strong_ordering operator<=>(const version &other) const noexcept{
            if ((major_ == undefined) or (other.major_ == undefined))
                abort_with_message("implementation error :invalid version comparison - one of the versions is undefined");

            if (auto cmp = major_ <=> other.major_; cmp != 0)
                return cmp;

            if ((minor_ == undefined) or (other.minor_ == undefined))
                return std::strong_ordering::equal;
            else if (auto cmp = minor_ <=> other.minor_; cmp != 0)
                return cmp;

            if ((patch_ == undefined) or (other.patch_ == undefined))
                return std::strong_ordering::equal;

            return patch_ <=> other.patch_;
        }
    };

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_UTIL_VERSION_HH
