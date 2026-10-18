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
#include "vidre/log.hh"

using namespace vidre;

io::channel::~channel() {
    unlink();
}

status io::channel::put(shared<packet> pkt) {
    if (!pkt)
        throw exception::logic() << "invalid null packet encountered";

    if (source_follower_) {
        // FIXME stats_.update(p);
        return source_follower_->put(pkt);
    }

    return status::bad;
}

status io::channel::prepare() {
    if (!islinked())
        return status::bad;

    if (sink::prepare() == status::bad)
        return status::bad;

    if (source::prepare() == status::bad)
        return status::bad;

    return status::ok;
}

status io::channel::unlink() {
    if (not sink_predecessor_ and not source_follower_)  // not connected
        return status::ok;

    shared<endpoint::source> src = sink_predecessor_;
    shared<endpoint::sink> snk = source_follower_;

    // unlink left edge (our sink from its source)
    if (sink_predecessor_) {
        if (sink::unlink() == status::bad)
            return status::bad;
    }

    // unlink right edge (our source from its sink)
    if (source_follower_) {
        if (source::unlink() == status::bad)
            return status::bad;
    }

    // re-link left and right edges
    if (src && snk)
        return endpoint::connect(std::move(src), std::move(snk));

    return status::ok;
}

bool io::channel::source_step() {
    std::this_thread::yield();
    return true;
}
