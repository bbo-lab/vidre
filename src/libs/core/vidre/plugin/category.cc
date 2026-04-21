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
    } else if (name == "script") {
        return type::script;
    } else if (name == "program") {
        return type::program;
    } else {
        return static_cast<type>(-1); // Invalid type
    }
}