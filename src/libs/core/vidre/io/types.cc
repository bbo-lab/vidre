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
#include "types.hh"
#include "packet.hh"

using namespace vidre;

std::shared_ptr<std::thread> io::connect(source::shared &src, sink::shared &snk) noexcept {
    // FIXME all should be done in a separate thread
    if (!(*src & mode::packet)) {
        uint8_t buf[4096];
        while (src->valid() && snk->valid()) {
            auto res = src->get({4096, buf});
            if (!res)
                break;
            if (res.count == 0)
                break;

            auto putres = snk->put({res.count, buf});
            if (!putres)
                break;
        }

        return nullptr;
    }
    else {
        // FIXME
        return nullptr;
    }
}
