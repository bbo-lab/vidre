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
#include "hub.hh"
#include "../log.hh"
#include "../util/develop.hh"

using namespace vidre;

std::filesystem::path vidre::plugin::hub::plugindir = VIDRE_INSTALL_PLUGINDIR;
std::filesystem::path vidre::plugin::hub::scriptdir = VIDRE_INSTALL_SCRIPTDIR;

json plugin::hub::find() noexcept {
    log::trace tr;

    if (develop::inbuild()) {
        plugindir = PROJECT_BINARY_DIR "/plugins";
        scriptdir = PROJECT_BINARY_DIR "/plugins";
    }

    // search plugins
    tr << "Searching for plugins in " << (std::string) plugindir << log::endl;
    // FIXME

    return {};
}