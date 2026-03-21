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

void vidre_lua_init(script::impl &g) {
    if (develop::inbuild()) {
        g.addpath(PROJECT_SOURCE_DIR "/src/scripts");
        g.addcpath(PROJECT_BINARY_DIR "/plugins");
    }

    // FIXME
    //g.require("vidre.init");
}

vidre::script::impl vidre::script::g(vidre_lua_init);

void script::impl::addpath(const std::filesystem::path &path) {
    stack top(*this);

    if (getglobal("package") != lua::type::table)
        throw exception() << throw_message("package table not accessible");

    if (getfield(-1, "path") != lua::type::string)
        throw exception() << throw_message("package.path not accessible");

    std::string search = tostring(-1);
    pop();

    search += LUA_PATH_SEP + std::string(path / LUA_PATH_MARK ".lua");
    search += LUA_PATH_SEP + std::string(path / LUA_PATH_MARK / "init.lua");

    pushlstring(search.data(), search.size());
    setfield(-2, "path");
}

void script::impl::addcpath(const std::filesystem::path &path) {
    stack top(*this);

    if (getglobal("package") != lua::type::table)
        throw exception() << throw_message("package table not accessible");

    if (getfield(-1, "cpath") != lua::type::string)
        throw exception() << throw_message("package.path not accessible");

    std::string search = tostring(-1);
    pop();

    search += LUA_PATH_SEP + std::string(path / LUA_PATH_MARK VIDRE_DLL_EXTENSION);
    search += LUA_PATH_SEP + std::string(path / LUA_PATH_MARK / "loadall" VIDRE_DLL_EXTENSION);

    pushlstring(search.data(), search.size());
    setfield(-2, "cpath");
}

void script::impl::require(const std::string &lib) {
    getglobal("require");
    pushlstring(lib.data(),lib.size());
    call(1);
}

void script::impl::run(const std::string &scr) {
    auto res = dostring(scr.c_str());
    switch (res) {
        case lua::status::ok:
            return;
        default:
            // FIXME
            throw vidre::exception() << throw_message("FIXME");
    }
}