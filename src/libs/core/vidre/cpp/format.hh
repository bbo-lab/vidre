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
#ifndef VIDRE_CORE_CPP_FORMAT_HH
#define VIDRE_CORE_CPP_FORMAT_HH
#pragma once

#include <vidre/config.hh>

///@cond

#if defined(VIDRE_HAVE_CPP_LIB_FORMAT) and VIDRE_HAVE_CPP_LIB_FORMAT
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

}  // namespace std
#endif
///@endcond

#endif
