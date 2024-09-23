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
#ifndef VIDRE_BASE_MODULE_DECLARE_HH
#define VIDRE_BASE_MODULE_DECLARE_HH

#include <vidre/base/config.hh>

namespace vidre {

    namespace module {
        /**@addtogroup api-base-module
         * */
        ///@{

        /* TODO
         *  - decide on a config file format (JSON)
         *    -> must list, e.g. for proprietry software, were to download to enable the feature
         *  - decide category/directory structure, e.g.
         *    device/
         *        pylon/
         *        onsemi/
         *    filter/
         *    endcode/
         *        lossless/
         *        mp4/
         *    etc.
         */

        /** Module information.
         *
         * TBD
         *
         * */
        struct VIDRE_BASE_VISIBILITY Information {
            const char *name;     ///< Name of the module.
            const char *version;  ///< Module version.

            struct {
                const char *name;         ///< Organization name (e.g. "MPINB/BBO").
                const char *domain = {};  ///< Organization domain (e.g. "de.mpg.mpinb.bbo").
            } organization;

            const char *brief;             ///< Brief description of the module.
            const char *description = {};  ///< Detailed description of the module.

            const char *category;  ///< Module category (see FIXME).
        };

        /** Symbol name of the information structure.
         * */
#define VIDRE_MODULE_INFO VidReModule

        /** Declare a module.
         *
         * Usage:
         * @code
         * VIDRE_MODULE(
         *     .name = "MyModule",
         *     .version = "0.0",
         *     .organization = {
         *         .name = "George"
         *     },
         *     .brief = "Example module that does nothing."
         * );
         * @endcode
         *
         */
#define VIDRE_MODULE(...)                                                  \
    VIDRE_API_MODULE struct vidre::module::Information VIDRE_MODULE_INFO { \
        __VA_ARGS__                                                        \
    }

        ///@}

    }  // namespace module

}  // namespace vidre

#endif  // VIDRE_BASE_MODULE_DECLARE_HH
