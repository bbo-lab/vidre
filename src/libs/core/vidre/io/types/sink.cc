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
#include "sink.hh"
#include "source.hh"
#include "../../log.hh"

using namespace vidre;
using namespace vidre::io;

sink::~sink() {
    // FIXME
    // terminate();
    // if (source_)
    //     source_->disconnect(this);
}

status io::sink::unlink() {
    // FIXME stop handler
    return detach(source_.get());
}

status sink::attach(shared<io::source> src, cap::flag_type cap) {
    if (source_)  // already connected
        return status::bad;

    // store fields, clear stats
    cap_[index::sink] = cap;
    clearstats();

    // attach source
    source_ = src;

    // start event handler if needed
    if (cap & cap::event) {
        // start handler
        sink_task_ = task::make(this, &sink::sink_handler, std::chrono::milliseconds(1));
        sink_task_->on_terminate([src = source_]() {
            src->source_event_.cancel();
        });
    }

    return status::ok;
}

status sink::detach(const io::source *src) {
    if (source_.get() != src)
        return status::bad;

    if (sink_task_->isrunning())
        return status::bad;

    // source must be disconnected first
    if (source_->detach(this) == status::bad)
        return status::bad;

    source_->detach(this);

    source_ = {};
    return status::ok;
}

void sink::sink_handler(shared<task> self, std::chrono::milliseconds pause) {
    task::scope scope(self);
    log::debug() << "sink_handler: started...\n";

    // wait for connection
    while (!source_ or not source_->isattached()) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    log::debug() << "sink_handler: preparing...\n";
    enter_handler();

    // event loop
    log::debug() << "sink_handler: running...\n";
    while (not scope.isterminated()) {
        auto retval = source_->wait();
        if (retval == status::bad)
            continue;

        // FIXME this logic needs to use source_->queue_depth()
        for (size_type i = 0; i < *retval; ++i)
            push(source_->pull());
    }

    leave_handler();
    log::debug() << "sink_handler: finished.\n";
}
