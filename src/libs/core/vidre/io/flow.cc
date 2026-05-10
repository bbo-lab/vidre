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
#include "flow.hh"

using namespace vidre;
using namespace vidre::io;

route::route(endpoint::shared src, endpoint::shared snk, std::string tag) : tag_(std::move(tag)) {
    if (not io::endpoint::match(src, snk))
        throw exception::logic() << "incompatible source and sink";

    // check, what we need to do


    // book keeping
    snk->parent_ = src;

    // if (src->tag().empty())
    //     src->tag("source");
    // items_.emplace(src->tag().c_str(), item{src, nullptr});

    // if (snk->tag().empty())
    //     snk->tag("sink");
    // items_.emplace(snk->tag().c_str(), item{snk, nullptr});


}

void route::execute() {
    //@todo implement
}