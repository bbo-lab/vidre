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
#include "channel.hh"

using namespace vidre;

io::result io::channel::put(shared<packet> p) {
    io::result retval = invalid<offset_type>;
    if (sink_) {
        retval = sink_->put(p);
        stats_.update(p, retval);
    }
    return retval;
}

void io::channel::start() {
    throw exception::internal() << "method not implemented or capability erroneously advertised";
}

void io::channel::stop() {
    throw exception::internal() << "method not implemented or capability erroneously advertised";
}

status io::channel::unlink() {
    throw exception::internal() << "method not implemented or capability erroneously advertised";
}

