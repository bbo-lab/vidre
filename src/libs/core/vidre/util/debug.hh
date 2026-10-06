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
/// @file
#ifndef VIDRE_CORE_UTIL_DEBUG_HH
#define VIDRE_CORE_UTIL_DEBUG_HH
#pragma once

/**@defgroup api-core-util-debug Debug utilities
 * @ingroup api-core-util
 * @details
 * @fileinfo
 *
 * @{
 * @namespace vidre::debug Debug utilities.
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>

#include <string>
#include <format>

namespace vidre {

    namespace debug {

        /**@addtogroup api-core-util-debug
         * */
        ///@{

        /** Debugging helper to prevent the compiler from optimizing away a variable.
         * @tparam T Type of the variable.
         * @param val The variable to prevent optimization on.
         *
         * @note The effects of this function are system-dependent and primarily intended for debugging purposes.
         *       At the moment of writing, only GCC and Clang are supported.
         *
         * */
        template <typename T>
        void do_not_optimize(T const &val [[maybe_unused]]) {
#if defined(__GNUC__) || defined(__clang__)
            asm volatile("" : : "r,m"(val) : "memory");
#endif
        }

        /** Demangle a C++ type name.
         * @param name Mangled type name.
         * @returns Demangled type name as a string.
         *
         * @note This function is primarily intended for debugging purposes and may not be available on all platforms.
         * */
        std::string demangle(const char *name);

#if defined(DEBUG) or defined(DOXYGEN)
        /** Get a diagnostic object ID based on its memory address.
         *
         * @param ptr Pointer to the object.
         * @returns 32-bit unique identifier derived from the object's address.
         *
         * @note This function *only* generates a (not necessarily unique) identifier
         *       based on the memory address of the object if `DEBUG` is defined.
         *       Otherwise, it returns a default value (0).
         *
         * @see tag<>()
         * */
        inline std::uint32_t object_id(const void *ptr) noexcept {
            auto v = reinterpret_cast<std::uintptr_t>(ptr);

            if constexpr (sizeof(v) > sizeof(std::uint32_t))
                v ^= v >> 32;

            return static_cast<std::uint32_t>(v);
        }

        /** Get a tag string for an object based on its memory address.
         *
         * @tparam Name Fixed string representing the tag name.
         * @param ptr Pointer to the object.
         * @returns Formatted tag string combining the name and object ID.
         *
         * @note This function generates a tag string based on the memory address of the object if `DEBUG` is defined.
         *       Otherwise, it returns an empty string.
         * */
        template <class T>
        std::string tag(const T *obj) {
            return std::format("⟨{}@{:08x}⟩", demangle(typeid(T).name()), object_id(obj));
        }

        /** Check if we're attached to a debugger.
         * */
        bool attached() noexcept;
#else
        constexpr inline std::uint32_t object_id(const void *) noexcept {
            return {};
        }

        template <fixed_string>
        constexpr std::string tag(const void *) {
            return {};
        }

        constexpr inline bool attached() noexcept {
            return false;
        }
#endif

        ///@}

    }  // namespace debug

}  // namespace vidre

#endif  //  VIDRE_CORE_UTIL_DEBUG_HH
