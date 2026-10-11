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
#include "vidre/io/endpoint/sink.hh"
#include "vidre/util/endian.hh"
#include "vidre/memory/base.hh"
#include "vidre/settings/json.hh"

using namespace vidre;

static io::packet::registry<io::packet::value>        //
    packet_factory_b(io::packet::major::value,        //
                     {io::packet::value::minor::nil,  //
                      io::packet::value::minor::uuid});
static io::packet::registry<io::packet::value>            //
    packet_factory_c(io::packet::major::value,            //
                     {io::packet::value::minor::isodate,  //
                      io::packet::value::minor::utctime});

void io::packet::value::finalize() noexcept {
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
            hdr.length = std::get<std::string>(value_).size();
            break;
        case minor::binary:
            hdr.length = std::get<memory::view>(value_).size();
            break;
        case minor::uuid:
            hdr.length = 16;
            break;
        case minor::isodate:
            hdr.length = sizeof(chrono::isodate);
            break;
        case minor::timestamp:
            hdr.length = sizeof(chrono::timestamp);
            break;
        case minor::utctime:
            hdr.length = sizeof(chrono::utctime);
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
        case minor::binary: {
            auto v = std::get<memory::view>(value_);
            retval += f.put(v);
            break;
        }
        case minor::uuid: {
            auto v = std::get<uuid>(value_);
            retval += f.put(static_cast<memory::view>(v));
            break;
        }
        case minor::isodate: {
            auto v = std::get<chrono::isodate>(value_);
            retval += f.put(static_cast<memory::view>(v));
            break;
        }
        case minor::timestamp: {
            auto v = std::get<chrono::timestamp>(value_);
            retval += f.put(static_cast<memory::view>(v));
            break;
        }
        case minor::utctime: {
            auto v = std::get<chrono::utctime>(value_);
            retval += f.put(static_cast<memory::view>(v));
            break;
        }
    }

    return retval + pad(f);
}

void io::packet::value::absorb(memory::view view, std::endian endian) {
    if (view.size() < size())
        throw exception::runtime() << "packet data is truncated";

    switch (static_cast<minor>(hdr.type.minor)) {
        case minor::nil:
            break;
        case minor::boolean: {
            int64_t v;
            memory::copy(view + sizeof(header), v);
            value_ = static_cast<bool>(endian::byteswap(v, endian));  // FIXME do we need the byteswap for boolean?
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
        case minor::binary: {
            memory::view bin_view = view + sizeof(header);
            memory::view v(hdr.length, bin_view.data());
            value_ = v;
            break;
        }
        case minor::uuid: {
            memory::view uuid_view = view + sizeof(header);
            uuid v(uuid_view);
            value_ = std::move(v);
            break;
        }
        case minor::isodate: {
            memory::view isodate_view = view + sizeof(header);
            chrono::isodate v;
            memory::copy(isodate_view, v);
            value_ = std::move(v);
            break;
        }
        case minor::timestamp: {
            memory::view timestamp_view = view + sizeof(header);
            chrono::timestamp v;
            memory::copy(timestamp_view, v);
            if (endian != std::endian::native) {
                v.sec = endian::byteswap(v.sec, endian);
                v.nsec = endian::byteswap(v.nsec, endian);
            }
            value_ = std::move(v);
            break;
        }
        case minor::utctime: {
            memory::view utctime_view = view + sizeof(header);
            chrono::utctime v;
            memory::copy(utctime_view, v);
            if (endian != std::endian::native) {
                v.sec = endian::byteswap(v.sec, endian);
                v.nsec = endian::byteswap(v.nsec, endian);
            }
            value_ = std::move(v);
            break;
        }
    }
}

settings::json io::packet::value::to_json() const {
    switch (static_cast<minor>(hdr.type.minor)) {
        case minor::nil:
            break;
        case minor::boolean: 
            return std::get<bool>(value_);
        case minor::integer: 
            return std::get<int64_t>(value_);
        case minor::natural: 
            return std::get<uint64_t>(value_);
        case minor::number: 
            return std::get<double>(value_);
        case minor::string: 
            return std::get<std::string>(value_);
        case minor::binary: {
            // FIXME
            return {};
        }
        case minor::uuid: {
            return std::get<uuid>(value_).tostring();
        }
        case minor::isodate: {
            return static_cast<std::string_view>(std::get<chrono::isodate>(value_));
        }
        case minor::timestamp: {
            settings::json node;
            auto value = std::get<chrono::timestamp>(value_);
            node["sec"] = uint64_t(value.sec);
            node["nsec"] = uint64_t(value.nsec);
            return node;
        }
        case minor::utctime: {
            settings::json node;
            auto value = std::get<chrono::utctime>(value_);
            node["sec"] = uint64_t(value.sec);
            node["nsec"] = uint64_t(value.nsec);
            return node;
        }
    }

    throw exception::internal() << "should not be reached";
}
