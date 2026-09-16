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
#ifndef VIDRE_CORE_IO_PACKET_VALUE_HH
#define VIDRE_CORE_IO_PACKET_VALUE_HH
#pragma once

#include <vidre/core/config.hh>

            // /** Plain data types.
            //  * */
            // enum class plain : uint16_t {
            //     sequence = 0x0000,   ///< Grouped packet (see @ref FIXME).
            //                          // === 0x0001-0x000F reserved
            //     nil = 0x0010,        ///< Nil value (no data).
            //     boolean = 0x0011,    ///< Boolean value (true/false).
            //     integer = 0x0012,    ///< 64-bit signed integer.
            //     natural = 0x0013,    ///< 64-bit unsigned integer.
            //     number = 0x0014,     ///< 64-bit floating-point value (double).
            //     string = 0x0015,     ///< UTF-8 sequence (not! zero-terminated).
            //     binary = 0x0016,     ///< Binary data (see @ref FIXME).
            //     uuid = 0x0017,       ///< UUID (128-bit universally unique identifier).
            //                          // === 0x0018-0x001F reserved for future POD types.
            //     isodate = 0x0020,    ///< ISO 8601 date ("YYYYMMDDTHHMMSS.SSSZ\0\0\0").
            //     timestamp = 0x0021,  ///< Timestamp.
            //     localtime = 0x0022,  ///< Local time (timestamp with timezone information).
            // };


#endif  // VIDRE_CORE_IO_PACKET_VALUE_HH
