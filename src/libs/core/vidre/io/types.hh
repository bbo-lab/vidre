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
#ifndef VIDRE_CORE_IO_TYPES_HH
#define VIDRE_CORE_IO_TYPES_HH
#pragma once

/**@addtogroup api-core-io-types
 * @brief Basic types and streaming fundamentals.
 * @details
 * @fileinfo
 *
 * The data flow in VidRe has to cover various forms of data (such as its main
 * focus: video images, see @ref api-video) as well as modes of transmission
 * (from fixed to variable sized data packets, as well as raw I/O).
 *
 * ## Overview
 *
 * FIXME
 * 
 * */

#include <vidre/core/config.hh>
#include <vidre/io/types/core.hh>
#include <vidre/io/types/endpoint.hh>
#include <vidre/io/types/sink.hh>
#include <vidre/io/types/source.hh>
#include <vidre/io/types/channel.hh>
// #include <vidre/io/types/arbiter.hh>
// #include <vidre/io/types/merger.hh>

#endif  // VIDRE_CORE_IO_TYPES_HH
