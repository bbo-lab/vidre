/* VidRe
 * Copyright (C) 2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
///@file
#ifndef VIDRE_CORE_CPP_FORMAT_HH
#define VIDRE_CORE_CPP_FORMAT_HH
#pragma once

#include <vidre/config.hh>

/**@defgroup api-core-cpp-format Format library
 * @ingroup api-core-cpp
 * @details
 * @fileinfo
 * 
 * On systems where @c \<format\> is not available the [fmtlib](https://github.com/fmtlib/fmt) is used. 
 * 
 * Available functionality:
 * - [std::format](https://en.cppreference.com/w/cpp/utility/format/format)
 * - [std::format_string](https://en.cppreference.com/w/cpp/utility/format/basic_format_string) 
 * 
 * */
///@{

#if VIDRE_USE_CPP_LIB_FORMAT
#    include <format>
#else
#    include <fmt/format.h>
namespace std {

    template <typename... T>
    [[nodiscard]] inline auto format(fmt::format_string<T...> fmt, T &&...args) -> std::string {
        return fmt::vformat(fmt, fmt::make_format_args(args...));
    }

    template <typename... T>
    using format_string = fmt::format_string<T...>;

    template<typename T, class Char>
    concept formattable = std::is_class<fmt::formatter<T>>; // FIXME

}  // namespace std
#endif

///@}

#endif
