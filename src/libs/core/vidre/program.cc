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
#include "program.hh"
#include "log.hh"

#include <iostream>

using namespace vidre;

detail::tinymap &detail::tinymap::set(const std::string &key, variant value) {
    for (auto &item : *this) {
        if (item.first == key) {
            item.second = value;
            return *this;
        }
    }

    emplace_back(key, value);

    return *this;
}

variant detail::tinymap::get(const std::string &key) {
    for (auto &item : *this) {
        if (item.first == key)
            return item.second;
    }
    throw vidre::exception() << throw_message("internal error: key lookup failed");
}

bool detail::tinymap::has(const std::string &key) {
    for (auto &item : *this) {
        if (item.first == key)
            return true;
    }
    return false;
}

static fs::path program_invocation_path{};

const fs::path &program::invocation() {
    if (program_invocation_path.empty())
        throw "internal error: function called before program arguments were parsed";
    return program_invocation_path;
}

const program::argument *program::argument::parser::find_short(char c) {
    for (size_t i = 0; i < opt_->size; ++i) {
        if (opt_->args[i].short_option == c)
            return &opt_->args[i];
    }
    return nullptr;
}

const program::argument *program::argument::parser::find_long(const std::string &name) {
    for (size_t i = 0; i < opt_->size; ++i) {
        if (opt_->args[i].long_option == name)
            return &opt_->args[i];
    }
    return nullptr;
}

void program::argument::parser::handle(const argument &opt) {
    std::string key = opt.long_option;

    // deal with the option
    switch (opt.type) {
        case argument::kind::flag: {
            long value = parsed_.get(key);
            parsed_.set(key, value + 1);
            break;
        }
        case argument::kind::positional:
            parsed_.set(opt.long_option, unparsed_[0]);
            break;
        default: {
            bool found = false;
            std::string value;

            auto eqpos = unparsed_[0].find('=');
            if (eqpos != std::string::npos) {
                found = true;
                value = unparsed_[0].substr(eqpos + 1);
            }

            if (opt.type == argument::kind::required) {
                if (not found) {
                    if ((unparsed_.size() < 2) or (unparsed_[1][0] == '-'))
                        argerror(std::format("Option '{}' requires an argument.", opt.long_option));

                    value = unparsed_[1];
                    unparsed_.erase(unparsed_.begin() + 1);
                }
            }
            else if (not found) {
                parsed_.set(key, opt.default_value);
                break;
            }

            switch (opt.value) {
                case variant::natural:
                    parsed_.set(key, std::stoul(value.c_str()));
                    break;
                case variant::integer:
                    parsed_.set(key, std::stol(value.c_str()));
                    break;
                case variant::number:
                    parsed_.set(key, std::stod(value.c_str()));
                    break;
                case variant::cstring:
                case variant::string:
                    parsed_.set(key, value);
                    break;
                default:
                    throw "internal error: should not be reached";
            }

            break;
        }
    }

    // execute callback, if applicable
    if (opt.callback)
        opt.callback(*this, parsed_.get(key));
}

void program::argument::parser::argerror [[noreturn]] (const std::string &msg) {
    std::cerr << "ERROR: " << msg << std::endl << std::endl;
    std::cerr << help();

    abort();
}

void program::argument::parser::parse(int argc, const char **argv) {
    bool haspositional = false;

    // === prepare "parsed"
    for (size_t i = 0; i < opt_->size; ++i) {
        auto &opt = opt_->args[i];

        if (opt.type == argument::kind::positional) {
            haspositional |= true;
            parsed_.set(opt.long_option, opt.default_value);
        }
        else {
            if (opt.type == argument::kind::flag)
                parsed_.set(opt.long_option, 0);
            else
                parsed_.set(opt.long_option, {});
        }
    }

    // === program invocation
    program_invocation_path = argv[0];

    // === put all arguments into "unparsed" list
    for (int i = 1; i < argc; ++i)
        unparsed_.push_back(argv[i]);

    // === loop over "unparsed"
    // non-positional arguments
    while (not unparsed_.empty()) {
        auto &cur = unparsed_[0];

        if (cur[0] == '-') {
            if (cur[1] == '-') {
                // check for "="
                std::string long_option = cur.substr(2);

                auto eqpos = cur.find('=');
                if (eqpos != std::string::npos)
                    long_option = cur.substr(2, eqpos - 2);

                if (cur[2] == 0) {
                    unparsed_.erase(unparsed_.begin());
                    break;
                }
                else {
                    const argument *found = find_long(long_option);

                    if (not found)
                        argerror(std::format("Argument '{}' is not recognized.", cur));

                    handle(*found);

                    unparsed_.erase(unparsed_.begin());
                    continue;
                }
            }
            else {
                for (size_t i = 1; i < cur.size(); ++i) {
                    const argument *found = find_short(cur[i]);

                    if (not found) {
                        if (cur.size() > 2)
                            argerror(std::format("In argument '{}', '-{}' is not recognized.", cur, cur[i]));
                        else
                            argerror(std::format("Argument '{}' is not recognized.", cur));
                    }

                    handle(*found);
                }

                unparsed_.erase(unparsed_.begin());
                continue;
            }
        }
        else
            break;
    }

    // positional arguments
    for (size_t i = 0; i < opt_->size; ++i) {
        if (unparsed_.empty())
            break;

        auto &opt = opt_->args[i];
        if (opt.type == argument::kind::positional) {
            handle(opt);
            unparsed_.erase(unparsed_.begin());
        }
    }

    // check for unset positional
    for (size_t i = 0; i < opt_->size; ++i) {
        auto &opt = opt_->args[i];
        if (opt.type == argument::kind::positional) {
            auto value = parsed_.get(opt.long_option);
            if (value.empty())
                argerror(std::format("Parameter '{}' is not set", opt.long_option));
        }
    }
}

