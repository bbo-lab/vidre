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
#ifndef VIDRE_CORE_PLUGIN_HUB_HH
#define VIDRE_CORE_PLUGIN_HUB_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/plugin/exception.hh>
#include <vidre/settings.hh>

#include <filesystem>

/* TODO
 *  - decide on a config file format (JSON)
 *    -> must list, e.g. for proprietry software, were to download to enable the feature
 *  - decide category/directory structure, e.g.
 *    device/
 *        camera/
 *            basler/
 *            onsemi/
 *            uvc/
 *    filter/
 *    encode/
 *        lossless/
 *        mp4/
 *    etc.
 */

namespace vidre {

    namespace plugin {

        /**@addtogroup api-core-plugin
         * */
        ///@{

        /** Plugin discovery and handling.
         *
         * FIXME
         *
         * */
        struct VIDRE_CORE_VISIBILITY hub {
            static std::filesystem::path plugindir;
            static std::filesystem::path scriptdir;

            /** Find all modules.
             * */
            static settings::json find() noexcept;

        protected:
            // FIXME
        };

        ///@}

    }  // namespace plugin

}  // namespace vidre

#endif  // VIDRE_CORE_PLUGIN_HUB_HH
