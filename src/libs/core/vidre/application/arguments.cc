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

std::string arg::parser::make_option_help(const arg::option &opt) {
    std::string retval;

    retval += "--";
    retval += opt.long_option;
    if (opt.short_option) {
        retval += ",-";
        retval += opt.short_option;
    }

    return retval;
}

int arg::parser::help() {
    // APPLICATON
    std::cout << std::string(invocation_.filename()) << std::endl;
    // FIXME version
    std::cout << std::endl;

    // FIXME description if available

    // USAGE
    std::cout << "Usage:" << std::endl;
    std::cout << "    " << std::string(invocation_.filename()) << " [options]";
    // FIXME positional arguments
    std::cout << std::endl << std::endl;

    // OPTIONS
    std::cout << "Options:" << std::endl;
    for (auto &opt : opts_) {
        switch (opt.flags_) {
            case arg::option::flags::no_argument:
                std::cout << make_option_help(opt) << " " << opt.brief << std::endl;
                break;
            case arg::option::flags::optional_argument:
            case arg::option::flags::requires_argument:
                break;
            default:
                break;
        }
    }

    return 0;
}

static arg::option nooption{};

const arg::option &arg::parser::find_short(char c) {
    for (auto &opt : opts_) {
        if (opt.short_option == c)
            return opt;
    }
    return nooption;
}

void arg::parser::handle(const option &opt) {
    if (not opt.long_option)
        throw vidre::exception() << throw_message("internal error");

    switch (opt.flags_) {
        case arg::option::flags::no_argument:
            if (opt.callback)
                opt.callback(*this);
            else {
                // FIXME
            }
            break;
        case arg::option::flags::optional_argument:
            break;
        case arg::option::flags::requires_argument:
            break;
        case arg::option::flags::positional_argument:
            break;
    }
}

void arg::parser::parse(int argc, const char **argv) {
    log::trace tr;

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

                // FIXME
            }
            else {
                // short option(s)
                for (size_t s = 1; s < cur.size(); ++s) {
                    auto &opt = find_short(cur[s]);
                    if (&opt != &nooption)
                        handle(opt);
                }

                unparsed_.erase(unparsed_.begin());
            }
        }
        else
            break;
    }

    // positional arguments...
}