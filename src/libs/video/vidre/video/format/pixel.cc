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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "pixel.hh"

#include <vidre/script.hh>
#include <vidre/log.hh>

using namespace vidre;
using namespace vidre::video;

const char *format::pixel::name() const noexcept {
    script::engine::stack top(script::g);

    // if (not script_get_table(code_)) {
    //     log::warning() << "unknown pixel format 0x" << std::format("{:08X}", code_) << log::endl;
    //     return {};
    // }

    // // the info table _always_ contains the "name" field:
    // script::g.pushlstring("name", 4);
    // script::g.gettable(-2);

    script::g.pushlstring("FIXME", 5);

    return script::g.tostring(-1);
}

settings::json format::pixel::json() const noexcept {
    return {{"name", name()}};
}
