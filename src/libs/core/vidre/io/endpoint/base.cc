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
#include "base.hh"
#include "vidre/log.hh"

using namespace vidre;
using namespace vidre::io;

void endpoint::stats::update(shared<io::packet> pkt, const io::result &result) noexcept {
    if (pkt) {
        if (result) {
            // FIXME
            // ++packets.transmitted;
            bytes.transmitted += result.count;
        }
        else {
            // FIXME
            // ++packets.dropped;
            ++errors;
        }
    }
}

void endpoint::stats::update(const io::result &result) noexcept {
    if (!result)
        ++errors;
    else
        bytes.transmitted += result.count;
}

settings::json endpoint::cap::setup(flag_type flgs) const {
    return {
        //
        {"packet", !!(flgs & endpoint::cap::packet)},      //
        {"chunk", !!(flgs & endpoint::cap::chunk)},        //
                                                           //
        {"push", !!(flgs & endpoint::cap::push)},          //
        {"nonblock", !!(flgs & endpoint::cap::nonblock)},  //
        {"event", !!(flgs & endpoint::cap::event)},        //
                                                           //
        {"multi", !!(flgs & endpoint::cap::multi)}         //
    };
}

settings::json endpoint::cap::setup() const {
    settings::json retval;

    if (sink != endpoint::cap::none)
        retval["sink"] = setup(sink);
    if (source != endpoint::cap::none)
        retval["source"] = setup(source);

    return retval;
}

status endpoint::connect(shared<endpoint::source> src, shared<endpoint::sink> snk, const settings::json &config) {
    if (not src.get())
        throw exception::internal() << "endpoint passed as first argument is null";
    if (not snk.get())
        throw exception::internal() << "endpoint passed as second argument is null";

    cap::flag_type src_cap = src->getcap().source;
    log::debug() << "connecting '" << src->tag() << "' to '" << snk->tag() << "'\n";

    log::debug() << "    src_cap: " << std::format("{}", src_cap) << log::endl;

    cap::flag_type snk_cap = snk->getcap().sink;
    log::debug() << "    snk_cap: " << std::format("{}", snk_cap) << log::endl;

    // sanity checks
    if (!src_cap)
        throw exception::internal() << "endpoint passed as first argument has no capabilities";
    if (!snk_cap)
        throw exception::internal() << "endpoint passed as second argument has no capabilities";

    // we only deal with packet-based endpoints
    if (!(src_cap & endpoint::cap::packet) || !(snk_cap & endpoint::cap::packet))
        throw exception::internal() << "endpoint capabilities are not packet-based";

    src_cap &= ~cap::chunk; // FIXME sources, such as log::endpoint, may support packet to chunk conversion (with style)
    snk_cap &= ~cap::chunk;

    // select mode (preferred mode is, that the source pushes if it can)
    if ((src_cap & cap::push) && (snk_cap & cap::push)) {
        src_cap &= ~cap::event;
        snk_cap &= ~cap::event;
    }
    else if ((src_cap & cap::event) && (snk_cap & cap::event)) {
        src_cap &= ~cap::push;
        snk_cap &= ~cap::push;
    }
    else {
        // FIXME if src does push and snk only event, we can install an adapter (based on channel) by default
        // FIXME that's actually rather an implementation error, isn't it?
        log::error() << "incompatible capabilities between source '" << src->tag() << "' and sink '" << snk->tag() << "'\n";
        return status::bad;
    }

    // handle "multi" capability for sink and source
    if (snk_cap & cap::multi)
        snk = snk->branch_sink();
    snk->setup(config);

    if (src_cap & cap::multi)
        src = src->branch_source();
    src->setup(config);

    // connect sink
    if (snk->attach(src, snk_cap) == status::bad) {
        src->detach(snk.get());
        return status::bad;
    }

    // connect source
    if (src->attach(snk, src_cap) == status::bad)
        return status::bad;

    return status::ok;
}
