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
#include "value.hh"
#include "factory.hh"
#include "../types/sink.hh"
#include "../../util/endian.hh"
#include "vidre/memory/base.hh"

using namespace vidre;

static io::packet::factory::registrar<io::packet::value>  //
    packet_factory_b(io::packet::major::value,            //
                     {io::packet::value::minor::nil,      //
                      io::packet::value::minor::uuid});
static io::packet::factory::registrar<io::packet::value>  //
    packet_factory_c(io::packet::major::value,            //
                     {io::packet::value::minor::isodate,  //
                      io::packet::value::minor::utctime});

void io::packet::value::update() noexcept {
    switch (static_cast<minor>(hdr.type.minor)) {
        case minor::nil:
            hdr.length = 0;
            break;
        case minor::boolean:
            hdr.length = sizeof(uint64_t);
            break;
        case minor::integer:
            hdr.length = sizeof(int64_t);
            break;
        case minor::natural:
            hdr.length = sizeof(uint64_t);
            break;
        case minor::number:
            hdr.length = sizeof(double);
            break;
        case minor::string:
            hdr.length = std::get<std::string>(value_).size() - 1;
            break;
        case minor::binary:
            hdr.length = std::get<std::vector<uint8_t>>(value_).size();
            break;
        case minor::uuid:
            hdr.length = sizeof("00000000-0000-0000-0000-000000000000") - 1;
            break;
        case minor::isodate:
            hdr.length = sizeof(chrono::isodate);  // FIXME
            break;
        case minor::timestamp:
            hdr.length = sizeof(chrono::timestamp);  // FIXME
            break;
        case minor::utctime:
            hdr.length = sizeof(chrono::clock);  // FIXME
            break;
    }
}

io::result io::packet::value::emit(io::sink &f) {
    auto retval = io::packet::emit(f);

    switch (static_cast<minor>(hdr.type.minor)) {
        case minor::nil:
            break;
        case minor::boolean: {
            auto v = static_cast<int64_t>(std::get<bool>(value_));
            retval += f.put(v);
            break;
        }
        case minor::integer: {
            auto v = std::get<int64_t>(value_);
            retval += f.put(v);
            break;
        }
        case minor::natural: {
            auto v = std::get<uint64_t>(value_);
            retval += f.put(v);
            break;
        }
        case minor::number: {
            auto v = std::get<double>(value_);
            retval += f.put(v);
            break;
        }
        case minor::string: {
            auto v = std::get<std::string>(value_);
            retval += f.put({v.size(), v.data()});
            break;
        }
        case minor::binary:
            // FIXME
            break;
        case minor::uuid: {
            auto v = std::get<uuid>(value_).tostring();
            retval += f.put({v.size(), v.data()});
            break;
        }
        case minor::isodate:
            // FIXME
            break;
        case minor::timestamp:
            // FIXME
            break;
        case minor::utctime:
            // FIXME
            break;
    }

    return retval;
}

void io::packet::value::absorb(memory::view view, std::endian endian) {
    switch (static_cast<minor>(hdr.type.minor)) {
        case minor::nil:
            break;
        case minor::boolean: {
            int64_t v;
            memory::copy(view + sizeof(header), v);
            value_ = static_cast<bool>(endian::byteswap(v, endian));
            break;
        }
        case minor::integer: {
            int64_t v;
            memory::copy(view + sizeof(header), v);
            value_ = endian::byteswap(v, endian);
            break;
        }
        case minor::natural: {
            uint64_t v;
            memory::copy(view + sizeof(header), v);
            value_ = endian::byteswap(v, endian);
            break;
        }
        case minor::number: {
            double v;
            memory::copy(view + sizeof(header), v);
            value_ = endian::byteswap(v, endian);
            break;
        }
        case minor::string: {
            memory::view str_view = view + sizeof(header);
            std::string v(reinterpret_cast<const char *>(str_view.data()), hdr.length);
            value_ = std::move(v);
            break;
        }
        case minor::binary:
            // FIXME
            break;
        case minor::uuid:
            // FIXME
            break;
        case minor::isodate:
            // FIXME
            break;
        case minor::timestamp:
            // FIXME
            break;
        case minor::utctime:
            // FIXME
            break;
    }
}