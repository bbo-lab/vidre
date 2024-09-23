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
#ifndef VIDRE_CORE_PLUGIN_HH
#define VIDRE_CORE_PLUGIN_HH

/**@addtogroup api-core-plugin
 * @details
 * @fileinfo
 *
 * This header provides an implementation to declare, load and deal
 * with plugins, specifically but not limited to dynamic modules. 
 * It's implementation takes into account, that not all libraries, 
 * specifically proprietry software, are and will be distributed along 
 * VidRe.
 *
 * */

#include <vidre/config.hh>
#include <vidre/plugin/information.hh>
#include <vidre/plugin/dynamic.hh>

namespace vidre {

    /** Plugins.
     * @ingroup api-core-plugin
     * */
    namespace plugin {}  // namespace module

}  // namespace vidre

#endif  // VIDRE_CORE_PLUGIN_HH
