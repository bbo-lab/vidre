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
 * The core library provides interfaces for YAML, JSON and Lua. Each
 * format has it's advantages and disadvantages:
 * 
 * **JSON** is lean and fast, especially useful for setting up devices
 * and transporting serialized data.
 * 
 * **YAML** is (as long as one stays away from its <a href="https://noyaml.com/">short-comings</a>).
 * However, in contrast to JSON, it can be commented.
 * 
 * **Lua**, finally, provides the potential for complex configuration scripts
 * (for example camera definitions), that allows interaction with the vidre
 * libraries, plugins, etc. 
 *
 * @internal
 *
 * @endinternal
 * 
 * @{
 * @defgroup api-core-settings-json JSON
 * @defgroup api-core-settings-yaml YAML
 * @defgroup api-core-settings-lua Lua
 * @}
 * 
 * @see @ref examples/core/settings.cc
 * */

#include <vidre/core/config.hh>
#include <vidre/script.hh>
#include <vidre/settings/lua.hh>
#include <vidre/settings/json.hh>
#include <vidre/settings/yaml.hh>

namespace vidre {

    /** Settings/configuration related.
     * @ingroup api-core-settings
     * */
    namespace settings {}  // namespace settings

}  // namespace vidre

#endif  // VIDRE_CORE_SETTINGS_HH
