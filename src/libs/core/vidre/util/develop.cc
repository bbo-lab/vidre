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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "develop.hh"
#if VIDRE_DEVELOP

#include <string>

using namespace vidre;

// get_program_invocation_name as suggested by DeepSeek Chat AI (https://deepseek.com):
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <climits>
#endif

std::string get_program_invocation_name() {
#if defined(_WIN32)
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return path;
#elif defined(__linux__) || defined(__APPLE__)
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return path;
    }
    // Fallback if /proc/self/exe isn't available (e.g., some BSDs)
    return "";
#else
    // Unsupported platform
    return "";
#endif
}

static constexpr std::string_view vidre_build_dir = VIDRE_BINARY_DIR;

bool develop::inbuild() noexcept {
    return get_program_invocation_name().substr(0, vidre_build_dir.size()) == vidre_build_dir;
}

#endif
