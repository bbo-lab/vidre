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
#include "color.hh"

#include "../../cpp/util.hh"

using namespace vidre;
using namespace vidre::video;

const char *format::color::primary::name() const noexcept {
    const char *names[] = {"none", "bt709",    "bt470m",   "bt470bg", "smpte170m", "smpte240m",
                           "film", "smpte431", "smpte432", "bt2020",  "jedec-p22"};
    return (id_ < size_of(names)) ? names[id_] : "unknown";
}

const char *format::color::space::name() const noexcept {
    const char *names[] = {"none", "bt709", "fcc", "bt470bg", "smpte170m", "smpte240m", "ycgco", "bt2020ncl"};
    return (id_ < size_of(names)) ? names[id_] : "unknown";
}

const char *format::color::transfer::name() const noexcept {
    const char *names[] = {"none",      "bt709", "bt470m",       "bt470bg",      "gamma22", "gamma28",   "smpte170m",
                           "smpte240m", "srgb",  "iec61966-2-1", "iec61966-2-4", "xvycc",   "bt2020-10", "bt2020-12"};
    return (id_ < size_of(names)) ? names[id_] : "unknown";
}

const char *format::color::quantization::name() const noexcept {
    const char *names[] = {"none", "tv", "mpeg", "pc", "jpeg"};
    return (id_ < size_of(names)) ? names[id_] : "unknown";
}

settings::json format::color::json() const noexcept {
    return {{"primary", primary_.name()},
            {"space", space_.name()},
            {"transfer", transfer_.name()},
            {"quantization", quantization_.name()}};
}