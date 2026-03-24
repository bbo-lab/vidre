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
#include "settings.hh"
#include "log.hh"

#include <fstream>
#include <sstream>

using namespace vidre;

// --- node.hh ----------------------------------------------------------------

lua::type settings::node::get(const std::string &field) const {
    if (push() != lua::type::table) {
        script::g.pop();
        throw vidre::exception() << throw_message("requested field is not a table");
    }

    script::g.getfield(-1, field.c_str());
    script::g.remove(-2);
    return script::g.type(-1);
}

std::vector<std::string> settings::node::keys() const {
    if (type_ != lua::type::table)
        return {};

    std::vector<std::string> retval;
    push();

    auto tidx = script::g.absindex(-1);
    script::g.pushnil();
    while (script::g.next(tidx)) {
        if (script::g.type(-2) == lua::type::string)
            retval.push_back(script::g.tostring(-2));
        script::g.pop(1);
    }
    return retval;
}

// --- file.hh ----------------------------------------------------------------

void settings::file::load(const std::filesystem::path &fname, bool force) {
    log::trace tr;
    tr << "settings::file::load(" << std::string(fname) << "," << force << ")\n";

    // make absolute
    std::filesystem::path file = fname;
    if (not file.is_absolute()) {
        file = std::filesystem::current_path() / file;
    }

    // check if file exists
    if (not std::filesystem::exists(file)) {
        throw vidre::exception() << throw_message("file does not exist");
    }

    // check if already loaded
    script::stack scope(script::g);
    script::g.newmetatable("vidre:config");
    script::g.getfield(-1, std::string(file).c_str());
    if (not script::g.isnil(-1)) {
        ref_ = script::g.tointeger(-1);
        if (not force) {
            tr << "-> already loaded\n";
            return;
        }
        else {
            tr << "- going to reload\n";
        }
    }
    script::g.pop(1);

    // load file
    std::string config;
    {
        std::ifstream thefile{std::string(file)};
        std::stringstream tmpbuf;
        tmpbuf << thefile.rdbuf();
        config = tmpbuf.str();
    }

    // assume, it's a config file, and "return" is missing
    auto top = scope.size();
    {
        std::string buffer = "return " + config;
        auto s = script::g.loadbuffer(buffer.data(), buffer.size(), std::string(file).c_str());
        if (s == lua::status::ok)
            s = script::g.pcall(0, 1, 0);
        switch (s) {
            case lua::status::ok:
                break;
            case lua::status::yield:
                // FIXME what to do?
                throw exception() << throw_message("execution yielded");
            default:
                break;
        }

        tr << "- loaded with return, status: " << int(s) << "\n";
    }

    if (top == scope.size()) {
        // possibly a complex file, need to load it with prefixing it with "return"
        auto s = script::g.loadbuffer(config.data(), config.size(), std::string(file).c_str());
        if (s == lua::status::ok)
            s = script::g.pcall(0, 1, 0);
        switch (s) {
            case lua::status::ok:
                break;
            case lua::status::yield:
                // FIXME what to do?
                throw exception() << throw_message("execution yielded");
            default:
                // FIXME debug
                throw exception() << throw_message("failed");
        }

        tr << "- loaded plain, status: " << int(s) << "\n";
    }

    if (top == scope.size())
        // FIXME what to do, if empty?
        script::g.newtable();

    if (script::g.type(-1) != lua::type::table) {
        log::error() << "While loading configuration from\n    " << std::string(file) << "\na value of type '"
                     << script::g.type_name(script::g.type(-1)) << "' was returned.\n";
        script::g.checktype(-1, lua::type::table);
    }

    // register
    if (not isnil()) {
        // FIXME requires metamethods to trigger reload of sub-nodes
        //       that also got a reference
        throw vidre::exception() << throw_message("FIXME: not implemented");
    }
    node::ref();
    script::g.pushinteger(ref_);
    script::g.setfield(-2, std::string(file).c_str());

    tr << "-> acquired reference " << ref_ << "\n";
}

std::string longquote(const std::string &s) {
    for (int i = 0; i < 16; ++i) {
        std::string q = "[" + std::string(i, '=') + "[";

        if (s.find(q) == std::string::npos) {
            return q + s + "]" + std::string(i, '=') + "]";
        }
    }

    throw vidre::exception() << throw_message("escaping configuration string failed");
}

std::string quote(const settings::node &t) {
    std::string retval = t;

    if (retval.find('\n') != std::string::npos) {
        retval = longquote("\n" + retval);
    }
    else {
        bool has_single = retval.find('\'') != std::string::npos;
        bool has_double = retval.find('"') != std::string::npos;

        if (has_single or has_double)
            retval = longquote(retval);
        else if (has_double)
            retval = '\'' + retval + '\'';
        else
            retval = '"' + retval + '"';
    }

    return retval;
}

void dump(std::ofstream &f, const settings::node &t, int indent) {
    switch (t.type()) {
        case lua::type::table: {
            auto keylist = t.keys();
            if (indent == 0)
                f << std::string(4 * indent, ' ');

            if (keylist.empty()) {
                f << "{}";
            }
            else {
                f << "{" << std::endl;
                if (indent == 0)
                    indent++;

                std::sort(keylist.begin(), keylist.end());

                for (size_t i = 0; i < keylist.size(); ++i) {
                    const std::string &key = keylist[i];
                    const settings::node child = t[key];

                    for (int i = 0; i < indent; ++i)
                        f << "    ";
                    f << key << " = ";

                    dump(f, child, indent + 1);

                    if (i + 1 < keylist.size())
                        f << "," << std::endl;
                    else
                        f << std::endl;
                }

                indent--;
                f << std::string(4 * indent, ' ') << "}";
            }
            break;
        }
        case lua::type::number:
            f << lua::number(t);
            break;
        case lua::type::string:
            f << quote(t);
            break;
        default:
            throw exception();  // FIXME
    }
}

void settings::file::save(const std::filesystem::path &file) {
    std::ofstream out(file);
    dump(out, *this, 0);
    out << std::endl;
}

void settings::file::save() {
    // find ref_ in vidre::config
    script::stack scope(script::g);
    script::g.newmetatable("vidre:config");

    script::g.pushnil();
    while (script::g.next(-2)) {
        if (ref_ == script::g.tointeger(-1)) {
            save(script::g.tostring(-2));
            return;
        }
    }
}