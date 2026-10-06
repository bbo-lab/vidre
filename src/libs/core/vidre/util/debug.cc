/* VidRe
 * Copyright (C) 2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#include "debug.hh"

#include <cstdlib>
#include <cxxabi.h>

#if defined(DEBUG)
#include <cstring>

#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#endif

bool vidre::debug::attached() noexcept {
#if defined(__linux__)
    char buf[4096];
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd == -1)
        return false;
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n > 0) {
        buf[n] = '\0';

        constexpr char key[] = "TracerPid:";
        const char *p = strstr(buf, key);

        if (!p)
            return false;

        p += sizeof(key) - 1;

        while (*p == ' ' || *p == '\t')
            ++p;

        return *p >= '1' && *p <= '9';
    }
    return false;
#elif defined(_WIN32)
    return ::IsDebuggerPresent() != FALSE;
#elif defined(__APPLE__)
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};

    struct kinfo_proc info{};
    size_t size = sizeof(info);

    if (sysctl(mib, 4, &info, &size, nullptr, 0) != 0)
        return false;

    return (info.kp_proc.p_flag & P_TRACED) != 0;
#else
    return false;
#endif
}
#endif  // DEBUG

std::string vidre::debug::demangle(const char *name) {
#if defined(__GNUC__) || defined(__clang__)
    int status = 0;
    char *demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled != nullptr) ? demangled : name;
    std::free(demangled);
    return result;
#else
    return name;
#endif
}
