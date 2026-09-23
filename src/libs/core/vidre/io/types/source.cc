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
#include "sink.hh"
#include "../../log.hh"

using namespace vidre;
using namespace vidre::io;

source::~source() {
    unlink();
}

status io::source::unlink() {
    
    return detach(sink_.get());
}

status source::attach(shared<io::sink> other, cap::flag_type cap) {
    if (sink_.get())  // already connected
        return status::bad;

    // store fields, clear stats
    cap_[index::source] = cap;
    clearstats();

    // attach sink
    sink_ = other;

    // start handler
    source_task_ = task::make(this, &source::source_handler, std::chrono::milliseconds(1));

    return status::ok;
}

status source::detach(const io::sink *snk) {
    if (sink_.get() != snk)
        return status::ok;

    if (source_task_->isrunning())
        return status::bad;

    sink_ = {};

    // FIXME stop stats?
    return status::ok;
}

void source::source_handler(shared<task> self, std::chrono::milliseconds pause) {
    task::scope scope(self);
    log::debug() << "source_handler: started...\n";

    // wait for connection
    while (!sink_ or not sink_->isattached()) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    // prepare
    log::debug() << "source_handler: preparing...\n";
    self->on_terminate([ev = &source_event_]() {
        ev->cancel();
    });

    enter_handler();

    // event loop
    log::debug() << "source_handler: running...\n";
    while (not scope.isterminated()) {
        source_step();
    }

    leave_handler();
    log::debug() << "source_handler: finished.\n";
}
