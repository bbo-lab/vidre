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

using namespace vidre;
using namespace vidre::io;

sink::~sink() {
    // FIXME
    // terminate();
    // if (source_)
    //     source_->disconnect(this);
}

void io::sink::start() {
    throw exception::internal() << "method not implemented or capability erroneously advertised";
}

void io::sink::stop() {
    throw exception::internal() << "method not implemented or capability erroneously advertised";
}

status io::sink::unlink() {
    throw exception::internal() << "method not implemented or capability erroneously advertised";
}

void sink::sink_handler(shared<task> self, std::chrono::milliseconds pause) {
    task::scope scope(self);

    // wait, until active (ie. started).
    while (not isactive() and not scope.isterminated()) {
        std::this_thread::sleep_for(pause);
    }

    // process loop
    while (not scope.isterminated()) {
        auto retval = source_->wait();
        if (retval == status::bad)
            continue;

        // FIXME this logic needs to use source_->queue_depth()
        for (size_type i = 0; i < *retval; ++i)
            put(source_->get());
    }
}

status sink::connect(shared<io::source> other, cap::flag_type cap) {
    if (source_.get())  // already connected
        return status::bad;

    // store fields, clear stats
    cap_[index::sink] = cap;
    source_ = other;
    clearstats();

    // push mechanism -> all done
    if (!(cap & cap::event))
        return status::ok;

    // event mechanism -> start sink task
    sink_task_ = task::make([this](shared<task> self) {
        sink_handler(self);
    });

    return status::ok;
}

status sink::disconnect(const io::source *src) {
    if (source_.get() != src)
        return status::bad;

    // FIXME
    // terminate();
    source_->disconnect(this);

    source_ = {};
    return status::ok;
}