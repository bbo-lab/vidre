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
#ifndef VIDRE_CORE_SETTINGS_YAML_HH
#define VIDRE_CORE_SETTINGS_YAML_HH
#pragma once

/**@addtogroup api-core-settings-yaml
 * @details
 * @fileinfo
 * 
 * FIXME
 * 
 * */

/*@todo replace with https://github.com/biojppm/rapidyaml ???*/

#include <vidre/core/config.hh>

#include <yaml-cpp/yaml.h>

namespace vidre {

    namespace settings {

        /** YAML namespace.
         * @ingroup api-core-settings-yaml
         * */
        namespace yaml {

            /**@addtogroup api-core-settings-yaml
             * */
            ///@{

            /** A YAML node.
             * @see https://codedocs.xyz/jbeder/yaml-cpp/index.html
             * */
            using node = YAML::Node;

            // FIXME

            ///@}

        }  // namespace yaml

    }  // namespace settings

}  // namespace vidre

#endif  // VIDRE_CORE_SETTINGS_YAML_HH
