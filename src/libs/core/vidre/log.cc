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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "log.hh"
#include "os/tty.hh"
#include "cpp/util.hh"
#include "io/file.hh"

#include <algorithm>
#include <memory>

using namespace std::literals::string_view_literals;
using namespace vidre;

static io::packet::registry<log::packet>                        //
    log_packet_factory(io::packet::major::log,                  //
                       {static_cast<uint16_t>(log::level_min),  //
                        static_cast<uint16_t>(log::level_max)});

#if defined(DEBUG)
log::level vidre::log::source::maxthreshold = log::level::trace;
#elif defined(NDEBUG)
log::level vidre::log::source::maxthreshold = log::level::notice;
#else
log::level vidre::log::source::maxthreshold = log::level::status;
#endif

shared<log::source> vidre::log::output = io::endpoint::make<log::source>();

// levels =====================================================================
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
    {"user", log::level::user, "A user-level message."},                                                           //
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

// io::standard::error ========================================================
struct style_tag {
    std::string_view prefix;
    std::string_view suffix;
};

static const style_tag vidre_log_style_color[] = {
    {"\e[1;91m", "\e[0m"},             // fatal
    {"\e[4;31m", "\e[0m"},             // alert
    {"\e[0;91m", "\e[0m"},             // critical
    {"\e[0;31m", "\e[0m"},             // error
    {"\e[0;33m", "\e[0m"},             // warning
    {"\e[0;97m", "\e[0m"},             // user
    {"\e[0;97m", "\e[0m"},             // notice
    {"\e[0;92m", "\e[0m"},             // status
    {"\e[0m", "\e[0m"},                // info
    {"\e[38:5:244m", "\e[0m"},         // debug
    {"\e[38:5:239m[TRACE] ", "\e[0m"}  // trace
};

static_assert(size_of(vidre_log_style_color) == log::level_n);

static const style_tag vidre_log_style_tagged[] = {
    {"[FATAL] ", {}},     //
    {"[ALERT] ", {}},     //
    {"[CRITICAL] ", {}},  //
    {"[ERROR] ", {}},     //
    {"[WARNING] ", {}},   //
    {"[USER] ", {}},      //
    {"[NOTICE] ", {}},    //
    {"[STATUS] ", {}},    //
    {"[INFO] ", {}},      //
    {"[DEBUG] ", {}},     //
    {"[TRACE] ", {}}      //
};

static_assert(size_of(vidre_log_style_tagged) == log::level_n);

static const std::string_view vidre_log_indent_s = "                                                                        ";

void vidre_log_indent(std::size_t indent) {
    indent *= log::indent_s.size();
    while (indent > 0) {
        auto outn = std::min(indent, vidre_log_indent_s.size());
        io::standard::error.write({outn, vidre_log_indent_s.data()});
        indent -= outn;
    }
}

void vidre_log_emit_standard(style_tag sty, const std::string &msg, unsigned indent) {
    // output per line
    for (size_t start = 0; start < msg.size();) {
        size_t end = msg.find('\n', start);
        if (end == std::string::npos)  // shouldn't happen, see emit()
            end = msg.size();

        size_t len = end - start;
        if (len > 0 && msg[start + len - 1] == '\r')
            --len;

        io::standard::error.write({sty.prefix.size(), sty.prefix.data()});
        vidre_log_indent(indent);
        if (start == 0) {
            // first line of the message
            io::standard::error.write({len, msg.data() + start});
        }
        else {
            io::standard::error.write({len, msg.data() + start});
        }
        io::standard::error.write({sty.suffix.size(), sty.suffix.data()});
        io::standard::error.write({log::endl.size(), log::endl.data()});

        if (end == msg.size())
            break;
        start = end + 1;
    }
}

void vidre_log_emit_standard(log::level lvl, const std::string &msg, unsigned indent) {
    static std::mutex mtx{};
    std::lock_guard<std::mutex> lock(mtx);

    auto level = to_underlying(lvl);
    if (level > log::level_max)
        level = log::level_max;

    if (/*(lvl <= log::level::debug) &&*/ vidre::os::isatty())
        vidre_log_emit_standard(vidre_log_style_color[level], msg, indent);
    else {
        auto tag = vidre_log_style_tagged[level];
        vidre_log_emit_standard(tag, msg, indent);
    }
}

void log::message::emit() noexcept {
    // empty log messages are ignored
    if (msg_->message.empty())
        return;

    // ensure newline at the end of the message
    if (msg_->message.end()[-1] != '\n')
        msg_->message += '\n';

#if defined(DEBUG)
    // emit the log message using the standard output function
    vidre_log_emit_standard(static_cast<log::level>(msg_->getheader().type.minor), msg_->message, 0);
#endif

    // emit packet
    log::output->put(std::move(msg_));
}

