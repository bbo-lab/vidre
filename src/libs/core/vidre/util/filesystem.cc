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
#include "filesystem.hh"
#include "../exception.hh"

using namespace vidre;

std::vector<fs::path> vidre::find_files(const fs::path &root, const std::regex &pattern) {
    std::vector<fs::path> matches;

    if (!fs::exists(root)) {
        return matches;
    }

    for (const auto &entry : fs::recursive_directory_iterator(root)) {
        if (fs::is_regular_file(entry.status())) {
            std::string filename = entry.path().filename().string();
            if (std::regex_search(filename, pattern)) {
                matches.push_back(entry.path());
            }
        }
    }

    return matches;
}

std::string vidre::loadfile(const fs::path &filename) {
    std::string retval;

    FILE *f = fopen(filename.c_str(), "r");
    if (not f)
        throw vidre::system_error({errno, std::generic_category()});  // FIXME add filename

    char buf[1024];
    while (not feof(f)) {
        const char *line = fgets(buf, sizeof(buf), f);
        retval += line;
    }
    fclose(f);

    return retval;
}

void vidre::savefile(const fs::path &filename, const std::string &buf) {
    //
}
