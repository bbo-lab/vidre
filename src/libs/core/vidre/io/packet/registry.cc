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
#include "registry.hh"
#include <bit>

using namespace vidre;

std::vector<vidre::io::details::factory::bucket_type> vidre::io::details::factory::bucket{};
std::array<uint8_t, 256> vidre::io::details::factory::index = [] {
    std::array<uint8_t, 256> a;
    a.fill(0xFF);
    return a;
}();

void vidre::io::details::factory::add(uint8_t major, uint8_t minor_from, uint8_t minor_to, make_type make) {
    auto major_index = packet::id::cast(major);

    // get or create the bucket for the major index
    if (index[major_index] == 0xFF) {
        index[major_index] = static_cast<uint8_t>(bucket.size());
        bucket.emplace_back();
    }

    auto &b = bucket[index[major_index]];

    // check if already registered for the minor range
    for (uint8_t minor = minor_from; minor <= minor_to; ++minor) {
        if (b.index[minor] != 0xFF) {
            throw exception::logic() << "packet type already registered";
        }
        b.index[minor] = static_cast<uint8_t>(b.construct.size());
    }

    // insert construct function into bucket
    uint8_t idx = static_cast<uint8_t>(b.construct.size());
    b.construct.push_back(make);
    for (uint8_t minor = minor_from; minor <= minor_to; ++minor) {
        b.index[minor] = idx;
    }
}

shared<io::packet> vidre::io::details::factory::make(packet::header hdr) {
    auto a = index[hdr.type.major];
    if (a == 0xFF)
        throw exception::logic() << "packet type not registered";

    auto b = bucket[a].index[hdr.type.minor];
    if (b == 0xFF)
        throw exception::logic() << "packet type not registered";

    return bucket[a].construct[b](hdr);
}

shared<io::packet> vidre::io::packet::decode(memory::view view, std::endian endian) {
    packet::header hdr;
    memory::copy(view, hdr);

    if (endian != std::endian::native) {
        hdr.stream = endian::byteswap(hdr.stream, endian);
        hdr.length = endian::byteswap(hdr.length, endian);
    }

    auto pkt = details::factory::make(hdr);
    pkt->absorb(view, endian);
    return pkt;
}