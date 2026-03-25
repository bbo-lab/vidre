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
#ifndef VIDRE_ONSEMI_MIDLIB2_TYPES_HH
#define VIDRE_ONSEMI_MIDLIB2_TYPES_HH
#pragma once

#include <vidre/onsemi/midlib2/config.hh>

/**@addtogroup api-device-onsemi-types
 * */
///@{

struct VIDRE_API_IMPORT mi_camera_t;

///@}

namespace vidre {

    namespace midlib2 {

        /**@addtogroup api-device-onsemi-types
         * */
        ///@{

        /** Internal camera device representation.
         * */
        using device = ::mi_camera_t *;

        ///@}

    }  // namespace midlib2

}  // namespace vidre

#endif  // VIDRE_ONSEMI_MIDLIB2_TYPES_HH
