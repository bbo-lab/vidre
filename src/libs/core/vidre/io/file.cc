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
#include "vidre/io/types.hh"

using namespace vidre;

io::offset_type io::file::sink::flush() {
    if (buf_.used) {
        auto retval = fd_.write({buf_.used, buf_.data});

        if (retval != static_cast<io::offset_type>(buf_.used))
            // if the underlying hardware can't take it, we assume,
            // it's out of memory...
            throw exception::memory();
    }
    return buf_.flush();
}

io::offset_type io::file::sink::write(const memory::view &buf) noexcept {
    if (buf_) {
        // using buffer
        size_type count = buf.size();
        memory::const_pointer src = buf.data();

        offset_type retval = count;
        offset_type pos = 0;
        while (count > 0) {
            size_type buffered = buf_.put(src + pos, count);
            if (buf_.full())
                flush();
            count -= buffered;
            pos += buffered;
        }

        return retval;
    }
    else
        // direct write
        return fd_.write(buf);
}

bool io::file::sink::open(const std::filesystem::path &filename, openmode_type mode) noexcept {
    file::access acc = file::access::writeonly;
    file::disposition disp = file::disposition::create;
    file::hint hint = file::hint::none;

    if (mode & openmode_type::truncate)
        disp |= file::disposition::truncate;
    else
        disp |= file::disposition::append;

    if (mode & openmode_type::direct)
        hint |= file::hint::direct | file::hint::synchronous;

    bool retval = fd_.open(filename, acc, disp, hint);
    if (not retval)
        return false;

    if ((mode & openmode_type::direct) and not(mode & openmode_type::unbuffered)) {
        auto mem = std::max(fd_.blocksize(), os::pagesize());
        buf_.setup(mem);
    }

    return true;
}

void io::file::sink::close() noexcept {
    if (buf_)
        flush();
    fd_.close();
    return;
}

bool io::file::source::step(io::sink &snk) {
    auto res = get(buf_);

    if (res.count)
        snk.put({res.count, buf_.data});
    else
        // end of file
        return false;

    if (not res)
        // and error occurred
        throw exception::runtime() << error::code(fd_.error());

    return true;
}

io::file vidre::io::standard::input = io::file{::stdin};

io::file vidre::io::standard::output = io::file{::stdout};

io::file vidre::io::standard::error = io::file{::stderr};
