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
 * The basic building blocks for data transmission are:
 * - `io::source`: a data producer (from input devices, such as video cameras, or data files).
 * - `io::sink`: a data consumer (from output devices, such as audio, video, or, as above, data files).
 * - `io::channel`: a channel exposing both, source and sink interface, allowing for, e.g., filtering data.
 * - `io::arbiter`: arbiters are responsible for distributing a single source to multiple sinks.
 * - `io::merger`: mergers are responsible for combining multiple sources into a single sink.
 *
 * Many of sources and sinks are related to actual devices (see @ref api-device), but may also be
 * software based components, such as the logging facility (see @ref api-core-log), or simply a
 * file or data buffer. Their bevhavior may also be very different concerning buffering. A video
 * device, for example, should buffer video frames of at least 1 second, but will drop frames
 * if the processing cannot keep up. A keyboard, on the other hand, must not drop key strokes—which
 * also, for example, applies to the logging facility. 
 *
 * FIXME introduce "stepping", "capabilities", "tasks", etc. 
 * 
 * @see @ref api-core-io-flow
 * */

#include <vidre/core/config.hh>
#include <vidre/io/types/basic.hh>
#include <vidre/io/types/sink.hh>
#include <vidre/io/types/source.hh>
#include <vidre/io/types/channel.hh>
#include <vidre/io/types/arbiter.hh>
#include <vidre/io/types/merger.hh>

#endif  // VIDRE_CORE_IO_TYPES_HH