std::string program::argument::parser::help() {
    // find text width of options
    size_t optwidth = 0;
    for (size_t i = 0; i < opt_->size; ++i) {
        auto &opt = opt_->args[i];

        auto len = strlen(opt.long_option);
        switch (opt.type) {
            case argument::kind::optional:
                len += 2;  // []
                [[fallthrough]];
            case argument::kind::required:
                len += 8;  // <value>
                [[fallthrough]];
            case argument::kind::flag:
                len += 2;  //--
                if (opt.short_option)
                    len += 3;  //,-
                [[fallthrough]];
            case argument::kind::positional:
                if (optwidth < len)
                    optwidth = len;
                break;
        }
    }

    // assemble help
    std::string retval = program_invocation_path.filename();

    retval += " - ";
    retval += opt_->brief;
    if (opt_->copyright) {
        retval += "\n";
        retval += opt_->copyright;
    }
    retval += "\n\n";

    retval += opt_->description;

    retval += "\n\nUsage:\n";

    retval += "\n    ";
    retval += program_invocation_path.filename();
    retval += " [options]";

    bool has_positional = false;
    for (size_t i = 0; i < opt_->size; ++i) {
        auto &opt = opt_->args[i];

        switch (opt.type) {
            case argument::kind::positional: {
                has_positional = true;
                bool hasdefault = not opt.default_value.empty();
                if (hasdefault)
                    retval += "=[<";
                else
                    retval += " <";

                retval += opt.long_option;

                if (hasdefault)
                    retval += ">]";
                else
                    retval += ">";
                break;
            }
            default:
                break;
        }
    }

    if (has_positional) {
        retval += "\n\n  with\n\n";

        for (size_t i = 0; i < opt_->size; ++i) {
            auto &opt = opt_->args[i];

            switch (opt.type) {
                case argument::kind::positional: {
                    retval += std::format("    {:<{}}  {}\n", opt.long_option, optwidth, opt.brief);
                    break;
                }
                default:
                    break;
            }
        }
    }

    retval += "\nOptions:\n";

    for (size_t i = 0; i < opt_->size; ++i) {
        auto &opt = opt_->args[i];

        if (opt.type == argument::kind::positional)
            continue;

        std::string opttext = "--";
        opttext += opt.long_option;
        if (opt.short_option) {
            opttext += ",-";
            opttext += opt.short_option;
        }

        switch (opt.type) {
            case argument::kind::required:
                opttext += " <value>";
                break;
            case argument::kind::optional:
                opttext += " [<value>]";
                break;
            default:
                break;
        }

        retval += std::format("    {:<{}}  {}\n", opttext, optwidth, opt.brief);
    }

    return retval;
}

void program::argument::parser::show() const noexcept {
    log::info info;
    for (auto &item : parsed_) {
        info << item.first << " = ";

        switch (item.second.type()) {
            case variant::none:
                info << "<none>";
                break;
            case variant::natural:
                info << (unsigned long)(item.second);
                break;
            case variant::integer:
                info << long(item.second);
                break;
            case variant::number:
                info << double(item.second);
                break;
            case variant::cstring:
                info << "\"" << (const char *)(item.second) << "\"";
                break;
            case variant::string:
                info << "\"" << std::string(item.second) << "\"";
                break;
            default:
                info << "???";
        }

        info << log::endl;
    }
}
