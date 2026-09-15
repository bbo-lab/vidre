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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "memory.hh"

using namespace vidre;

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/sysinfo.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif

#include <string_view>

vidre::os::memory vidre::os::memory::get() noexcept {
    vidre::os::memory info{};

#if defined(_WIN32)
    // FIXME
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        info.total = status.ullTotalPhys;
        info.available = status.ullAvailPhys;
        info.used = info.total - info.available;
    }
#elif defined(__linux__)
    FILE *memf = fopen("/proc/meminfo", "r");
    if (memf == nullptr) {
        auto total_pages = ::sysconf(_SC_PHYS_PAGES);
        auto avl_pages = ::sysconf(_SC_AVPHYS_PAGES);
        auto page_size = ::sysconf(_SC_PAGESIZE);

        info.total = page_size * total_pages;
        info.available = page_size * avl_pages;

        struct ::sysinfo sys_info;
        if (::sysinfo(&sys_info) == 0) {
            info.used = (sys_info.totalram - sys_info.freeram) * sys_info.mem_unit;
        }
    }
    else {
        // The following lines contain what we're after. The format itself 
        // hasn't changed since Linux kernel version 2.6 (according to DeepSeek),
        // only fields were added.

        // MemTotal:       63427612 kB
        // MemFree:        10586284 kB
        // MemAvailable:   45630752 kB

        char lbuf[256];
        while (not feof(memf)) {
            if (not fgets(lbuf, sizeof(lbuf), memf))
                break;

            auto n = strlen(lbuf);

            auto colon = std::find(lbuf, lbuf + n, ':');
            auto num = std::find_if(colon + 1, lbuf + n, isdigit);

            if (std::string_view{lbuf, colon} == "MemTotal")
                info.total = strtol(num, nullptr, 10) * 1024;
            else if (std::string_view{lbuf, colon} == "MemFree")
                info.used = strtol(num, nullptr, 10) * 1024;
            else if (std::string_view{lbuf, colon} == "MemAvailable")
                info.available = strtol(num, nullptr, 10) * 1024;

            if (info.total and info.available and info.used)
                break;
        }

        info.used = info.total - info.used;

        fclose(memf);
    }

#elif defined(__APPLE__)
    // Get total memory
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    size_t length = sizeof(info.total);
    sysctl(mib, 2, &info.total, &length, NULL, 0);

    // Get available memory
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics64_data_t vm_stats;

    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (host_page_size(mach_port, &page_size) == KERN_SUCCESS &&
        host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t)&vm_stats, &count) == KERN_SUCCESS)
    {
        info.available = (uint64_t)vm_stats.free_count * (uint64_t)page_size;
        info.used = ((uint64_t)vm_stats.active_count + (uint64_t)vm_stats.inactive_count + (uint64_t)vm_stats.wire_count) *
                    (uint64_t)page_size;
    }
    mach_port_deallocate(mach_task_self(), mach_port);
#endif

    return info;
}
