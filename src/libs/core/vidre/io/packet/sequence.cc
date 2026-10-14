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
#include "sequence.hh"
#include "factory.hh"

using namespace vidre;

static io::packet::registry<io::packet::sequence>        //
    packet_factory_b(io::packet::major::sequence,        //
                     0);

void io::packet::sequence::finalize() noexcept {
    hdr.length = 0;
    for (auto &pkt : packets_) {
        pkt->finalize();
        hdr.length += pkt->size();
    }
}

io::result io::packet::sequence::emit(io::sink &f) {
    auto retval = io::packet::emit(f);
    for (auto &pkt : packets_)
        retval += pkt->emit(f);
    return retval + pad(f);
}

void io::packet::sequence::absorb(memory::view view, std::endian endian) {
    if (view.size() < size())
        throw exception::runtime() << "packet data is truncated";

    memory::view it = view + sizeof(header);
    memory::const_pointer end_it = view.begin() + size();

    while (it.begin() < end_it) {
        auto pkt = io::packet::factory::make(it,endian);
        it += pkt->size();
        packets_.push_back(std::move(pkt));
    }
}
settings::json io::packet::sequence::to_json() const {
    settings::json node; 
    for (const auto &pkt : packets_)
        node.push_back(pkt->to_json());
    return node;
}
