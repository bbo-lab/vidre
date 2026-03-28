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
#ifndef VIDRE_CORE_SETTINGS_JSON_HH
#define VIDRE_CORE_SETTINGS_JSON_HH
#pragma once

#include <vidre/core/config.hh>

#include <nlohmann/json.hpp>

namespace vidre {

    namespace settings {

        /**@addtogroup api-core-settings
         * */
        ///@{

        /** Variant (JSON node).
         * @see @ref examples/core/settings.cc
         * @see https://json.nlohmann.me/
         * */
        using json = nlohmann::json;

        ///@}

    }  // namespace settings

}  // namespace vidre

#endif  // VIDRE_CORE_SETTINGS_JSON_HH
