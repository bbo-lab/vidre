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
#include "object.hh"
#include "factory.hh"

using namespace vidre;

static io::packet::registry<io::packet::object>  //
    packet_factory_b(io::packet::major::object,  //
                     io::packet::minor::group);

void io::packet::object::update() noexcept {
    key_->update();
    value_->update();
    hdr.length = key_->size() + value_->size();
}

io::result io::packet::object::emit(io::sink &f) {
    auto retval = io::packet::emit(f);
    retval += key_->emit(f);
    retval += value_->emit(f);
    return retval + pad(f);
}

void io::packet::object::absorb(memory::view view, std::endian endian) {
    if (view.size() < size())
        throw exception::runtime() << "packet data is truncated";

    memory::view it = view + sizeof(header);

    key_ = io::packet::factory::make<packet::value>(it,endian);
    it += key_->size();
    value_ = io::packet::factory::make<packet::value>(it,endian);
    it += value_->size();
}

settings::json io::packet::object::to_json() const {
    return {{key_->to_json(), value_->to_json()}};
}
