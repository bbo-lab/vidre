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
#include "arguments.hh"
#include "../log.hh"
#include "../exception.hh"

using namespace vidre;

arg::map &arg::map::set(const std::string &key, variant value) {
    for (auto &item : *this) {
        if (item.first == key) {
            item.second = value;
            return *this;
        }
    }

    emplace_back(key, value);

    return *this;
}

arg::variant arg::map::get(const std::string &key) {
    for (auto &item : *this) {
        if (item.first == key)
            return item.second;
    }
    throw vidre::exception() << throw_message("internal error: key lookup failed");
}

bool arg::map::has(const std::string &key) {
    for (auto &item : *this) {
        if (item.first == key)
            return true;
    }
    return false;
}

#if 0

std::filesystem::path arg::parser::invocation_{};
arg::map arg::parser::parsed_{};
std::vector<std::string> arg::parser::unparsed_{};

const arg::option *arg::parser::find_short(char c) {
    for (auto &opt : opts_) {
        if (not opt.long_option)
            break;

        if (opt.short_option == c)
            return &opt;
    }

    return nullptr;
}

const arg::option *arg::parser::find_long(const std::string &name) {
    for (auto &opt : opts_) {
        if (not opt.long_option)
            break;

        if (opt.long_option == name)
            return &opt;
    }

    return nullptr;
}

void arg::parser::handle(const option &opt) {
    if (not opt.long_option)
        throw vidre::exception() << throw_message("internal error");

    switch (opt.flags_) {
        case arg::option::flag_type::no_argument: {
            long value = parsed_.get(opt.long_option);
            parsed_.set(opt.long_option, value + 1);

            if (opt.callback) {
                opt.callback(*this);
            }

            break;
        }
        case arg::option::flag_type::requires_argument:
            if ((unparsed_.size() < 2) or (unparsed_[1][0] == '-')) {
                std::cerr << "expected an argument for '" << opt.long_option << "'\n\n";
                help();
                std::exit(1);
            }
            [[fallthrough]];
        case arg::option::flag_type::optional_argument:
            if ((unparsed_.size() > 1) and (unparsed_[1][0] != '-')) {
                // FIXME handle type in options
                parsed_.set(opt.long_option, unparsed_[1]);
                unparsed_.erase(unparsed_.begin() + 1);
            }
            break;
        case arg::option::flag_type::positional_argument:
            parsed_.set(opt.long_option, unparsed_[0]);
            break;
    }
}

void arg::parser::parse(int argc, const char **argv) {
    log::trace tr;

    // set defaults
    for (auto &opt : opts_) {
        if (not opt.long_option)
            break;

        if (not opt.value.empty())
            parsed_.set(opt.long_option, opt.value);
        else if (opt.flags_ == arg::option::flag_type::no_argument)
            parsed_.set(opt.long_option, 0);
    }

    // copy everything to unparsed_, first
    invocation_ = argv[0];
    for (int c = 1; c < argc; ++c)
        unparsed_.push_back(argv[c]);

    // parse the "unparsed_" list (non-positional first)...
    while (unparsed_.size()) {
        auto &cur = unparsed_[0];

        if (cur[0] == '-') {
            if (cur[1] == '-') {
                // long option
                if (cur.size() == 2) {
                    // positional arguments follow
                    unparsed_.erase(unparsed_.begin());
                    break;
                }

                auto opt = find_long(cur.c_str() + 2);
                if (opt)
                    handle(*opt);
                else {
                    std::cerr << "Argument error: unknown option '" << cur << "'\n\n";
                    help();
                    std::exit(1);
                }

                unparsed_.erase(unparsed_.begin());
            }
            else {
                // short option(s)
                for (size_t s = 1; s < cur.size(); ++s) {
                    auto opt = find_short(cur[s]);
                    if (opt)
                        handle(*opt);
                    else {
                        std::cerr << "Argument error: unknown option '-" << cur[s] << "'\n\n";
                        help();
                        std::exit(1);
                    }
                }

                unparsed_.erase(unparsed_.begin());
            }
        }
        else
            break;
    }

    // positional arguments
    for (auto &opt : opts_) {
        if (not opt.long_option)
            break;
        if (unparsed_.empty())
            break;

        switch (opt.flags_) {
            case arg::option::flag_type::positional_argument:
                handle(opt);
                unparsed_.erase(unparsed_.begin());
                break;
            default:
                break;
        }
    }

    // check unset positional arguments
    for (auto &opt : opts_) {
        if (not opt.long_option)
            break;

        switch (opt.flags_) {
            case arg::option::flag_type::positional_argument:
                if (not parsed_.has(opt.long_option)) {
                    std::cerr << "missing positional argument '" << opt.long_option << "'\n\n";
                    help();
                    std::exit(1);
                }
                break;
            default:
                break;
        }
    }
}

