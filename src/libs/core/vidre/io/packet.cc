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
#include "packet.hh"
#include "types/sink.hh"
#include "../util/endian.hh"

using namespace vidre;
using namespace vidre::io;

io::result packet::emit(io::sink &f) const {
    result retval = f.put({sizeof(packet), reinterpret_cast<memory::const_pointer>(this)});
    return retval;
}

// FIXME should be possibly inline (in sink.hh)
io::result packet::pad(io::sink &f) const {
    size_t padding = size() - hdr.length;
    if (padding > 0) {
        static const uint8_t padbuf[alignment] = {};
        return f.put({padding, padbuf});
    }
    return 0;
}

void packet::absorb(memory::view view, std::endian endian) {
    if (sizeof(*this) > view.size())
        throw exception::internal() << "invalid view size for packet absorption";

    // FIXME
    // memory::copy(view, this);

    if (endian != std::endian::native) {
        endian::byteswap(hdr.type.major, endian);
        endian::byteswap(hdr.type.minor, endian);
        endian::byteswap(hdr.stream, endian);
        endian::byteswap(hdr.length, endian);
    }
}
