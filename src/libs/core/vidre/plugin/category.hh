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
#ifndef VIDRE_CORE_PLUGIN_CATEGORY_HH
#define VIDRE_CORE_PLUGIN_CATEGORY_HH
#pragma once

/**@addtogroup api-core-plugin-category
 * @details
 * @fileinfo
 *
 * VidRe supports several types of plugins, which are categorized by their purpose
 * and/or technical implementation.
 *
 * FIXME
 *
 * */

#include <vidre/core/config.hh>

#include <string_view>

namespace vidre {

    namespace plugin {

        /**@addtogroup api-core-plugin-category
         * */
        ///@{

        /** Plugin category.
         *
         * The plugin category only describes the technical implementation of a plugin,
         * not its purpose or capabilities - only distinguishing how it is loaded.
         * This also requires that the plugin description is formatted in a specific way.
         *
         * FIXME
         *
         * */
        struct category {
            enum type {
                native,  ///< Compiled and dynamically-linked loadable module.
                script,  ///< Script-based plugin.
                program  ///< Other plugin type.
            };

            static std::string_view name(type t) noexcept;
            static type id(std::string_view name) noexcept;
        };

        ///@}

    }  // namespace plugin

}  // namespace vidre

#endif  // VIDRE_CORE_PLUGIN_INFORMATION_HH
