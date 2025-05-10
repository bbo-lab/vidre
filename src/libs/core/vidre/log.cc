/* VidRe
 * Copyright (C) 2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#include "log.hh"

#include <array>
#include <algorithm>

using namespace vidre;

int vidre::log::threshold_ = vidre::log::default_level;
std::vector<vidre::log::stream *> vidre::log::stream_ = {new vidre::log::stream};

constexpr unsigned indent_precalc_depth = 64;
consteval auto indent_precalc_init() noexcept {
    std::array<char, indent_precalc_depth> buf;
    for (auto &it : buf)
        it = '\t';
    return buf;
}
constexpr auto indent_precalc = indent_precalc_init();

log::~log() {
    // FIXME
}

void log::emit(const string_type &msg) noexcept {
    if (level_ >= threshold_) {
        for (auto *it : stream_)
            it->emit(msg);
    }
}

void log::emit(unsigned depth, const string_type &msg) noexcept {
    if (level_ >= threshold_) {
        depth = std::min(depth, indent_precalc_depth);

        auto ptr = msg.data();
        auto end = ptr + msg.size();

        do {
            auto nl = std::string_view(ptr, end).find_first_of('\n');
            auto nxt = (nl == std::string_view::npos) ? end : ptr + nl + 1;

            for (auto *it : stream_) {
                it->emit({indent_precalc.data(), depth});
                it->emit({ptr, nxt});
                it->emit(eol);
            }

            ptr = nxt;
        } while (ptr < end);
    }
}
