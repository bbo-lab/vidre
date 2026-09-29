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

io::channel::~channel() {
    unlink();
}

io::result io::channel::push(shared<packet> p) {
    io::result retval = invalid<offset_type>;
    if (source_right_) {
        retval = source_right_->push(p);
        stats_.update(p, retval);
    }
    return retval;
}

status io::channel::unlink() {
    if (not sink_left_ and not source_right_)  // not connected
        return status::ok;

    shared<endpoint::source> src = sink_left_;
    shared<endpoint::sink> snk = source_right_;

    // unlink left edge (our sink from its source)
    if (sink_left_) {
        if (sink::unlink() == status::bad)
            return status::bad;
    }

    // unlink right edge (our source from its sink)
    if (source_right_) {
        if (source::unlink() == status::bad)
            return status::bad;
    }

    // re-link left and right edges
    if (src && snk)
        return endpoint::connect(std::move(src), std::move(snk));

    return status::ok;
}

// status io::channel::attach(shared<io::source> src, cap::flag_type cap) {
//     throw exception::internal() << "method not implemented or capability erroneously advertised";
// }

// status io::channel::attach(shared<io::sink> src, cap::flag_type cap) {
//     throw exception::internal() << "method not implemented or capability erroneously advertised";
// }

// status io::channel::detach(const io::source *ptr) {
//     throw exception::internal() << "method not implemented or capability erroneously advertised";
// }

// status io::channel::detach(const io::sink *ptr) {
//     throw exception::internal() << "method not implemented or capability erroneously advertised";
// }
