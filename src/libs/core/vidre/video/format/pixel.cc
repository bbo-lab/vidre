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
#include "pixel.hh"

#include "../../script.hh"
#include "../../log.hh"

using namespace vidre;

bool script_get_table(video::format::fourcc code) {
    // look-up logic is stored in this module:
    script::g.require("vidre.video.format.fourcc");
    if (script::g.type(-1) != lua::type::table) 
        log::fatal() << "vidre.video.format.fourcc not accessible";

    // get entry for code:
    script::g.pushinteger(static_cast<lua::integer>(code));
    script::g.gettable(-2);

    // success if we got a table back:
    return script::g.type(-1) == lua::type::table;
}

std::string video::format::pixel::name() const noexcept {
    script::engine::stack top(script::g);

    if (not script_get_table(format_)) {
        log::warning() << "unknown pixel format 0x" << std::format("{:08X}", static_cast<uint32_t>(format_)) << log::endl;
        return {};
    }
    
    // the info table _always_ contains the "name" field:
    script::g.pushlstring("name", 4);
    script::g.gettable(-2);

    return script::g.tostring(-1);
}
