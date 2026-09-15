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
#ifndef VIDRE_CORE_PLUGIN_INFORMATION_HH
#define VIDRE_CORE_PLUGIN_INFORMATION_HH
#pragma once

#include <vidre/core/config.hh>

namespace vidre {

    namespace plugin {
        
        /**@addtogroup api-core-plugin
         * */
        ///@{

        /** Module information.
         *
         * @TBD{maybe it's better, to just hold a JSON string}
         *
         * @see VIDRE_MODULE
         * */
        struct VIDRE_CORE_VISIBILITY information {
            const char *name;     ///< Name of the module.
            const char *version;  ///< Module version.

            struct {
                const char *name;         ///< Organization name (e.g. "MPINB/BBO").
                const char *domain = {};  ///< Organization domain (e.g. "de.mpg.mpinb.bbo").
            } organization;

            const char *brief;             ///< Brief description of the module.
            const char *description = {};  ///< Detailed description of the module.

            const char *category;  ///< Module category (see FIXME).

            void (*init)() = {};  ///< Module initialization function.
        };

        /** Symbol name of the information structure.
         * */
#define VIDRE_PLUGIN_INFO VidRePlugin

        /** Declare a module.
         *
         * Usage:
         * @code
         * VIDRE_MODULE(
         *     .name = "MyModule",
         *     .version = "0.0",
         *     .organization = {
         *         .name = "MPINB",
         *         .domain = "de.mpg.mpinb"
         *     },
         *     .brief = "Example module that does nothing.",
         *     .category = "example"
         * );
         * @endcode
         *
         */
#define VIDRE_MODULE(...)                                 \
    struct vidre::plugin::information VIDRE_PLUGIN_INFO { \
        __VA_ARGS__                                       \
    }

        ///@}

    }  // namespace plugin

}  // namespace vidre

#endif  // VIDRE_CORE_PLUGIN_INFORMATION_HH
