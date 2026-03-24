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
#ifndef VIDRE_CORE_SETTINGS_HH
#define VIDRE_CORE_SETTINGS_HH
#pragma once

/**@addtogroup api-core-settings
 * @details
 * @fileinfo
 *
 * Configuration files are, as also stated elsewhere, implemented using Lua
 * (see @ref api-core-script).
 * The advantage of using Lua as a configuration language is, that it also
 * allows to implement complex functions, such as is required for abstracting
 * camera interfaces, where, for example setting the frame rate or exposure time,
 * would require, first to do calculations and, second, update multiple registers
 * at once.
 *
 * @internal
 *
 * @endinternal
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/script.hh>
#include <vidre/settings/node.hh>
#include <vidre/settings/file.hh>

namespace vidre {

    /** Settings/configuration related.
     * @ingroup api-core-settings
     * */
    namespace settings {}  // namespace settings

}  // namespace vidre

#endif  // VIDRE_CORE_SETTINGS_HH
