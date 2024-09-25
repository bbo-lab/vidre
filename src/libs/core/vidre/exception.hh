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
#ifndef VIDRE_CORE_EXCEPTION_HH
#define VIDRE_CORE_EXCEPTION_HH

/**@addtogroup api-core-exception
 * @details
 * @fileinfo
 *
 * @TBD{needs a pulled-in version of https://www.boost.org/doc/libs/1_75_0/libs/exception/doc/tutorial_transporting_data.html}
 *
 * */
#include <vidre/config.hh>

#include <boost/exception/all.hpp>

#include <source_location>

namespace vidre {
    /**@addtogroup api-core-exception
     * */
    ///@{

    template <class Tag, class T>
    using error_info = boost::error_info<Tag, T>;

    /// ...

    /** Default VidRe exception.
     *
     * @TBD{add std::source_location}
     * */
    class VIDRE_CORE_VISIBILITY exception
        : virtual boost::exception
        , virtual std::exception {
    public:
        exception(const std::source_location location = std::source_location::current()) {
            throw_function_ = location.function_name();
        }
    };

    ///@}
}  // namespace vidre

#endif  // VIDRE_CORE_EXCEPTION_HH
