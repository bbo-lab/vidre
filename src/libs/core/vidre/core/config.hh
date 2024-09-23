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
#ifndef VIDRE_CORE_CONFIG_HH
#define VIDRE_CORE_CONFIG_HH

#include <vidre/config.hh>

/**@addtogroup api-core-details
 * @details
 * @fileinfo
 * 
 * TODO
 * 
 * */
///@{

/** Declaration specification for import/export for the base library.
 * 
 * This macro is needed for compiling on Windows®.
 * 
 * Usage:
 * @code
 * class VIDRE_CORE_VISIBILITY MyClass {
 * public:
 *     MyClass() = default;
 * };
 * @endcode
 * 
 * @see
 *      [dllexport,dllimport](https://learn.microsoft.com/en-us/cpp/cpp/dllexport-dllimport?view=msvc-170)
 * */
#if not defined(VIDRE_CORE_VISIBILITY)
#define VIDRE_CORE_VISIBILITY VIDRE_API_IMPORT
#endif

///@}

#endif // VIDRE_CORE_CONFIG_HH
