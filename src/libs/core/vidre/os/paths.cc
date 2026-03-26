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
#include "paths.hh"
#include "../log.hh"

using namespace vidre;

#if defined(_WIN32)
#include <windows.h>

static std::filesystem::path localappdata_dir() {
    std::filesystem::path dir;

    if (dir.empty()) {
        const char *p = getenv("LOCALAPPDATA");
        if (p)
            dir = p;
        else {
            p = getenv("APPDATA");
            if (p)
                dir = p;
            else
                dir = getenv("HOME");
        }
    }

    return dir;
}

static std::filesystem::path config_dir() {
    static std::filesystem::path dir;

    if (dir.empty()) {
        dir = localappdata_dir() / "vidre";

        FIXME ensure directory exists
    }

    return dir;
}

static std::filesystem::path tmp_dir() {
    static std::filesystem::path dir;

    if (dir.empty()) {
        dir = getenv("TEMP")
    }

    return dir;
}

static std::filesystem::path cache_dir() {
    static std::filesystem::path dir;

    if (dir.empty()) {
        dir = localappdata_dir() / "vidre" / "cache";

        FIXME ensure directory exists
    }

    return dir;
}

#elif defined(__linux__)
#include <cstdlib>

static std::filesystem::path config_dir() {
    static std::filesystem::path dir;

    if (dir.empty()) {
        const char *p = getenv("XDG_CONFIG_HOME");
        if (p) {
            if (p[0] == '~') {
                dir = getenv("HOME");
                dir /= p + 2;
            }
            else
                dir = p;
        }
        else {
            dir = getenv("HOME");
            dir /= ".config";
        }

        dir /= "vidre";

        if (not std::filesystem::is_directory(dir)) {
            if (not std::filesystem::create_directory(dir)) {
                log::error() << "Configuration directory\n\n    "  //
                             << dir.string()                       //
                             << "\n\ncould not be created.\n";
            }
        }
    }

    return dir;
}

static std::filesystem::path tmp_dir() {
    static std::filesystem::path dir;

    if (dir.empty()) {
        dir = "/tmp";

        if (not std::filesystem::is_directory(dir)) {
            log::error() << "Temporary directory\n\n"  //
                         << dir.string()               //
                         << "\n\ndoes not exist.\n";
        }
    }

    return dir;
}

static std::filesystem::path cache_dir() {
    static std::filesystem::path dir;

    if (dir.empty()) {
        const char *p = getenv("XDG_CACHE_HOME");
        if (p) {
            if (p[0] == '~') {
                dir = getenv("HOME");
                dir /= p + 2;
            }
            else
                dir = p;
        }
        else {
            dir = getenv("HOME");
            dir /= ".cache";
        }

        dir /= "vidre";

        if (not std::filesystem::is_directory(dir)) {
            if (not std::filesystem::create_directory(dir)) {
                log::error() << "Cache directory\n\n    "  //
                             << dir.string()               //
                             << "\n\ncould not be created.\n";
            }
        }
    }

    return dir;
}

#elif defined(__APPLE__)

static std::filesystem::path config_dir() {
    static std::filesystem::path dir;
    FIXME Library / Application Support return dir;
}

static std::filesystem::path tmp_dir() {
    static std::filesystem::path dir;
    FIXME also / tmp return dir;
}

static std::filesystem::path cache_dir() {
    static std::filesystem::path dir;
    FIXME ? ? ? return dir;
}

#endif

std::filesystem::path vidre::os::configdir() noexcept {
    return config_dir();
}

std::filesystem::path vidre::os::tmpdir() noexcept {
    return tmp_dir();
}

std::filesystem::path vidre::os::cachedir() noexcept {
    return cache_dir();
}

std::filesystem::path vidre::os::installdir() noexcept {
    static std::filesystem::path dir = VIDRE_INSTALL_PREFIX;
    return dir;
}