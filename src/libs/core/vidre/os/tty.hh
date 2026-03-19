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
/// @file
#ifndef VIDRE_CORE_OS_TTY_HH
#define VIDRE_CORE_OS_TTY_HH
#pragma once

/**@defgroup api-core-os-tty Terminal helpers
 * @ingroup api-core-os
 * @details
 * @fileinfo
 *
 * */

#include <vidre/core/config.hh>

/**@addtogroup api-core-os
 * */
///@{

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define VIDRE_OS_ISATTY ::_isatty
#define VIDRE_OS_FILENO ::_fileno
#else
#include <unistd.h>
/** POSIX @c isatty abstraction.
 * */
#define VIDRE_OS_ISATTY ::isatty
/** POSIX @c fileno abstraction.
 * */
#define VIDRE_OS_FILENO ::fileno
#endif

///@}

namespace vidre {

    /**@addtogroup api-core-os-tty
     * */
    ///@{

    /** Check, if a C-stream is a TTY.
     * */
    inline bool isatty(FILE *stream) noexcept {
        return VIDRE_OS_ISATTY(VIDRE_OS_FILENO(stream));
    }

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_OS_TTY_HH