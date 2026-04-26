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
#ifndef VIDRE_CORE_IO_HH
#define VIDRE_CORE_IO_HH
#pragma once

/**@addtogroup api-core-io
 * @brief Data packets and general I/O related functionality.
 * @details
 * @fileinfo
 *
 * The I/O abstraction focuses on providing mechanism to retrieve, convert and
 * store data, more specifically data streams of various types (such as video
 * and audio). For that it takes inspiration from streaming libraries, such as
 * gstreamer. That means, for example, that the concept of "source" and "sink" 
 * are used to represent data providers and consumers, respectively. The concept 
 * of "pipes" (or filters, respectively) is used to represent data transformers, 
 * such as decoders and encoders. The implementation, in combination with appropriate
 * synchronization primitives, also allows for concurrent data processing, 
 * passing either raw byte-sequences or data packets with metadata.
 *
 * @{
 * @namespace vidre::io             In-/output related functionality.
 * @defgroup api-core-io-types      Streaming fundamentals
 * @defgroup api-core-io-packet     Packet based I/O
 * @defgroup api-core-io-file       File and memory I/O
 * @defgroup api-core-io-details    Implementation details
 * @}
 * 
 * @todo memory mapped io
 * */

 /**@addtogroup api-core-io-details
  * @{
  * @namespace vidre::io::detail    Implementation details for I/O.
  * @}
  */

#include <vidre/core/config.hh>
#include <vidre/io/types.hh>
#include <vidre/io/file.hh>

#endif  // VIDRE_CORE_IO_HH
