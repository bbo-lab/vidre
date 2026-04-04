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
#include "exception.hh"

#include "util/filesystem.hh"

using namespace vidre;

// --- node.hh ----------------------------------------------------------------

lua::type settings::node::get(const std::string &field) const {
    using namespace vidre::script;

    if (push() != lua::type::table) {
        g.pop();
        throw exception::logic() << error::message("requested field is not a table");
    }

    g.getfield(-1, field.c_str());
    g.remove(-2);
    return g.type(-1);
}

std::vector<std::string> settings::node::keys() const {
    using namespace vidre::script;

    if (type_ != lua::type::table)
        return {};

    std::vector<std::string> retval;
    push();

    auto tidx = g.absindex(-1);
    g.pushnil();
    while (g.next(tidx)) {
        if (g.type(-2) == lua::type::string)
            retval.push_back(g.tostring(-2));
        g.pop(1);
    }
    return retval;
}

// --- file.hh ----------------------------------------------------------------

void settings::file::load(const std::filesystem::path &fname, bool force) {
    using namespace vidre::script;

    log::trace tr;
    tr << "settings::file::load(" << std::string(fname) << "," << force << ")\n";

    // make absolute
    std::filesystem::path file = fname;
    if (not file.is_absolute()) {
        file = std::filesystem::current_path() / file;
    }

    // check if file exists
    if (not std::filesystem::exists(file)) {
        throw exception::logic() << error::message("file does not exist");
    }

    // check if already loaded
    engine::stack scope(g);
    g.newmetatable("vidre:config");
    g.getfield(-1, std::string(file).c_str());
    if (not g.isnil(-1)) {
        ref_ = g.tointeger(-1);
        if (not force) {
            tr << "-> already loaded\n";
            return;
        }
        else {
            tr << "- going to reload\n";
        }
    }
    g.pop(1);

    // load file
    std::string config = loadfile(file);

    // assume, it's a config file, and "return" is missing
    auto top = scope.size();
    {
        std::string buffer = "return " + config;
        auto s = g.loadbuffer(buffer.data(), buffer.size(), std::string(file).c_str());
        if (s == lua::status::ok)
            s = g.pcall(0, 1, 0);
        switch (s) {
            case lua::status::ok:
                break;
            case lua::status::yield:
                // should not happen
                throw exception::runtime() << error::message("execution yielded");
            default:
                break;
        }

        tr << "- loaded with return, status: " << int(s) << "\n";
    }

    if (top == scope.size()) {
        // possibly a complex file, need to load it with prefixing it with "return"
        auto s = g.loadbuffer(config.data(), config.size(), std::string(file).c_str());
        if (s == lua::status::ok)
            s = g.pcall(0, 1, 0);
        switch (s) {
            case lua::status::ok:
                break;
            case lua::status::yield:
                // should not happen
                throw exception::runtime() << error::message("execution yielded");
            default:
                // FIXME we need inspection here...
                throw vidre::exception() << error::message("failed");
        }

        tr << "- loaded plain, status: " << int(s) << "\n";
    }

    if (top == scope.size()) {
        throw exception::runtime() << error::message("file is neither a plain nor a complex configuration file");
    }

    if (g.type(-1) != lua::type::table) {
        log::error() << "While loading configuration from\n    " << std::string(file) << "\na value of type '"
                     << g.type_name(g.type(-1)) << "' was returned.\n";
        g.checktype(-1, lua::type::table);
    }

    // register
    if (not isnil()) {
        // FIXME requires metamethods to trigger reload of sub-nodes
        //       that also got a reference
        throw exception::logic() << error::message("FIXME: not implemented");
    }
    node::ref();
    g.pushinteger(ref_);
    g.setfield(-2, std::string(file).c_str());

    tr << "-> acquired reference " << ref_ << "\n";
}

std::string longquote(const std::string &s) {
    for (int i = 0; i < 16; ++i) {
        std::string q = "[" + std::string(i, '=') + "[";

        if (s.find(q) == std::string::npos) {
            return q + s + "]" + std::string(i, '=') + "]";
        }
    }

    throw exception::logic() << error::message("escaping configuration string failed");
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

void dump(FILE *f, const settings::node &t, int indent) {
    switch (t.type()) {
        case lua::type::table: {
            auto keylist = t.keys();
            auto indent_s = std::string(4 * indent, ' ');

            if (indent == 0)
                fwrite(indent_s.data(), indent_s.size(), 1, f);

            if (keylist.empty()) {
                fprintf(f, "{}");
            }
            else {
                fprintf(f, "{\n");
                if (indent == 0)
                    indent++;

                std::sort(keylist.begin(), keylist.end());

                for (size_t i = 0; i < keylist.size(); ++i) {
                    const std::string &key = keylist[i];
                    const settings::node child = t[key];

                    for (int i = 0; i < indent; ++i)
                        fprintf(f, "    ");
                    fprintf(f, "%s = ", key.c_str());

                    dump(f, child, indent + 1);

                    if (i + 1 < keylist.size())
                        fprintf(f, ",\n");
                    else
                        fprintf(f, "\n");
                }

                indent--;
                fwrite(indent_s.data(), indent_s.size(), 1, f);
                fprintf(f, "}");
            }
            break;
        }
        case lua::type::number:
            fprintf(f, "%g", lua::number(t));
            break;
        case lua::type::string: {
            auto q = quote(t);
            fwrite(q.data(), q.size(), 1, f);
            break;
        }
        default:
            throw exception();  // FIXME
    }
}

void settings::file::save(const std::filesystem::path &file) {
    FILE *out = fopen(file.c_str(), "r");
    if (not out)
        throw exception::runtime() << error::system() << error::location(file.c_str());

    dump(out, *this, 0);

    fprintf(out, "\n");
    fclose(out);
}

void settings::file::save() {
    using namespace script;

    // find ref_ in vidre::config
    engine::stack scope(g);
    g.newmetatable("vidre:config");

    g.pushnil();
    while (g.next(-2)) {
        if (ref_ == g.tointeger(-1)) {
            save(g.tostring(-2));
            return;
        }
    }
}