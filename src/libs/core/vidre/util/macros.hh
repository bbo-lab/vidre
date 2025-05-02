/* VidRe
 * Copyright (C) 2024-2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_UTIL_MODULE_HH
#define VIDRE_CORE_UTIL_MODULE_HH
#pragma once

/**@defgroup api-core-util-macro Macro utilities
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */
///@{

/** Stringify an expression.
 * Example:
 * @code
 * VIDRE_STRINGIFY(__LINE__)
 * @endcode
 * expands to
 * > "__LINE__"
 * @see
 *      VIDRE_STRINGIFY_EXPAND
 * */
#define VIDRE_STRINGIFY(EXPR) #EXPR

/** Stringify an expanded expression.
 * Example:
 * @code
 * VIDRE_STRINGIFY(__LINE__)
 * @endcode
 * expands to
 * > "42"
 * @see
 *      VIDRE_STRINGIFY
 * */
#define VIDRE_STRINGIFY_EXPAND(EXPR) VIDRE_STRINGIFY(EXPR)

///@}

#endif  // VIDRE_CORE_UTIL_MODULE_HH
