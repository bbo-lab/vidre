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

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif

bool is_clog_terminal() {
    return ISATTY(FILENO(stderr)) != 0;
}

using namespace vidre;

static log::fret vidre_use_fret() {
    if (is_clog_terminal())
        return log::fret::color;
    else
        return log::fret::tag;
}

static vidre::log::stream vidrelog{vidre_use_fret()};

std::vector<vidre::log::stream *> vidre::log::stream_ = {&vidrelog};

void log::remove(unsigned id) noexcept {
    if (id == 0)
        return;
    else if (id >= stream_.size())
        return;
    else {
        stream_[id] = nullptr;

        while (stream_.back() == nullptr)
            stream_.pop_back();
    }
}

constexpr unsigned indent_precalc_depth = 64;
consteval auto indent_precalc_init() noexcept {
    std::array<char, indent_precalc_depth> buf;
    for (auto &it : buf)
        it = '\t';
    return buf;
}
constexpr auto indent_precalc = indent_precalc_init();

void log::emit(const message_type &msg, unsigned depth) noexcept {
    depth = std::min(depth, indent_precalc_depth);

    // split lines
    std::vector<message_type> lines;

    auto ptr = msg.data();
    auto end = ptr + msg.size();

    do {
        auto nl = std::string_view(ptr, end).find_first_of('\n');
        auto nxt = (nl == std::string_view::npos) ? end : ptr + nl + 1;

        lines.emplace_back(ptr, nxt);

        ptr = nxt;
    } while (ptr < end);

    // output per stream
    for (auto *it : stream_) {
        if (level_ > it->threshold())
            continue;

        auto &fret = log::stream::frets[it->fret_][level_ - log::min_level];

        for (auto &txt : lines) {
            if (first_) {
                it->write(fret.first);
                first_ = false;
            }
            else
                it->write(fret.prefix);

            it->write(txt);

            it->write(fret.suffix);
        }
    }

}

static const vidre::log::stream::fret vidre_log_fret_plain[vidre::log::level_n] = {
    {{}, {}, {}},  //
    {{}, {}, {}},  //
    {{}, {}, {}},  //
    {{}, {}, {}},  //
    {{}, {}, {}},  //
    {{}, {}, {}},  //
    {{}, {}, {}},  //
    {{}, {}, {}},  //
    {{}, {}, {}},  //
    {{}, {}, {}}   //
};

static const vidre::log::stream::fret vidre_log_fret_color[vidre::log::level_n] = {
    {"\e[1;91m", "\e[1;91m", "\e[0m"},          // fatal
    {"\e[4;31m", "\e[4;31m", "\e[0m"},          // alert
    {"\e[0;91m", "\e[0;91m", "\e[0m"},          // critical
    {"\e[0;31m", "\e[0;31m", "\e[0m"},          // error
    {"\e[0;33m", "\e[0;33m", "\e[0m"},          // warning
    {"\e[0;97m", "\e[0;97m", "\e[0m"},          // notice
    {"\e[0;92m", "\e[0;92m", "\e[0m"},          // status
    {"\e[0m", "\e[0m", "\e[0m"},                // info
    {"\e[38:5:246m", "\e[38:5:246m", "\e[0m"},  // debug
    {"\e[38:5:252m", "\e[38:5:252m", "\e[0m"}   // trace
};

static const vidre::log::stream::fret vidre_log_fret_tag[vidre::log::level_n] = {
    {"[FATAL] ", "    ", {}},     //
    {"[ALERT] ", "    ", {}},     //
    {"[CRITICAL] ", "    ", {}},  //
    {"[ERROR] ", "    ", {}},     //
    {"[WARNING] ", "    ", {}},   //
    {"[NOTICE] ", "    ", {}},    //
    {"[STATUS] ", "    ", {}},    //
    {"[INFO] ", "    ", {}},      //
    {"[DEBUG] ", "    ", {}},     //
    {"[TRACE] ", "    ", {}}      //
};

const log::stream::fret *vidre::log::stream::frets[] = {vidre_log_fret_plain, vidre_log_fret_color, vidre_log_fret_tag};