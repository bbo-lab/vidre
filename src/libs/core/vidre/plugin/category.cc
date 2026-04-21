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
#include "category.hh"

using namespace vidre;

std::string_view plugin::category::name(type t) noexcept {
    switch (t) {
        case type::native:
            return "native";
        case type::script:
            return "script";
        case type::program:
            return "program";
        default:
            return {};
    }
}

plugin::category::type plugin::category::id(std::string_view name) noexcept {
    if (name == "native") {
        return type::native;
    }
    else if (name == "script") {
        return type::script;
    }
    else if (name == "program") {
        return type::program;
    }
    else {
        return static_cast<type>(-1);  // Invalid type
    }
}

std::string_view plugin::interface::name(type t) noexcept {
    switch (t) {
        case type::other:
            return "other";
        case type::sink:
            return "sink";
        case type::source:
            return "source";
        case type::pipe:
            return "pipe";
        default:
            return {};
    }
}

plugin::interface::type plugin::interface::id(std::string_view name) noexcept {
    if (name == "sink") {
        return type::sink;
    }
    else if (name == "source") {
        return type::source;
    }
    else if (name == "pipe") {
        return type::pipe;
    }
    else if (name == "other") {
        return type::other;
    }
    else {
        return static_cast<type>(-1);  // Invalid type
    }
}

std::string_view plugin::endpoint::name(type t) noexcept {
    switch (t) {
        case type::none:
            return "none";
        case type::memory:
            return "memory";
        case type::stdio:
            return "stdio";
        case type::fifo:
            return "fifo";
        case type::socket:
            return "socket";
        default:
            return {};
    }
}

plugin::endpoint::type plugin::endpoint::id(std::string_view name) noexcept {
    if (name == "none") {
        return type::none;
    }
    else if (name == "memory") {
        return type::memory;
    }
    else if (name == "stdio") {
        return type::stdio;
    }
    else if (name == "fifo") {
        return type::fifo;
    }
    else if (name == "socket") {
        return type::socket;
    }
    else {
        return static_cast<type>(-1);  // Invalid type
    }
}
