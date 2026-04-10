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
#ifndef VIDRE_CORE_STORAGE_FILE_HH
#define VIDRE_CORE_STORAGE_FILE_HH
#pragma once

/**@defgroup api-core-storage-file File handling.
 * @ingroup api-core-storage
 * @fileinfo
 * @details
 * 
 * Apart for data sinks and sources, this module provides basic file handling 
 * functionality mostly pulled in from standard libraries (see 
 * [std::filesystem](https://en.cppreference.com/w/cpp/filesystem.html)) 
 * for convenience. 
 * */

#include <vidre/core/config.hh>
#include <vidre/storage/io.hh>

#include <filesystem>

namespace vidre {

    namespace storage {

        /** File related functionality.
         * @ingroup api-core-storage-file
         * */
        namespace file {

            /**@addtogroup api-core-storage-file
             * */
            ///@{

            using path = std::filesystem::path;

            /** Remove file.
             * */
            inline bool remove(const path &filename) noexcept {
                return std::filesystem::remove(filename);
            }

            // FIXME move fileio/filesink here?

            ///@}

        }  // namespace file

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_IO_HH
