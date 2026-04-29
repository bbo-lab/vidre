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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_CONFIG_HH
#define VIDRE_CORE_CONFIG_HH
#pragma once

#include <vidre/config.hh>

#include <cstdint>

namespace vidre {
    namespace detail {}
}  // namespace vidre

/**@addtogroup api-core-details
 * */
///@{

/** Declaration specification for import/export for the core library.
 *
 * This macro is needed for compiling on Windows®.
 *
 * Usage:
 * @code
 * class VIDRE_CORE_VISIBILITY MyClass {
 * public:
 *     MyClass() = default;
 * };
 * @endcode
 *
 * @see
 *      [dllexport,dllimport](https://learn.microsoft.com/en-us/cpp/cpp/dllexport-dllimport?view=msvc-170)
 * */
#if not defined(VIDRE_CORE_VISIBILITY) or defined(DOXYGEN)
#define VIDRE_CORE_VISIBILITY VIDRE_API_IMPORT
#endif

/** Standard packing attribute.
 * @note At the moment of writing, the VidRe software is expected to be
 *       compiled with GNU C/Clang compatible compilers, that understand
 *       this attribute syntax.
 * */
#define __vidre_packed__(ALIGN) __attribute__((packed, aligned(ALIGN)))

/** No-inline attribute.
 * */
#define __vidre_noinline__ __attribute__((noinline))

#if defined(DOXYGEN)

/** Host is a Linux system */
#define VIDRE_HOST_LINUX <0/1>

/** Host is a Windows system */
#define VIDRE_HOST_WINDOWS <0/1>

/** Host is an Apple system */
#define VIDRE_HOST_APPLE <0/1>

#endif  // DOXYGEN

/** Host is big-endian
 * @internal
 * The header <bit> needs to be included for this macro to work.
 * @endinternal
 * */
#define VIDRE_IS_BIGENDIAN (std::endian::native == std::endian::big)

/** Host is little-endian
 * @internal
 * The header <bit> needs to be included for this macro to work.
 * @endinternal
 * */
#define VIDRE_IS_LITTLEENDIAN (std::endian::native == std::endian::little)

#if defined(DOXYGEN)
#define VIDRE_HOST_LINUX <1/0>
#define VIDRE_HOST_WINDOWS <1/0>
#define VIDRE_HOST_APPLE <1/0>
#endif

///@cond
#if defined(linux) || defined(__linux__) || defined(__linux)
#define VIDRE_HOST_LINUX 1
#elif defined(_WIN32) || defined(_WIN64)
#define VIDRE_HOST_WINDOWS 1
#elif defined(__APPLE__) || defined(__MACH__)
#define VIDRE_HOST_APPLE 1
#else
#error "Unsupported host platform"
#endif

#if not defined(VIDRE_HOST_WINDOWS)
#define VIDRE_HOST_WINDOWS 0
#endif

#if not defined(VIDRE_HOST_APPLE)
#define VIDRE_HOST_APPLE 0
#endif
///@endcond

/** Check bit-depth of host.
 * */
#define VIDRE_HOST_BITS(N) (__SIZEOF_POINTER__ * 8 == (N))

///@}

#endif  // VIDRE_CORE_CONFIG_HH
