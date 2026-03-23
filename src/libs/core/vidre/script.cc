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
#include "script.hh"
#include "log.hh"
#include "exception.hh"
#include "util/develop.hh"

using namespace vidre;

void vidre_lua_init(script::interface &g) {
    // set up package.path and package.cpath
    // FIXME this should be done initially in cmake/vidre/deps/lua.cmake
    g.clearpath();
    g.addpath(VIDRE_INSTALL_SCRIPTDIR, "path");
    g.addpath(VIDRE_INSTALL_PLUGINDIR, "cpath");

    // developer help
    if (develop::inbuild()) {
        g.addpath(PROJECT_SOURCE_DIR "/src/scripts", "path");
        g.addpath(PROJECT_BINARY_DIR "/lib", "cpath");
    }

    // FIXME
    g.require("vidre");
}

vidre::script::interface vidre::script::g(vidre_lua_init);

std::string makepath(const std::string &path, int i) {
    switch (i) {
        case 'p':
            return path + "/" LUA_PATH_MARK ".lua" +  //
                   LUA_PATH_SEP + path + "/" LUA_PATH_MARK "/init.lua";
        case 'c':
            return path + "/" LUA_PATH_MARK VIDRE_DLL_EXTENSION +  //
                   LUA_PATH_SEP + path + "/" LUA_PATH_MARK "/loadall" VIDRE_DLL_EXTENSION;
        default:
            // must not happen
            throw exception();
    }
}

void script::interface::clearpackage(const std::string &field) {
    stack top(*this);

    if (getglobal("package") != lua::type::table)
        throw exception() << throw_message("package table not accessible");

    if (field == "path") {
    }
    else if (field == "cpath") {
    }
    else
        throw vidre::exception() << throw_message("trying to set unknown package field");

    pushlstring("", 0);
    setfield(-2, field.c_str());
}

void script::interface::addpackage(const std::string &path, const std::string &field) {
    stack top(*this);

    if (getglobal("package") != lua::type::table)
        throw exception() << throw_message("package table not accessible");

    if (getfield(-1, field.c_str()) != lua::type::string)
        throw exception() << throw_message("package.path not accessible");

    std::string search = tostring(-1);
    if (not search.empty())
        search += LUA_PATH_SEP;

    if (field == "path")
        search += makepath(path, 'p');
    else if (field == "cpath")
        search += makepath(path, 'c');
    else {
        throw vidre::exception() << throw_message("trying to add to unknown package field");
    }

    pushlstring(search.data(), search.size());
    setfield(-3, field.c_str());
}

void script::interface::require(const std::string &lib) {
    getglobal("require");
    pushlstring(lib.data(), lib.size());
    call(1);
}

lua::status script::interface::exec(const std::string &s) {
    lua::status res = loadstring(s.c_str());
    if (res == lua::status::ok)
        res = pcall(0, lua::multret, 0);
    switch (res) {
        case lua::status::ok:
        case lua::status::yield:
            return res;
        case lua::status::error_run:
            throw exception("runtime error");
        case lua::status::syntax:
            throw exception("syntax error");
        case lua::status::memory:
            throw exception("memory allocation error");
        case lua::status::error:
            throw exception("error handler failure");
        default:
            throw exception("unrecognized error return code");
    }
}

void script::exception::getinfo(script::interface &L) {
    // FIXME retrieve debug info, fill chunk_, line_ and column_
}