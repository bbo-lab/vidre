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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_PLUGIN_HH
#define VIDRE_CORE_PLUGIN_HH
#pragma once

/**@addtogroup api-core-plugin
 * @details
 * @fileinfo
 *
 * This header provides an implementation to declare, load and deal with plugins, 
 * specifically but not limited to "dynamic" modules (i.e. "DLL"s).
 *
 * @see vidre::plugin::hub
 * @see @ref examples/core/plugins.cc
 *
 * @{
 * @defgroup api-core-plugin-category Plugin categories.
 * @defgroup api-core-plugin-details Implementation details
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/plugin/exception.hh>
#include <vidre/plugin/types.hh>

// FIXME
#include <vidre/plugin/information.hh>
#include <vidre/plugin/native.hh>


namespace vidre {

    namespace plugin {}  // namespace plugin

}  // namespace vidre

#endif  // VIDRE_CORE_PLUGIN_HH
