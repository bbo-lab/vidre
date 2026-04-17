/* VidRe
 * Copyright (C) 2025-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_MEMORY_HH
#define VIDRE_CORE_MEMORY_HH
#pragma once

/**@addtogroup api-core-memory
 * @details
 * @fileinfo
 * 
 * The C++-API of VidRe provides several ways of managing memory, especially 
 * those for recording data. 
 * 
 * FIXME 
 * - camera drivers provide their own "buffer factories" vs.
 * - we provide a memory factory tailored for data recording devices...
 * - etc.
 * 
 * @{
 * @namespace vidre::memory Memory handling.
 * @defgroup api-core-memory-base Basic types and utilities
 * @defgroup api-core-memory-chunk Data chunk handling
 * @defgroup api-core-memory-details Implementation details
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/memory/chunk.hh>

#endif  // VIDRE_CORE_MEMORY_HH