void arg::parser::show() const noexcept {
    log::info info;
    for (auto &item : parsed_) {
        info << item.first << " = ";

        switch (item.second.type()) {
            case arg::variant::none:
                info << "<none>";
                break;
            case arg::variant::natural:
                info << (unsigned long)(item.second);
                break;
            case arg::variant::integer:
                info << long(item.second);
                break;
            case arg::variant::number:
                info << double(item.second);
                break;
            case arg::variant::cstring:
                info << "\"" << (const char *)(item.second) << "\"";
                break;
            case arg::variant::string:
                info << "\"" << std::string(item.second) << "\"";
                break;
            default:
                info << "???";
        }

        info << log::endl;
    }
}

int arg::parser::help() {
    // make list (used for indentation of "brief")
    std::vector<std::pair<std::string, std::string>> optlist;
    size_t leftlen = 0;
    for (auto &opt : opts_) {
        if (not opt.long_option)
            break;
        std::string opttxt;

        opttxt = "--" + std::string(opt.long_option);
        if (opt.short_option)
            opttxt += ",-" + std::string(&opt.short_option, 1);

        switch (opt.flags_) {
            case arg::option::no_argument:
                break;
            case arg::option::flag_type::optional_argument:
                opttxt += " [";
                opttxt += "value";
                opttxt += "]";
                break;
            case arg::option::flag_type::requires_argument:
                opttxt += " ";
                opttxt += "value";
                break;
            default:
                continue;
        }

        if (leftlen < opttxt.size())
            leftlen = opttxt.size();
        optlist.push_back({opttxt, opt.brief});
    }

    // APPLICATON
    std::cout << std::string(invocation_.filename()) << brief_ << std::endl;
    std::cout << std::endl;

    if (description_)
        std::cout << description_ << std::endl << std::endl;

    // USAGE
    std::cout << "Usage:" << std::endl << std::endl;
    std::cout << "    " << std::string(invocation_.filename()) << " [options]";

    bool has_positional = false;
    for (auto &opt : opts_) {
        if (not opt.long_option)
            break;

        switch (opt.flags_) {
            case arg::option::flag_type::positional_argument:
                has_positional = true;
                if (opt.value.empty())
                    std::cout << " <" << opt.long_option << ">";
                else
                    std::cout << " [<" << opt.long_option << ">]";
                break;
            default:
                break;
        }
    }
    std::cout << std::endl << std::endl;

    if (has_positional) {
        std::cout << "  with" << std::endl << std::endl;

        // adjust left indentation
        for (auto &opt : opts_) {
            if (not opt.long_option)
                break;

            switch (opt.flags_) {
                case arg::option::flag_type::positional_argument:
                    if (leftlen < strlen(opt.long_option))
                        leftlen = strlen(opt.long_option);
                    break;
                default:
                    break;
            }
        }

        // output positional
        for (auto &opt : opts_) {
            if (not opt.long_option)
                break;

            switch (opt.flags_) {
                case arg::option::flag_type::positional_argument:
                    std::cout << std::format("    {:<{}}  {}", opt.long_option, leftlen, opt.brief) << std::endl;
                    break;
                default:
                    break;
            }
        }

        std::cout << std::endl;
    }

    // OPTIONS

    // output
    std::cout << "Options:" << std::endl;
    for (auto &opt : optlist) {
        std::cout << std::format("    {:<{}}  {}", opt.first, leftlen, opt.second) << std::endl;
    }

    return 0;
}

#endif