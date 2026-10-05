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
#include "source.hh"
#include "vidre/log.hh"
#include "vidre/util/debug.hh"

using namespace vidre;
using namespace vidre::io;

source::~source() {
    unlink();
}

status io::source::unlink() {
    if (!source_follower_)  // not connected
        return status::ok;

    log::trace tr;
    tr << "unlinking source...\n";

    auto retval = endpoint::detach(source_follower_, this);
    if (retval == status::bad) {
        tr << "failed to detach sink\n";
        return status::bad;
    }

    retval = detach(source_follower_.get());
    tr << retval << ".\n";

    return retval;
}

status source::attach(shared<endpoint::sink> other, cap::flag_type cap) {
    if (source_follower_)  // already connected
        return status::bad;

    log::debug() << "attaching source to sink with cap: " << std::format("{}", cap) << "\n";

    // store fields, clear stats
    cap_.source = cap;
    clearstats();

    // attach sink
    source_follower_ = other;

    // start handler
    source_event_.reset();
    source_task_ = task::make(this, &source::source_handler, std::chrono::milliseconds(1));

    return status::ok;
}

status source::detach(const endpoint::sink *ptr) {
    if (!source_follower_)
        return status::ok;

    if (source_follower_.get() != ptr)
        // shouldn't happen
        return status::bad;

    log::trace tr;
    tr << debug::tag(this) << " detaching from sink...\n";

    // stop task
    if (source_task_) {
        if (source_task_->isrunning()) {
            source_task_->terminate();
            source_task_->join();
        }

        source_task_ = {};
    }

    // clear link
    source_follower_.reset();

    tr << "done.\n";
    return status::ok;
}

void source::source_handler(shared<task> self, std::chrono::milliseconds pause) {
    task::scope scope(self);
    log::debug() << "source_handler: started...\n";

    // wait for connection
    while (!source_follower_ or not source_follower_->isready()) {
        if (scope.isterminated())
            return;

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    // prepare
    log::debug() << "source_handler: preparing...\n";
    self->on_terminate([ev = &source_event_]() {
        ev->cancel();
    });

    // event loop
    log::debug() << "source_handler: running...\n";
    while (not scope.isterminated()) {
        if (not source_step())
            break;
    }

    log::debug() << "source_handler: finished.\n";
}
