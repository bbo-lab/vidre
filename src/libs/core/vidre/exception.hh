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
 * There are many pro's and con's for using exceptions versus error return 
 * types—which we shall not repeat here. As a rule of thumb: do not use
 * exceptions in fast computational paths (where stack unwinding is an 
 * unpredictable cost), but use it whereever more information can be transmitted 
 * with an exception than with a simple error code.
 * 
 * @TBD{Either we need a pulled-in version of https://www.boost.org/doc/libs/1_75_0/libs/exception/doc/tutorial_transporting_data.html
 *      and the rest of it's exception interface, or we implement our own
 *      "arbitrary data passing". I should note also, that after some trials
 *      the Boost API seems quite cumbersome...}
 *
 * */
#include <vidre/core/config.hh>

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
     * */
    class VIDRE_CORE_VISIBILITY exception
        : public virtual boost::exception
        , public virtual std::exception {
    public:
        exception(const std::source_location location = std::source_location::current());
    };

    inline exception::exception(const std::source_location location) : boost::exception(), std::exception() {
        (*this)                                                 //
            << boost::throw_function(location.function_name())  //
            << boost::throw_file(location.file_name())          //
            << boost::throw_line(location.line());
    }

    ///@}
}  // namespace vidre

#endif  // VIDRE_CORE_EXCEPTION_HH
