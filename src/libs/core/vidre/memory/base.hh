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
#ifndef VIDRE_CORE_MEMORY_BASE_HH
#define VIDRE_CORE_MEMORY_BASE_HH
#pragma once

/**@addtogroup api-core-memory-base
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>

namespace vidre {

    namespace memory {

        /**@addtogroup api-core-memory-base
         * */
        ///@{

        using size_type = std::size_t;
        using value_type = uint8_t;
        using pointer = uint8_t *;
        using const_pointer = const uint8_t *;

        ///@}
        
    }  // namespace memory

}  // namespace vidre

#endif  // VIDRE_CORE_MEMORY_BASE_HH