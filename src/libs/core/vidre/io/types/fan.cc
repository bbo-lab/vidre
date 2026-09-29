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
#include "fan.hh"
#include <memory>

using namespace vidre;

// === fan::out ===============================================================

io::fan::out::~out() {
    unlink();
}

io::result io::fan::out::push(shared<packet> p) {
    // FIXME this raises the question how a return value for push is sensible...

    for (auto &branch : branches_right_)
        branch->queue(p);

    // FIXME
    return {};
}

bool io::fan::out::isready(endpoint::dir direction) const noexcept {
    if (!sink_left_ or branches_right_.empty())
        return false;

    if (!!(direction & endpoint::dir::source)) {
        if (not sink_left_->isready(endpoint::dir::source))
            return false;
    }

    if (!!(direction & endpoint::dir::sink)) {
        for (const auto &branch : branches_right_) {
            if (not branch->isready(endpoint::dir::sink))
                return false;
        }
    }

    return true;
}

status io::fan::out::unlink() {
    if (not sink_left_ and branches_right_.empty())  // not connected
        return status::ok;

    if (sink_left_) {
        if (sink_left_->unlink() == status::bad)
            return status::bad;

        sink_left_.reset();
    }

    for (auto &branch : branches_right_) {
        if (branch->unlink() == status::bad)
            // FIXME maybe this should be an exception, if we leave here, the whole graph will be left dangling
            return status::bad;
    }
    branches_right_.clear();

    return status::ok;
}

status io::fan::out::attach(shared<endpoint::sink> src, cap::flag_type cap) {
    shared<out::branch> branch = endpoint::make<out::branch>(*this);

    auto retval = branch->attach(std::move(src), cap);
    if (retval == status::ok)
        branches_right_.emplace_back(std::move(branch));

    return retval;
}

status io::fan::out::detach(const endpoint::sink *ptr) {
    for (auto &branch : branches_right_) {
        if (branch->source_right_.get() == ptr) {
            auto retval = branch->detach(ptr);
            branches_right_.erase(std::remove(branches_right_.begin(), branches_right_.end(), branch), branches_right_.end());
            return retval;
        }
    }

    return status::bad;
}

// === fan::out::branch =======================================================

status io::fan::out::branch::queue(shared<packet> pkt) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.emplace(std::move(pkt));
    
    // FIXME queue_depth logic

    notify();
    
    return status::ok;
}

shared<io::packet> io::fan::out::branch::pull() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_.empty())
        return {};

    auto pkt = std::move(queue_.front());
    queue_.pop();
    
    return pkt;
}

status io::fan::out::branch::unlink() {
    // FIXME
    return status::bad;
}
