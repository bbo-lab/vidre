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
#include "vidre/log.hh"

#include <memory>

using namespace vidre;

// === fan::out ===============================================================
#pragma region fan::out

io::fan::out::~out() {
    unlink();
}

status io::fan::out::put(shared<packet> p) {
    for (auto &branch : branches_out_)
        branch->put(p);

    return status::ok;
}

bool io::fan::out::isready(endpoint::dir direction) const noexcept {
    if (!sink_predecessor_ or branches_out_.empty())
        return false;

    if (!!(direction & endpoint::dir::source)) {
        if (not sink_predecessor_->isready(endpoint::dir::source))
            return false;
    }

    if (!!(direction & endpoint::dir::sink)) {
        for (const auto &branch : branches_out_) {
            if (not branch->isready(endpoint::dir::sink))
                return false;
        }
    }

    return true;
}

status io::fan::out::unlink() {
    if (not sink_predecessor_ and branches_out_.empty())  // not connected
        return status::ok;

    status retval = status::ok;
    if (sink_predecessor_) {
        if (sink_predecessor_->unlink() == status::bad) {
            // TODO we need a better thought-through strategy for unlink failures
            log::error() << "failed to unlink source from fan-out\n";
            retval = status::bad;
        }

        sink_predecessor_.reset();
    }

    for (auto &branch : branches_out_) {
        if (branch->unlink() == status::bad) {
            retval = status::bad;
            log::error() << "failed to unlink branch from fan-out\n";
        }
    }
    branches_out_.clear();

    return retval;
}

status io::fan::out::attach(shared<endpoint::sink> src, cap::flag_type cap) {
    throw exception::internal() << "a fan::out must not be attached directly";
    return status::bad;
}

shared<io::endpoint::source> io::fan::out::branch_source() {
    shared<out::branch> branch = endpoint::make<out::branch>(*this);
    branches_out_.push_back(branch);
    return branch;
}

status io::fan::out::detach(const endpoint::sink *ptr) {
    for (auto &branch : branches_out_) {
        if (branch->source_follower_.get() == ptr) {
            branches_out_.erase(std::remove(branches_out_.begin(), branches_out_.end(), branch), branches_out_.end());
            return status::ok;
        }
    }

    return status::bad;
}
#pragma endregion

// === fan::out::branch =======================================================
#pragma region fan::out::branch
status io::fan::out::branch::put(shared<packet> pkt) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_.emplace(std::move(pkt));
    }

    // FIXME queue_depth logic

    notify();

    return status::ok;
}

shared<io::packet> io::fan::out::branch::get() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (queue_.empty())
        return {};

    auto pkt = std::move(queue_.front());
    queue_.pop();

    return pkt;
}

status io::fan::out::branch::unlink() {
    if (!source_follower_)  // not connected
        return status::ok;

    log::trace tr;
    tr << "unlinking branch from fan-out...\n";

    auto retval = endpoint::detach(source_follower_, this);
    if (retval == status::bad) {
        tr << "failed to detach sink\n";
        return status::bad;
    }

    retval = detach(source_follower_.get());
    tr << retval << ".\n";

    return retval;
}

status io::fan::out::branch::attach(shared<endpoint::sink> other, cap::flag_type cap) {
    if (source_follower_)  // already connected
        return status::bad;

    // store fields, clear stats
    cap_.source = cap;
    clearstats();

    // attach sink
    source_follower_ = other;

    // start handler
    source_event_.reset();
    if (cap & endpoint::cap::push) {
        source_task_ = task::make(this, &branch::source_handler, std::chrono::milliseconds(1));
    }

    return status::ok;
}

status io::fan::out::branch::detach(const endpoint::sink *ptr) {
    return parent_->detach(ptr);
}

bool io::fan::out::branch::source_step() {
    auto retval = wait();
    if (retval != status::ok) {
        if (retval.error() == std::errc::operation_canceled)
            return false;
        log::debug() << "!!! " << std::make_error_condition(retval.error()).message() << "\n";
    }
    else {
        // FIXME need to use queue_depth logic

        while (not queue_.empty())  // FIXME this is read without locking
            source_follower_->put(get());
    }

    return true;
}
#pragma endregion

// === fan::in ================================================================
#pragma region fan::in

io::fan::in::~in() {
    unlink();
}

shared<io::packet> io::fan::in::get() {
    // Implement the logic to retrieve a packet from the fan-in endpoint.
    // This is a placeholder implementation and should be replaced with actual logic.
    return {};
}

status io::fan::in::unlink() {
    // Implement the logic to unlink the fan-in endpoint from all connected branches and sources.
    // This is a placeholder implementation and should be replaced with actual logic.
    return status::ok;
}

bool io::fan::in::isready(endpoint::dir direction) const noexcept {
    // Implement the logic to check if the fan-in endpoint is ready.
    // This is a placeholder implementation and should be replaced with actual logic.
    return false;
}

status io::fan::in::attach(shared<endpoint::source> src, cap::flag_type cap) {
    // Implement the logic to attach a source to the fan-in endpoint.
    // This is a placeholder implementation and should be replaced with actual logic.
    return status::ok;
}

status io::fan::in::detach(const endpoint::source *ptr) {
    // Implement the logic to detach a source from the fan-in endpoint.
    // This is a placeholder implementation and should be replaced with actual logic.
    return status::ok;
}

shared<io::endpoint::sink> io::fan::in::branch_sink() {
    // Implement the logic to retrieve the branch sink associated with the fan-in endpoint.
    // This is a placeholder implementation and should be replaced with actual logic.
    return {};
}

#pragma endregion

// === fan::in::branch ========================================================
#pragma region fan::in::branch

#pragma endregion