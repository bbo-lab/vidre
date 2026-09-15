/* VidRe
 * Copyright (C) 2024-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_ERROR_HH
#define VIDRE_CORE_ERROR_HH
#pragma once

/**@addtogroup api-core-error
 * @details
 * @fileinfo
 *
 * There are many pro's and con's for using exceptions versus error return
 * types—which we shall not repeat here. We should note, though, that the
 * mantra that exceptions are generally slower is [untenable](https://www.youtube.com/watch?v=bY2FlayomlE).
 *
 * Within VidRe, exceptions are used for transporting terse error reporting
 * information, and, therefore, should not be used in fast code paths (use
 * @c std::optional and @c std::expected there). For general handling errors
 * in the program flow, rather use `status` and `result<>`.
 *
 * @{
 * @namespace vidre::error Exception and errors.
 * @defgroup api-core-error-flow Program flow.
 * @defgroup api-core-error-exceptions Exceptions.
 * @defgroup api-core-error-details Implementation details.
 * @}
 * */
#include <vidre/core/config.hh>
#include <vidre/error/details.hh>
#include <vidre/error/flow.hh>
#include <vidre/error/exceptions.hh>

#endif  // VIDRE_CORE_ERROR_HH
