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
#include "os/tty.hh"

#include <array>
#include <algorithm>
#include <chrono>

using namespace vidre;

inline bool is_clog_terminal() {
    return vidre::isatty(stderr);
}

static log::io::style::type vidre_use_fret() {
    if (is_clog_terminal())
        return log::io::style::type::color;
    else
        return log::io::style::type::tagged;
}

static vidre::log::stream vidrelog{vidre_use_fret()};

std::vector<vidre::log::io::stream *> vidre::log::io::stream::channels = {&vidrelog};
int vidre::log::io::stream::maxthreshold = vidrelog.threshold_;

void log::io::stream::remove(unsigned chanid) noexcept {
    if (chanid == 0)
        return;
    else if (chanid >= channels.size())
        return;
    else {
        channels[chanid] = nullptr;

        while (channels.back() == nullptr)
            channels.pop_back();

        maxthreshold = 0;
        for (auto *it : channels) {
            if (it->threshold_ > maxthreshold)
                maxthreshold = it->threshold_;
        }
    }
}

log::level log::io::stream::threshold(unsigned chanid) noexcept {
    if (chanid >= channels.size())
        return log::level::fatal;
    else {
        return static_cast<log::level>(channels[chanid]->threshold_);
    }
}

void log::io::stream::threshold(unsigned chanid, int lvl) noexcept {
    if (chanid >= channels.size())
        return;
    else {
        channels[chanid]->threshold_ = lvl;
        if (lvl > maxthreshold)
            maxthreshold = lvl;
    }
}

struct style_tags {
    log::message_type prefix;
    log::message_type suffix;
};

static const style_tags vidre_log_style_color[log::level_n] = {
    {"\e[1;91m", "\e[0m"},      // fatal
    {"\e[4;31m", "\e[0m"},      // alert
    {"\e[0;91m", "\e[0m"},      // critical
    {"\e[0;31m", "\e[0m"},      // error
    {"\e[0;33m", "\e[0m"},      // warning
    {"\e[0;97m", "\e[0m"},      // notice
    {"\e[0;92m", "\e[0m"},      // status
    {"\e[0m", "\e[0m"},         // info
    {"\e[38:5:244m", "\e[0m"},  // debug
    {"\e[38:5:239m", "\e[0m"}   // trace
};

static const style_tags vidre_log_style_tagged[log::level_n] = {
    {"[FATAL] ", {}},     //
    {"[ALERT] ", {}},     //
    {"[CRITICAL] ", {}},  //
    {"[ERROR] ", {}},     //
    {"[WARNING] ", {}},   //
    {"[NOTICE] ", {}},    //
    {"[STATUS] ", {}},    //
    {"[INFO] ", {}},      //
    {"[DEBUG] ", {}},     //
    {"", {}}              //
};

static const char *vidre_log_spaces = "                    ";

void log::handler::emit(const message_type &txt, unsigned indent [[maybe_unused]]) noexcept {
    if (txt.size() == 0)
        return;

    auto ptr = txt.data();
    auto end = ptr + txt.size();
    unsigned sty = level_ - level_min;
    assert(sty < level_n);

    do {
        auto nl = std::string_view(ptr, end).find_first_of('\n');
        auto nxt = (nl == std::string_view::npos) ? end : ptr + nl + 1;
        auto last = nxt[-1];

        for (auto *it : io::stream::channels) {
            if (level_ > it->threshold_)
                continue;

#if defined(DEBUG)
            // indent
            if ((last_ == 0) || (last_ == '\n')) {
                for (unsigned i = 0; i < indent; ++i)
                    it->put(indent_s);
            }
#endif

            // prefix
            switch (it->style_) {
                case io::style::plain:
                    break;
                case io::style::tagged:
                    if (last_ == 0)
                        it->put(vidre_log_style_tagged[sty].prefix);
                    else if (last_ == '\n')
                        it->put({vidre_log_spaces, vidre_log_style_tagged[sty].prefix.size()});
                    break;
                case io::style::color:
                    if ((last_ == 0) || (last_ == '\n'))
                        it->put(vidre_log_style_color[sty].prefix);
                    break;
            }

            // text
            it->put({ptr, nxt});

            // suffix
            switch (it->style_) {
                case io::style::plain:
                    break;
                case io::style::tagged:
                    break;
                case io::style::color:
                    if (last == '\n')
                        it->put(vidre_log_style_color[sty].suffix);
                    break;
            }
        }

        ptr = nxt;
        last_ = last;
    } while (ptr < end);
}

log::io::file::file(const std::string_view filename, level th, style::type sty) noexcept : stream(th, sty) {
    outs_ = ::fopen(filename.data(), "a");
    if (outs_ == nullptr)
        log::warning() << "Opening log file '" << filename << "' failed.\n";
    else {
        chanid_ = add(*this);
        header();
    }
}

void log::io::file::header() noexcept {
    if (ftell(outs_) > 0)
        put("\n");

    put("[[[=== ");

    auto now = std::chrono::utc_clock::now();
    put(std::format("{:%Y-%m-%dT%H:%M:%SZ}", now));

    put(" ===]]]\n");
}

#if defined(DEBUG)

std::vector<std::source_location> vidre::log::trace::stack{};
std::regex vidre::log::trace::match{};

void vidre::log::trace::pattern(std::string_view wild) {
    if (wild == "*") {
        stack.emplace_back();
    }
    else {
        std::string pat = std::string{wild};
        pat = std::regex_replace(pat, std::regex("[.]"), "[.]");
        pat = std::regex_replace(pat, std::regex("[?]"), ".");
        pat = std::regex_replace(pat, std::regex("[*]"), ".*");
        match = std::regex(pat);
    }
}

log::trace::trace(std::source_location source) noexcept : log::handler{log::level::trace}, follow_{stack.size()} {
    if (io::stream::maxthreshold < level_)
        return;

    if ((stack.size() == 0) and not(std::regex_match(source.file_name(), match))) {
        return;
    }

    stack.push_back(source);

    *this << "[" << file_name()                //
          << " " << line() << ":" << column()  //
          << ", " << function_name() << "]" << endl;
}

log::trace::~trace() {
    if (stack.size() > 0)
        stack.pop_back();
}

#endif
