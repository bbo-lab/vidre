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
#include "vidre/io/endpoint/sink.hh"

using namespace vidre;
using namespace vidre::io;

shared<packet> vidre::io::packet::nil = packet::make<packet>();

io::result packet::emit(io::sink &f) {
    static_assert(sizeof(header) == 16);

    finalize();
    return f.put(memory::view{hdr});
}

io::result packet::pad(io::sink &f) const {
    size_t padding = size() - hdr.length - sizeof(header);
    if (padding > 0) {
        static const uint8_t padbuf[alignment] = {};
        return f.put({padding, padbuf});
    }
    return 0;
}

void packet::absorb(memory::view view, std::endian endian) {
    // nothing to be done here packet::decode has taken care of it
}
