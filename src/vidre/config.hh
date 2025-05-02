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
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CONFIG_HH
#define VIDRE_CONFIG_HH
#pragma once

/** General namespace.
 * @ingroup api */
namespace vidre {}

/**@addtogroup api-config
 * @details
 * @fileinfo
 * 
 * Compile-time support macros (such as version information or ABI related
 * declarations).
 * */

/**@defgroup api-config-version Version information
 * @ingroup api-config
 * */
///@{

/** Major version. */
#define VIDRE_VERSION_MAJOR <…>

/** Minor version. */
#define VIDRE_VERSION_MINOR <…>

/** Version string. */
#define VIDRE_VERSION_S "<…>.<…>"

///@}

/**@defgroup api-config-abi API/ABI and system settings
 * @ingroup api-config
 * */
///@{

/** Import library symbols. */
#define VIDRE_API_IMPORT <…>

/** Export library symbols. */
#define VIDRE_API_EXPORT <…>

/** Export library symbols. */
#define VIDRE_API_MODULE <…>

/** File extension for dynamically linked libraries.  */
#define VIDRE_DLL_EXTENSION "<…>"

///@}

/**@defgroup api-config-have Software support flags.
 * @ingroup api-config
 * @details
 * @fileinfo
 * */
///@{
/** Flag if libuuid is used. */
#define VIDRE_HAVE_LIBUUID <0/1>
///@}

#endif // VIDRE_CONFIG_HH
