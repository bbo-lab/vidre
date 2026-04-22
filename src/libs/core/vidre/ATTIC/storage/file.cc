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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "file.hh"
#include "../os/memory.hh"

using namespace vidre;

storage::offset_type storage::file::sink::flush() {
    if (buf_.used) {
        auto retval = write(buf_.data, buf_.used);

        if (retval != buf_.used)
            // if the underlying hardware can't take it, we assume,
            // it's out of memory...
            throw exception::memory();
    }
    return buf_.flush();
}

bool storage::file::sink::open(const std::filesystem::path &filename, openmode_type mode, io::perms permissions) noexcept {
    io::flag_type f = io::writeonly | io::create | io::close_on_exec;

    if (mode & openmode_type::truncate)
        f |= io::truncate;
    else
        f |= io::append;

    if (mode & openmode_type::direct)
        f |= io::direct;

    bool retval = fd_.open(filename, f, permissions);
    if (not retval)
        return false;

    if ((mode & openmode_type::direct) and not(mode & openmode_type::unbuffered)) {
        auto mem = std::max(detail::file::blocksize(fd_), os::pagesize());
        buf_.setup(mem);
    }

    return true;
}

void storage::file::sink::close() noexcept {
    if (buf_)
        flush();
    fd_.close();
    return;
}

storage::offset_type storage::file::sink::put(size_type count, const uint8_t *src) {
    if (buf_) {
        // using buffer
        offset_type retval = count;
        offset_type pos = 0;
        while (count > 0) {
            size_type buffered = buf_.append(src + pos, count);
            if (buf_.full())
                flush();
            count -= buffered;
            pos += buffered;
        }

        return retval;
    }
    else
        // direct write
        return write(src, count);
}
