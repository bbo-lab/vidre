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
#ifndef VIDRE_CORE_UTIL_STATUS_HH
#define VIDRE_CORE_UTIL_STATUS_HH
#pragma once

/**@defgroup api-core-util-status Program flow status
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>

namespace vidre {

    /**@addtogroup api-core-util-status
     * */
    ///@{

    /** Program flow status.
     *
     * This is a simple wrapper around a boolean value, to make it more explicit
     * that a function returns a status (i.e. success or failure) rather than a
     * value.
     *
     * */
    class VIDRE_CORE_VISIBILITY status {
    public:
        enum status_type {
            good = 0,
            bad = 1
        };

    public:
        status() = default;
        status(const status &) = default;
        status &operator=(const status &) = default;
        status(status &&) noexcept = default;
        status &operator=(status &&) noexcept = default;
        status(status_type s) : status_(s) {}

    protected:
        status_type status_ = bad;

    public:
        /** Convert the status to a boolean value.
         * @return true if the status is good, false otherwise.
         * Example:
         * @code
         * if (some_function() == status::bad) {
         *     // handle error
         * }
         * @endcode
         * which is equivalent to
         * @code
         * if (not some_function()) {
         *     // handle error
         * }
         * @endcode
         * */
        operator bool() const noexcept {
            return status_ == good;
        }

        /** Implicit conversion to status type.
         * @return The status type (good or bad).
         * */
        operator status_type() const noexcept {
            return status_;
        }

        /** Boolean negation.
         * @return true if the status is bad, false otherwise.
         * */
        bool operator!() const noexcept {
            return status_ == bad;
        }
    };

    ///@}

}  // namespace vidre

#endif  //  VIDRE_CORE_UTIL_STATUS_HH