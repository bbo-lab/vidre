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
#include "core.hh"
#include <thread>
#include "vidre/memory/base.hh"

using namespace vidre;
using namespace vidre::io;

void cache::allocate(size_type size, size_type n) {
    if (in_ != out_)
        throw exception::runtime() << error::message("Buffer is not empty during setup.");

    if (data_)
        std::free(data_);

    slots_ = std::bit_ceil(n);
    size_ = std::bit_ceil(size);

    data_ = reinterpret_cast<pointer>(std::aligned_alloc(8, size_ * slots_));
}

cache::size_type cache::put(const memory::view &v, std::chrono::milliseconds timeout) {
    size_type mask = (slots_ * size_) - 1;

    memory::const_pointer eptr = v.data() + v.size();  // pointer to end of buffer
    memory::size_type n = v.size();                    // bytes left to copy

    auto now = chrono::timer::clock::now();  // used for timeout
                                             // (we boldly assume the copying itself is very fast)

    // The cache behaves like a ring buffer, and, on very full slot, the
    // reader has to be notified.
    while (n > 0) {
        size_type slotnext = (in_ & ~(size_ - 1)) + size_;  // start of next slot
        size_type slotavail = slotnext - in_;               // number of bytes available for filling the slot

        // copy only as much as to fill one slot
        bool slotfull = false;
        size_type copy = n;
        if ((in_ + copy) >= slotnext) {
            // we reach the slot limit, need to check if cache is full
            while ((slotnext & mask) == out_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if ((chrono::timer::clock::now() - now) > timeout)
                    goto timed_out;  // timeout expired
            }

            copy = slotavail;
            slotfull = true;
        }

        // copy data, update positions
        std::memcpy(data_ + in_, eptr - n, copy);
        in_ = (in_ + copy) & mask;
        n -= copy;

        // notify "slot full"
        if (slotfull) {
            event_.notify();
        }
    }

timed_out:
    return v.size() - n;
}

