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

using namespace vidre;
using namespace vidre::io;

source::~source() {
    terminate();
    if (sink_)
        sink_->disconnect(this);
}

void source::terminate() {
    if (source_task_ && source_task_->isrunning())
        source_task_->terminate();
}

void source::source_handler(shared<task> self, std::chrono::milliseconds pause) {
    task::scope scope(self);

    // prepare
    self->on_terminate([ev = &source_event_]() {
        ev->cancel();
    });

    // wait, until active (ie. started).
    while (not isactive() and not scope.isterminated()) {
        std::this_thread::sleep_for(pause);
    }

    // process loop
    while (not scope.isterminated()) {
        source_step();
    }
}

status source::connect(shared<io::sink> other, cap::flag_type cap) {
    if (sink_.get())  // already connected
        return status::bad;

    // store fields, clear stats
    cap_[index::source] = cap;
    sink_ = other;
    clearstats();

    // start source task
    source_task_ = task::make([this](shared<task> self) {
        source_handler(self);
    });

    return status::ok;
}

status source::disconnect(const io::sink *snk) {
    if (sink_.get() != snk)
        return status::bad;

    terminate();

    sink_ = {};
    return status::ok;
}