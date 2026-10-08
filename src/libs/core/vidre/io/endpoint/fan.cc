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
#include "vidre/io/endpoint/source.hh"
#include "vidre/log.hh"

#include <memory>
#include <mutex>

using namespace vidre;

// === fan::out ===============================================================
#pragma region fan::out

io::fan::out::~out() {
    unlink();
}

status io::fan::out::put(shared<packet> p) {
    for (auto &branch : branches_out_.snapshot())
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
        for (const auto &branch : branches_out_.snapshot()) {
            if (not branch->isready(endpoint::dir::sink))
                return false;
        }
    }

    return true;
}

status io::fan::out::unlink() {
    // check if connected at all
    if (not sink_predecessor_ and branches_out_.empty())
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

    // remove branches from the back while unlinking them
    while (!branches_out_.empty()) {
        auto branch = branches_out_.back();

        if (branch->unlink() == status::bad) {
            retval = status::bad;
            log::error() << "failed to unlink branch from fan-out\n";
            // need to decide how to handle failure
        }
    }

    // ensure all branches are removed from the back
    if (!branches_out_.empty())
        log::error() << "not all branches were removed from the back\n";
    branches_out_.clear();

    return retval;
}

status io::fan::out::attach(shared<endpoint::sink> src, cap::flag_type cap) {
    throw exception::internal() << "a fan::out must not be attached directly";
    return status::bad;
}

shared<io::endpoint::source> io::fan::out::branch_source() {
    shared<out::branch> branch = endpoint::make<out::branch>(*this);
    branches_out_.append(branch);
    return branch;
}

status io::fan::out::detach(const endpoint::sink *ptr) {
    // no lock here!

    // find the branch corresponding to the given sink pointer, and remove it from the list if found
    for (auto &branch : branches_out_.snapshot()) {
        if (branch->source_follower_.get() == ptr) {
            branches_out_.remove(branch.get());
            return status::ok;
        }
    }

    return status::bad;
}
#pragma endregion

// === fan::out::branch =======================================================
#pragma region fan::out::branch
status io::fan::out::branch::put(shared<packet> pkt) {
    queue_.push(std::move(pkt));
    notify();

    return status::ok;
}

shared<io::packet> io::fan::out::branch::get() {
    return queue_.pop();
}

bool io::fan::out::branch::isready(endpoint::dir direction) const noexcept {
    if (!!(direction & endpoint::dir::sink)) {
        if (!source_follower_)
            return false;
    }

    if (!!(direction & endpoint::dir::source))
        return parent_->isready(endpoint::dir::source);

    return true;
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
    auto n = branches_in_.size();
    if (n == 0)
        return {};
    if (n >= round_robin_)
        round_robin_ = 0;

    auto branch = branches_in_.at(round_robin_);
    if (!branch)
        throw exception::internal() << "branch is null";

    auto pkt = branch->pop();
    if (pkt)
        round_robin_++;
    return pkt;
}

status io::fan::in::unlink() {
    if (!source_follower_ && branches_in_.empty())
        return status::ok;

    status retval = status::ok;
    if (source_follower_) {
        if (source_follower_->unlink() == status::bad) {
            // TODO we need a better thought-through strategy for unlink failures
            log::error() << "failed to unlink source from fan-out\n";
            retval = status::bad;
        }

        source_follower_.reset();
    }

    // remove branches from the back while unlinking them
    while (!branches_in_.empty()) {
        auto branch = branches_in_.back();

        if (branch->unlink() == status::bad) {
            retval = status::bad;
            log::error() << "failed to unlink branch from fan-in\n";
            // need to decide how to handle failure
        }
    }

    // ensure all branches are removed from the back
    if (!branches_in_.empty())
        log::error() << "not all branches were removed from the back\n";
    branches_in_.clear();

    return retval;
}

bool io::fan::in::isready(endpoint::dir direction) const noexcept {
    if (!source_follower_ or branches_in_.empty())
        return false;

    if (!!(direction & endpoint::dir::sink)) {
        if (not source_follower_->isready(endpoint::dir::sink))
            return false;
    }

    if (!!(direction & endpoint::dir::source)) {
        for (const auto &branch : branches_in_.snapshot()) {
            if (not branch->isready(endpoint::dir::source))
                return false;
        }
    }

    return true;
}

status io::fan::in::attach(shared<endpoint::source> src, cap::flag_type cap) {
    throw exception::internal() << "a fan::in must not be attached directly";
    return status::bad;
}

status io::fan::in::detach(const endpoint::source *ptr) {
    // locked in unlink()!
    for (auto &branch : branches_in_.snapshot()) {
        if (branch->sink_predecessor_.get() == ptr) {
            branches_in_.remove(branch.get());
            return status::ok;
        }
    }

    return status::bad;
}

shared<io::endpoint::sink> io::fan::in::branch_sink() {
    shared<in::branch> branch = endpoint::make<in::branch>(*this);
    branches_in_.append(branch);
    return branch;
}

#pragma endregion

// === fan::in::branch ========================================================
#pragma region fan::in::branch

status io::fan::in::branch::put(shared<io::packet> p) {
    queue_.push(p);
    parent_->notify();

    return status::ok;
}

bool io::fan::in::branch::isready(io::endpoint::dir direction) const noexcept {
    if (!!(direction & endpoint::dir::source)) {
        if (!sink_predecessor_)
            return false;
    }

    if (!!(direction & endpoint::dir::sink))
        return parent_->isready(endpoint::dir::sink);

    return true;
}

status io::fan::in::branch::unlink() {
    if (!sink_predecessor_)  // not connected
        return status::ok;

    log::trace tr;
    tr << "unlinking branch from fan-out...\n";

    auto retval = endpoint::detach(sink_predecessor_, this);
    if (retval == status::bad) {
        tr << "failed to detach sink\n";
        return status::bad;
    }

    retval = detach(sink_predecessor_.get());
    tr << retval << ".\n";

    return retval;
}

status io::fan::in::branch::attach(shared<io::endpoint::source> src, cap::flag_type cap) {
    if (sink_predecessor_)  // already connected
        return status::bad;

    // store fields, clear stats
    cap_.source = cap;
    clearstats();

    // attach sink
    sink_predecessor_ = src;

    // start handler
    if (cap & endpoint::cap::push) {
        sink_task_ = task::make(this, &branch::sink_handler, std::chrono::milliseconds(1));
    }

    return status::ok;
}

status io::fan::in::branch::detach(const io::endpoint::source *ptr) {
    return parent_->detach(ptr);
}

#pragma endregion