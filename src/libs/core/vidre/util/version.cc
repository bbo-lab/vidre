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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "version.hh"

#include <cctype>

vidre::version vidre::version::vidre{VIDRE_VERSION_MAJOR, VIDRE_VERSION_MINOR, VIDRE_VERSION_PATCH};

vidre::version::version(std::string_view str) {
    auto it = str.begin();
    if (it == str.end())
        return;

    // major
    major_ = parse(it, str.end());
    if (it == str.begin())
        goto parsing_error;
    if (it == str.end())
        return;

    // minor
    if (*it != '.')
        goto parsing_error;
    else
        ++it;
    minor_ = parse(it, str.end());
    if (it == str.end())
        return;

    // patch
    if (it != str.end() && *it == '.')
        ++it;
    patch_ = parse(it, str.end());

    if ((it == str.end()) or std::isspace(*it) or std::ispunct(*it))
        return;

parsing_error:
    throw exception::runtime() << error::message("invalid version string: " + std::string(str));
}
