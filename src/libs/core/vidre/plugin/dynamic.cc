/* VidRe
 * Copyright (C) 2024-2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#include "information.hh"
#include "dynamic.hh"
#include "../util/macros.hh"

using namespace vidre;

#if defined VIDRE_USING_DLFCN

plugin::dynamic::library::~library() {
    if (handle_)
        dlclose(handle_);
}

bool plugin::dynamic::library::open(const char *path, mode flags) noexcept {
    if (handle_)
        close();

    handle_ = dlopen(path, (int)flags);
    if (not handle_)
        emsg_ = dlerror();

    return handle_;
}

void plugin::dynamic::library::close() noexcept {
    if (handle_)
        dlclose(handle_);
    handle_ = nullptr;
}

plugin::dynamic::library::handle_type plugin::dynamic::library::get(const std::string &symname) noexcept {
    dlerror();  // clears old error
    auto sym = dlsym(handle_, symname.c_str());
    if (not sym) {
        emsg_ = dlerror();
        if (not emsg_)
            emsg_ = "Cannot handle a NULL result";  // see man dlsym, NOTES
    }
    return sym;
}

#elif defined(VIDRE_USING_LIBLOADERAPI)

FIXME

plugin::dynamic::library::library(const char *path, mode flags) {
    handle_ = LoadLibraryExA(library_name.c_str(), NULL, DWORD(flags));
    if (not handle_)
        // FIXME need to check if we actually got a static string...???
        emsg_ = std::error_code(GetLastError(), std::system_category()).message().c_str();
}

plugin::dynamic::library::~library() {
    if (handle_)
        FreeLibrary(handle_);
}

plugin::dynamic::library::handle_type plugin::dynamic::library::get(const std::string &symname) noexecpt {
    auto sym = GetProcAddress(handle_, symname.c_str());
    if (not sym)
        emsg_ = std::error_code(GetLastError(), std::system_category()).message().c_str();
    return sym;
}

#endif

void plugin::dynamic::load(const std::string &name) {
    // FIXME check if already loaded (this would require library to use shared_ptr for the handle...)

    // open module
    module_.open(name + extension, library::mode::lazy);
    if (not module_.isvalid()) {
        // FIXME error handling, user information, etc.
        return;
    }

    // check info
    try {
        info_ = module_.get<information *>(VIDRE_STRINGIFY_EXPAND(VIDRE_PLUGIN_INFO));
    }
    catch(...) {
        return;
    }

    // initialize
    info_->init();

    // FIXME register with handler
}