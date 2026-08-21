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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "basic.hh"

using namespace vidre;
using namespace vidre::io;

settings::json endpoint::cap::setup(flag_type flgs) const {
    return {
        //
        {"packet", !!(flgs & endpoint::cap::packet)},      //
        {"chunk", !!(flgs & endpoint::cap::chunk)},        //
                                                           //
        {"task", !!(flgs & endpoint::cap::task)},          //
        {"realtime", !!(flgs & endpoint::cap::realtime)},  //
                                                           //
        {"push", !!(flgs & endpoint::cap::push)},          //
        {"pull", !!(flgs & endpoint::cap::pull)},          //
        {"event", !!(flgs & endpoint::cap::event)},        //
                                                           //
        {"multi", !!(flgs & endpoint::cap::multi)}         //
    };
}

settings::json endpoint::cap::setup() const {
    settings::json retval;

    if (flags[sink] != endpoint::cap::none)
        retval["sink"] = setup(flags[sink]);
    if (flags[source] != endpoint::cap::none)
        retval["source"] = setup(flags[source]);

    return retval;
}
std::vector<endpoint *> endpoint::sources() noexcept {
    // FIXME
    // if (parent_.get())
    //     return parent_->sources();
    return {};
}