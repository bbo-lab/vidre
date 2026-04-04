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
#include <format>

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
    // if ((cptr < mem_) || ((cptr - mem_) >= used_) || (not used_))
    //     goto dealloc_failed;

    if (cptr < mem_)
        goto dealloc_failed;
    if ((cptr - mem_) >= used_)
        goto dealloc_failed;
    if (not used_)
        goto dealloc_failed;

    // {
    //     auto *p = reinterpret_cast<entry *>(mem_);
    //     for (int i = 0; p && (p->addr() < &mem_[used_]); ++i) {
    //         std::cout << std::format("{:3} {} {}\n", i, (const void *)p, p->size);

    //         p = p->next;
    //     }
    // }

    {
        // find entry
        auto *e = reinterpret_cast<entry *>(mem_);
        auto *beg = e;  // track first zero entry
        while (e->data() != ptr) {
            if (not e->next)
                break;
            if (e->size != 0)
                beg = e->next;
            e = e->next;
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

const char *exception::what() const noexcept {
    return "general exception";
}

const char *exception::logic::what() const noexcept {
    return "logic error";
}

const char *exception::runtime::what() const noexcept {
    return "runtime error";
}

const char *exception::argument::what() const noexcept {
    return "argument error";
}

error::string error::diagnostic::dump() noexcept {
    string msg(1023, ' ');
    msg.clear();

    msg = "An exception occured: ";
    msg += e_->what();
    msg += '\n';

    bool firstinfo = true;
    auto ptr = &(e_->next_);
    while (ptr->get() != nullptr) {
        auto skel = ptr->get();
        auto infop = static_cast<error::info *>(skel);
        if (infop == nullptr) {
            // FIXME must not happend
            break;
        }

        if (firstinfo) {
            msg += "\nDiagnostic information:\n\n";
            firstinfo = false;
        }

        infop->make(msg);
        msg += "\n";

        // next
        ptr = &(infop->next_);
    }

    return msg;
}

void error::location::make(error::string &msg) noexcept {
    if (not function_.empty() and not filename_.empty())
        msg += std::format(
            "    source: in {}\n"
            "               {}",
            function_,  //
            filename_);
    else
        msg += std::format("    source: in {}", filename_);

    if (line_ and column_)
        msg += std::format(" ({},{})", line_, column_);
    else if (line_)
        msg += std::format(" ({})", line_);

    msg += '\n';
}

void error::message::make(error::string &emsg) noexcept {
    emsg += "    message: ";
    emsg += msg_ + "\n";
}

void error::errc::make(error::string &emsg) noexcept {
    auto &cat = std::generic_category();

    emsg += "    ";
    emsg += cat.name();
    emsg += ": ";
    emsg += cat.message(static_cast<int>(ec_));
    emsg += "\n";
}

void error::system::make(error::string &emsg) noexcept {
    auto &cat = std::system_category();

    emsg += "    ";
    emsg += cat.name();
    emsg += ": ";
    emsg += cat.message(ec_);
    emsg += "\n";
}
