/* VidRe
 * Copyright (C) 2024-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#include "native.hh"
#include "../util/macros.hh"

#include <utility>
#include <system_error>

using namespace vidre;

#if defined(VIDRE_USING_DLFCN)
std::pair<plugin::detail::library::handle_type, std::string> plugin::detail::library::open(const char *filename,
                                                                                           int flags) noexcept {
    handle_type handle = dlopen(filename, flags);
    if (not handle)
        return std::make_pair(nullptr, dlerror());
    else
        return std::make_pair(handle, std::string{});
}

std::pair<bool, std::string> plugin::detail::library::close(handle_type handle) noexcept {
    if (handle) {
        if (dlclose(handle))
            return std::make_pair(false, dlerror());
    }
    return std::make_pair(true, std::string{});
}

std::pair<plugin::detail::library::symbol_type, std::string> plugin::detail::library::symbol(handle_type handle,
                                                                                             const char *symname) noexcept {
    // see man dlsym, NOTES
    dlerror();
    symbol_type symbol = dlsym(handle, symname);
    if (not symbol) {
        const char *emsg = dlerror();
        if (not emsg)
            emsg = "weak symbol access or nullptr access not possible";
        return std::make_pair(nullptr, emsg);
    }
    return std::make_pair(symbol, std::string{});
}

#elif defined(VIDRE_USING_LIBLOADERAPI)
using handle_type = HMODULE;

enum mode {
    lazy = DONT_RESOLVE_DLL_REFERENCES,
    now = 0
};

std::pair<handle_type, std::string> plugin::detail::library::open(const char *filename, mode flags) noexcept {
    handle_type handle = LoadLibraryExA(library_name.c_str(), NULL, DWORD(flags));
    if (not handle)
        return std::make_pair(nullptr, std::error_code(GetLastError(), std::system_category()).message().c_str());
    else
        return std::make_pair(handle, std::string{});
}

std::pair<bool, std::string> plugin::detail::library::close(handle_type handle) noexcept {
    if (handle_)
        FreeLibrary(handle_);
    return std::make_pair(true, std::string{});
}

std::pair<symbol_type, std::string> plugin::detail::library::symbol(handle_type handle, const char *symname) noexcept {
    auto sym = GetProcAddress(handle_, symname.c_str());
    if (not sym)
        return std::make_pair(nullptr, std::error_code(GetLastError(), std::system_category()).message().c_str());

    return std::make_pair(sym, std::string{});
}
#endif

void plugin::dynamic::load(const std::string &name) {
    // FIXME check if already loaded (this would require library to use shared_ptr for the handle...)

    // open module
    module_.open(name + extension, library::mode::lazy);
    if (not module_.valid()) {
        // FIXME error handling, user information, etc.
        return;
    }

    // check info
    try {
        info_ = module_.get<information *>(VIDRE_STRINGIFY_EXPAND(VIDRE_PLUGIN_INFO));
    } catch (...) {
        return;
    }

    // initialize
    info_->init();

    // FIXME register with handler
}
