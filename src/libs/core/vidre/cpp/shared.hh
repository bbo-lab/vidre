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
///@file
#ifndef VIDRE_CORE_CPP_SHARED_HH
#define VIDRE_CORE_CPP_SHARED_HH
#pragma once

#include <vidre/config.hh>
#include <memory>

/**@defgroup api-core-cpp-shared Shared objects
 * @ingroup api-core-cpp
 * @details
 * @fileinfo
 *
 * Provides facilities for objects with a managed, shared lifetime.
 *
 * In applications with complex and potentially non-obvious data flows,
 * ownership and lifetime cannot always be tied to a single scope or
 * component. Shared objects remain alive as long as they are referenced
 * by any participating part of the application, avoiding dependencies on
 * the lifetime of the object that originally created them.
 *
 * `vidre::shared<T>` is an alias for `std::shared_ptr<T>`, and
 * `vidre::make_shared()` refers directly to `std::make_shared()`.
 *
 * Prefer shared objects over non-owning references whenever an object
 * must remain valid independently of the lifetime of its creator or
 * intermediary objects through which it is passed.
 *
 * @see [std::shared_ptr](https://en.cppreference.com/cpp/memory/shared_ptr)
 * @see [std::make_shared](https://en.cppreference.com/cpp/memory/shared_ptr/make_shared)
 *
 * */

namespace vidre {
    /**@addtogroup api-core-cpp-shared
     * */
    ///@{

    /** Shared object.
     *
     * Alias for `std::shared_ptr<T>`.
     *
     * @tparam T Type of the managed object.
     *
     * @see [std::shared_ptr](https://en.cppreference.com/cpp/memory/shared_ptr)
     * */
    template <typename T>
    using shared = std::shared_ptr<T>;

    /** Deleter for "foreign" objects.
     *
     * This is a no-op deleter intended for use with `std::shared_ptr` when managing
     * objects that should not be deleted by the `shared_ptr`. This is, for example,
     * the case when a device uses a ring buffer for data acquisition and passes
     * self-managed packet pointers to an endpoint.
     *
     * Example:
     * @code
     * shared<MyType> ptr(&myObject, foreign<MyType>{});
     * @endcode
     *
     * */
    template <class T>
    struct foreign {
        void operator()(T *) const noexcept {}
    };

    /** Create a managed object.
     *
     * Imports `std::make_shared()` into the `vidre` namespace.
     *
     * @see `shared<>`
     * @see [std::make_shared](https://en.cppreference.com/cpp/memory/shared_ptr/make_shared)
     * */
#if defined(DOXYGEN)
    template <class T, class... Args>
    shared<T> make_shared(Args &&...args);
#else
    using std::make_shared;
#endif

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_CPP_SHARED_HH