void log::packet::absorb(memory::view view, std::endian endian) {
    if (view.size() < size())
        throw exception::runtime() << "packet data is truncated";

    view += sizeof(header);
    memory::copy(view, timestamp);

    view += sizeof(timestamp);
    message.resize(hdr.length - sizeof(timestamp) + 1);
    memory::copy(view, {message.size(), reinterpret_cast<uint8_t *>(message.data())});
}

status log::source::put(shared<io::packet> pkt) {
    auto lptr = std::static_pointer_cast<log::packet>(pkt);
    auto curlvl = static_cast<unsigned>(lptr->getlevel());

    if (curlvl > static_cast<unsigned>(log::source::maxthreshold))
        return vidre::status::ok;

    for (auto &branch : branches_out_.snapshot()) {
        auto bptr = std::static_pointer_cast<log::source::branch>(branch);
        if (curlvl > static_cast<unsigned>(bptr->threshold_))
            continue;
        branch->put(pkt);
    }

    return vidre::status::ok;
}

shared<io::endpoint::source> log::source::branch_source() {
    shared<log::source::branch> branch = endpoint::make<log::source::branch>(*this);
    branch->threshold_ = log::source::maxthreshold;

    branches_out_.append(branch);

    return branch;
}

void log::source::branch::setup(const settings::json &config) {
    if (config.empty())
        return;

    if (config.contains("log")) {
        auto log_config = config["log"];
        if (log_config.empty() || !log_config.is_object())
            return;

        std::string threshold = log_config.value("threshold", "");
        if (!threshold.empty()) {
            auto lvl = tolevel(threshold);
            if (lvl != level_invalid)
                this->threshold_ = static_cast<log::level>(lvl);
        }

        std::string style = log_config.value("style", "plain");
        if (style == "plain")
            this->style_ = log::style::plain;
        else if (style == "tagged")
            this->style_ = log::style::tagged;
        else if (style == "color")
            this->style_ = log::style::color;
    }
}

#if defined(DEBUG)

thread_local std::vector<std::source_location> vidre::log::trace::stack = {};
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

log::trace::trace(std::source_location source) noexcept : log::message{log::level::trace}, follow_{stack.size()} {
    if (log::source::maxthreshold < msg_->getlevel())
        return;

    if (not follow_ and not std::regex_match(source.file_name(), match))
        return;

    stack.push_back(source);

    *this << "[" << file_name()                //
          << " " << line() << ":" << column()  //
          << ", " << function_name() << "]" << endl;
}

log::trace::~trace() {
    if (stack.size() > 0)
        stack.pop_back();

    emit(true);
}

void log::trace::emit(bool last) noexcept {
    // empty log messages are ignored
    if (message_.empty())
        return;

    if (last && (message_.end()[-1] != '\n'))
        message_ += '\n';

    // emit the log message using the standard output function
    vidre_log_emit_standard(static_cast<log::level>(level::trace), message_, follow_ - 1);

    // emit packet
    write(message_);
    log::output->put(std::move(msg_));
    if (not last)
        msg_ = packet::make(level::trace, 0);
}

void log::trace::emit(const std::string_view &txt) noexcept {
    if (log::source::maxthreshold < msg_->getlevel())
        return;

    if (follow_ == 0)
        return;

    auto eol = txt.find_last_of('\n');
    if (eol == std::string_view::npos) {
        message_ += txt;
        return;
    }

    message_ += txt.substr(0, eol + 1);
    emit();

    message_ = txt.substr(eol + 1);
}
#endif

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
                        // FIXME vidre_log_files.emplace_back(part[0], static_cast<log::level>(threshold));
                        log::trace::pattern(part[2]);
                        continue;
                    }
                    else if (threshold != level_invalid) {
                        // <file-name>:<log-level>:<pattern>
                        // (pattern is ignored)
                        // FIXME vidre_log_files.emplace_back(part[0], static_cast<log::level>(threshold));
                        continue;
                    }
                }
                else if (not part[1].empty()) {
                    int threshold = tolevel(part[0]);
                    if (threshold == static_cast<int>(level::trace)) {
                        // trace:<pattern>
                        // FIXME log::io::stream::threshold(0, level::trace);
                        log::trace::pattern(part[1]);
                        continue;
                    }
                    else if (threshold == level_invalid) {
                        threshold = tolevel(part[1]);
                        if (threshold != level_invalid) {
                            // <file-name>:<log-level>
                            // FIXME vidre_log_files.emplace_back(part[0], static_cast<log::level>(threshold));
                            continue;
                        }
                    }
                }
                else {
                    int threshold = tolevel(part[0]);
                    if ((threshold != level_invalid) and (threshold != static_cast<int>(level::trace))) {
                        // <log-level>
                        // (trace not allowed, missing pattern)
                        // FIXME log::io::stream::threshold(0, static_cast<log::level>(threshold));
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
