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
#include "basic.hh"

using namespace vidre;
using namespace vidre::io;

bool vidre::io::endpoint::match(endpoint::cap::flag_type &from, endpoint::cap::flag_type &to) noexcept {
    auto common = from & to;

    if ((common & endpoint::cap::any) == endpoint::cap::none)
        return false;

    // select stream/packet mode
    if (!!((common & endpoint::cap::any) & endpoint::cap::packet))
        common = endpoint::cap::packet;  // packet mode
    else
        common = endpoint::cap::chunk;  // stream mode

    from &= ~endpoint::cap::any;
    from |= common;
    to &= ~endpoint::cap::any;
    to |= common;

    // push/pull mode
    if (!!(from & endpoint::cap::push))
        to &= ~endpoint::cap::pull;
    else if (!!(to & endpoint::cap::pull))
        from &= ~endpoint::cap::push;

    return true;
}
