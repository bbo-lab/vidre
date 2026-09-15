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
#ifndef VIDRE_CORE_STORAGE_HH
#define VIDRE_CORE_STORAGE_HH
#pragma once

/**@addtogroup api-core-storage
 * @brief Data storage and file formats.
 * @details
 * @fileinfo
 *
 * VidRe provides a custom raw file format for storing recorded data. The raw file 
 * format is designed to be flexible and extensible, allowing for the inclusion 
 * of various types of data streams (e.g., video, audio, metadata) while maintaining 
 * a consistent structure. The use of TLV encoding facilitates the addition of new 
 * data types and structures without breaking compatibility.
 * 
 * The design coincides with how data is transported within the framework itself 
 * (see @ref api-core-io-packet).
 *
 * @{
 * @namespace vidre::storage Storage related.
 * @defgroup api-core-storage-raw Raw data file
 * @defgroup api-core-storage-details Implementation details
 * @}
 * */

#include <vidre/core/config.hh>

#endif  // VIDRE_CORE_STORAGE_HH
