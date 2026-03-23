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
#ifndef VIDRE_CORE_SETTINGS_NODE_HH
#define VIDRE_CORE_SETTINGS_NODE_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/script.hh>
#include <vidre/settings/node.hh>

namespace vidre {

    namespace settings {

        /**@addtogroup api-core-settings
         * */
        ///@{

        /** Configuration file.
         *
         * Configuration nodes represent table in the internal Lua script
         * interface.
         *
         * FIXME
         *
         * */
        class VIDRE_CORE_VISIBILITY file : public node {
        public:
            file() = default;

        public:
        };

        ///@}

    }  // namespace settings

}  // namespace vidre

#endif  // VIDRE_CORE_SETTINGS_HH
