/* VidRe
 * Copyright (C) 2025-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#include "cpp/util.hh"
#include "exception.hh"
#include "io/file.hh"

#include <array>
#include <algorithm>
#include <chrono>
#include <ranges>
#include <string_view>

using namespace vidre;

thread_local io::detail::buffer vidre::lognew::detail::thread::buffer_{VIDRE_CORE_LOG_BUFFER_SIZE};

// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================

using namespace std::literals::string_view_literals;

static log::io::style::type vidre_use_fret() {
    if (vidre::os::isatty())
        return log::io::style::type::color;
    else
        return log::io::style::type::tagged;
}

static vidre::log::stream vidrelog{vidre_use_fret()};

std::vector<vidre::log::io::stream *> vidre::log::io::stream::channels = {&vidrelog};
int vidre::log::io::stream::maxthreshold = vidrelog.threshold_;

void log::io::stream::put(const message_type &msg) noexcept {
    vidre::io::standard::error.write({msg.size(), msg.data()});
}

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
    outs_ = ::fopen(std::string(filename).c_str(), "a");
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

static struct {
    const char *name;
    log::level level;
    const char *brief;
} setup_levels[] = {
    {"fatal", log::level::fatal, "System is in an unusable state and the program will terminate."},                //
    {"alert", log::level::alert, "Immediate user interaction is required, usually when data corruption occurs."},  //
    {"critical", log::level::critical, "A critical error, such as hardware failure."},                             //
    {"error", log::level::error, "An application error occurred."},                                                //
    {"warning", log::level::warning, "Warning level, which may hint to future failure."},                          //
    {"notice", log::level::notice, "A noteworthy condition occurred."},                                            //
    {"status", log::level::status, "A status message."},                                                           //
    {"info", log::level::info, "A purely informational message."},                                                 //
    {"debug", log::level::debug, "Debugging information."},                                                        //
    {"trace", log::level::trace, "Tracing."},                                                                      //
};

static constexpr int level_invalid = std::numeric_limits<int>::min();
static std::optional<std::array<std::string_view, 3>> split_option(const std::string_view option) {
    std::array<std::string_view, 3> result;

    size_t i = 0;
    for (const auto part : std::views::split(option, ":"sv)) {
        if (i == 3)
            return std::nullopt;

        result[i++] = std::string_view(part);
    }

    return result;
}

static int tolevel(const std::string_view &str) {
    for (size_t i = 0; i < size_of(setup_levels); ++i) {
        if (str == setup_levels[i].name)
            return static_cast<int>(setup_levels[i].level);
    }
    return level_invalid;
}

static std::vector<log::io::file> vidre_log_files;

void log::setup(const std::string &def) {
    if (def == "help") {
        vidre::io::standard::output.write(                                                    //
            "Log-levels can be set for console output, but also file logging is possible.\n"  //
            "The general format is:\n"                                                        //
            "\n"                                                                              //
            "    <option>[,<option>]...\n"                                                    //
            "\n"                                                                              //
            "where option is\n"                                                               //
            "\n"                                                                              //
            "   <log-level>\n"                                                                //
            "   trace:<pattern>\n"                                                            //
            "\n"                                                                              //
            "to set the log level for console output or\n"                                    //
            "\n"                                                                              //
            "   <file-name>:<log-level>\n"                                                    //
            "   <file-name>:trace:<pattern>\n"                                                //
            "\n"                                                                              //
            "to log message of the given level to a file.\n"                                  //
            "\n"                                                                              //
            "For tracing, the wildcard <pattern>, referring to the source code path(s),\n"    //
            "has to be provided.\n"                                                           //
            "\n"                                                                              //
            "Available log levels are:\n");

        for (size_t i = 0; i < size_of(setup_levels); ++i) {
            vidre::io::standard::output.write(std::format("    {:<20} {}\n", setup_levels[i].name, setup_levels[i].brief));
        }
    }
    else {
        for (const auto option : std::views::split(def, ","sv)) {
            auto def = split_option(std::string_view(option));

            if (def) {
                auto const &part = *def;

                if (not part[2].empty()) {
                    int threshold = tolevel(part[1]);
                    if (threshold == static_cast<int>(level::trace)) {
                        // <file-name>:trace:<pattern>
                        vidre_log_files.emplace_back(part[0], static_cast<log::level>(threshold));
                        log::trace::pattern(part[2]);
                        continue;
                    }
                    else if (threshold != level_invalid) {
                        // <file-name>:<log-level>:<pattern>
                        // (pattern is ignored)
                        vidre_log_files.emplace_back(part[0], static_cast<log::level>(threshold));
                        continue;
                    }
                }
                else if (not part[1].empty()) {
                    int threshold = tolevel(part[0]);
                    if (threshold == static_cast<int>(level::trace)) {
                        // trace:<pattern>
                        log::io::stream::threshold(0, level::trace);
                        log::trace::pattern(part[1]);
                        continue;
                    }
                    else if (threshold == level_invalid) {
                        threshold = tolevel(part[1]);
                        if (threshold != level_invalid) {
                            // <file-name>:<log-level>
                            vidre_log_files.emplace_back(part[0], static_cast<log::level>(threshold));
                            continue;
                        }
                    }
                }
                else {
                    int threshold = tolevel(part[0]);
                    if ((threshold != level_invalid) and (threshold != static_cast<int>(level::trace))) {
                        // <log-level>
                        // (trace not allowed, missing pattern)
                        log::io::stream::threshold(0, static_cast<log::level>(threshold));
                        continue;
                    }
                }
            }

            std::string cerr;
            cerr = "invalid log setup option: ";
            cerr += std::string_view(option);
            cerr += "\n";
            cerr += "use 'help' for further information.\n";
            vidre::io::standard::error.write(cerr);

            std::exit(-1);
        }
    }
}
