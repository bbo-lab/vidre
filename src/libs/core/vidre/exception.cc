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
#include "exception.hh"
#include "storage/io.hh"

#include <iostream>

#include <cstdlib>
#include <cstring>

using namespace vidre;

thread_local size_t vidre::detail::exception::store::used_ = 0;
thread_local char vidre::detail::exception::store::mem_[VIDRE_EXCEPTION_STORE] = {};

detail::exception::store::pointer detail::exception::store::allocate(size_t n) noexcept {
    if ((VIDRE_EXCEPTION_STORE - used_) < (n + 2 * granularity))
        abort_with_message("implementation error: exception store depleted");

    auto ptr = &mem_[used_];
    used_ += n + sizeof(entry);

    // ensure alignment
    used_ = (used_ + granularity - 1) & ~(granularity - 1);

    // set "entry"
    auto *e = reinterpret_cast<entry *>(ptr);
    *e = {&mem_[used_], n};
    if ((e->next->data() - mem_) <= VIDRE_EXCEPTION_STORE)
        *(e->next) = {};

    return ptr + sizeof(entry);
}

void detail::exception::store::deallocate(pointer ptr) noexcept {
    const char *cptr = reinterpret_cast<const char *>(ptr);
    if ((cptr < mem_) || ((cptr - mem_) >= used_) || (not used_))
        goto dealloc_failed;

    {
        // find entry
        auto *e = reinterpret_cast<entry *>(mem_);
        auto *beg = e;  // track first zero entry
        while (e->data() != ptr) {
            if (not e->next)
                break;
            e = e->next;
            if (beg->size != 0)
                beg = e;
        }

        if (e->data() == ptr) {
            // find end of zero entries
            auto end = e->next;
            while ((end->next) && (end->size == 0))
                end = end->next;

            if (end->addr() >= &mem_[used_]) {
                // free from first zero
                *beg = {};
                used_ = static_cast<size_type>(reinterpret_cast<const char *>(beg) - mem_);
            }
            else
                // free at entry (fragmented)
                e->size = 0;

            return;
        }
    }

dealloc_failed:
    abort_with_message("implementation error: deallocating invalid pointer");
}

void vidre::abort_with_message(const char *msg) {
    detail::file::write(2, msg, strlen(msg));
    abort();
}

const char *exception::what() {
    return "general exception";
}

const char *exception::runtime::what() {
    return "runtime error";
}

const char *exception::logic::what() {
    return "logic error";
}

const char *error::location::message() noexcept {
    return "FIXME";
}

void error::diagnostic::show() noexcept {
    auto ptr = &(e_->next_);
    while (ptr->get() != nullptr) {
        auto skel = ptr->get();
        auto infop = static_cast<error::info *>(skel);
        if (infop == nullptr) {
            break;
        }

        std::cerr << "   [" << infop->tag() << "]" << std::endl;

        // next
        ptr = &(infop->next_);
    }
}
