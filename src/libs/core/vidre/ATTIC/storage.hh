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
#ifndef VIDRE_CORE_STORAGE_HH
#define VIDRE_CORE_STORAGE_HH
#pragma once

/**@addtogroup api-core-storage
 * @details
 * @fileinfo
 *
 * The core library provides a number of fundamental mechanisms for storing data
 * generically. File formats are, however, only supported through plugins.
 *
 * @{
 * @namespace vidre::storage Storage related.
 * @defgroup api-core-storage-io In-/output
 * @defgroup api-core-storage-file File handling
 * @defgroup api-core-storage-mapped Memory-mapped file handling
 * @defgroup api-core-storage-bit Bit packing and more
 * @defgroup api-core-storage-raw Raw data file
 * @defgroup api-core-storage-details Implementation details
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/storage/io.hh>
#include <vidre/storage/file.hh>
#include <vidre/storage/bit.hh>
#include <vidre/storage/raw.hh>

#endif  // VIDRE_CORE_STORAGE_HH
