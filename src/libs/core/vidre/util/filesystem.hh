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
#ifndef VIDRE_CORE_UTIL_FILESYSTEM_HH
#define VIDRE_CORE_UTIL_FILESYSTEM_HH
#pragma once

/**@defgroup api-core-util-filesystem File-system tools.
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>

#include <filesystem>
#include <regex>
#include <vector>
#include <string>

namespace vidre {

    /**@addtogroup api-core-util-filesystem
     * */
    ///@{

    namespace fs = std::filesystem;

    /** Find files recursively using regex-pattern.
     * @param root The start directory.
     * @param patter Regex pattern (default: ECMAscript, eg. @c R"(^file[.]cpp$)" ).
     * Example:
     * @code
     * auto list = find_files("/tmp", R"(.*[.]cfg$)");
     * @endcode
     * */
    std::vector<fs::path> find_files(const fs::path &root, const std::regex &pattern);
    inline std::vector<fs::path> find_files(const fs::path &root, const char *pattern) {
        return find_files(root, std::regex(pattern));
    }

    // FIXME
    // template<std::size_t N>
    // inline std::vector<fs::path> find_files(const fs::path &root, char const pattern[N]) {
    //     return find_files(root, std::regex(pattern, N));
    // }

    /** Load a file.
     * This helper loads a complete file into a string avoiding the costly standard streams.
     * */
    std::string loadfile(const fs::path &filename);

    /** Save a file.
     * This helper saves a string buffer to a file avoiding the costly standard streams.
     * */
    void savefile(const fs::path &filename, const std::string &buf);

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_UTIL_FILESYSTEM_HH
