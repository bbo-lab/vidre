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
/// @file
#ifndef VIDRE_CORE_UTIL_VERSION_HH
#define VIDRE_CORE_UTIL_VERSION_HH
#pragma once

#include <vidre/core/config.hh>

/**@defgroup api-core-util-version Version macros
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */
///@{

/** Macro for calculating an integral value from a version. 
 * */
#define VIDRE_RELEASE_VALUE(MAJOR, MINOR, PATCH) (((MAJOR * 100) + MINOR) * 1000 + PATCH)

/** Integral version number.
 * @see VIDRE_RELEASE_VALUE
 * */
#define VIDRE_RELEASE VIDRE_RELEASE_VALUE(VIDRE_VERSION_MAJOR, VIDRE_VERSION_MINOR, VIDRE_VERSION_PATCH)


///@}

#endif  // VIDRE_CORE_UTIL_VERSION_HH
