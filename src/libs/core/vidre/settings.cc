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

#include <fstream>
#include <sstream>

using namespace vidre;

// --- node.hh ----------------------------------------------------------------

lua::type settings::node::get(const std::string &field) {
    if (push() != lua::type::table) {
        script::g.pop();
        throw vidre::exception() << throw_message("requested field is not a table");
    }

    script::g.getfield(-1, field.c_str());
    script::g.remove(-2);
    return script::g.type(-1);
}

// --- file.hh ----------------------------------------------------------------

void settings::file::load(const std::filesystem::path &file) {
    // FIXME make full path
    // FIXME check if file exists

    // check if already loaded
    script::stack scope(script::g);
    script::g.newmetatable("vidre:config");
    script::g.getfield(-1, std::string(file).c_str());
    if (not script::g.isnil(-1)) {
        ref_ = script::g.tointeger(-1);
        return;
    }
    script::g.pop(1);

    // load file 
    std::string buffer;
    {
        std::ifstream thefile{std::string(file)};
        std::stringstream tmpbuf;
        tmpbuf << thefile.rdbuf();
        buffer = tmpbuf.str();
    }

    auto top = scope.size();
    auto s = script::g.loadbuffer(buffer.data(), buffer.size(), std::string(file).c_str());
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

    if (top == scope.size()) {
        // load config file did not produce any result, we retry with "return ..."
        buffer = "return " + buffer;
        auto s = script::g.loadbuffer(buffer.data(), buffer.size(), std::string(file).c_str());
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
    }

    if (top == scope.size())
        // FIXME what to do, if empty?
        script::g.pushnil();

    // register
    script::g.ref(-1);
    script::g.pushinteger(ref_);
    script::g.setfield(-2, std::string(file).c_str());
}