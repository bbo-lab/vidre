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
#ifndef VIDRE_CORE_IO_HH
#define VIDRE_CORE_IO_HH
#pragma once

/**@addtogroup api-core-io
 * @brief Chunk and packet based I/O functionality.
 * @details
 * @fileinfo
 *
 * The I/O abstraction provides mechanisms to acquire, transport, transform,
 * distribute, and store data, in particular continuous packetized data streams
 * such as video, audio, and sensor data (see @ref api-core-io-packet).
 *
 * Data flow is represented as a directed graph of interconnected endpoints.
 * Sources produce data, sinks consume data, and channels combine both roles to
 * transform or otherwise process data while forwarding it through the graph.
 * Fan-in and fan-out endpoints provide aggregation and distribution where the
 * one-to-one source/sink model is insufficient.
 *
 * The design is strongly influenced by GStreamer, in particular its element,
 * pad, capability, and push/pull data-flow concepts; see
 * [Pads and capabilities](https://gstreamer.freedesktop.org/documentation/application-development/basics/pads.html?gi-language=c).
 * VidRe uses its own considerably smaller abstraction tailored to acquisition
 * and recording systems, with an emphasis on data integrity rather than user
 * perception.
 *
 * In addition to packetized data streams, many I/O endpoints (see
 * @ref api-core-io-endpoint) provide a chunk-based interface for reading and
 * writing raw data. This interface is used, in particular, to store and retrieve
 * serialized packet streams.
 *
 * @{
 * @namespace vidre::io             In-/output related functionality.
 * @defgroup api-core-io-types      Basic types
 * @defgroup api-core-io-packet     Packet types
 * @defgroup api-core-io-endpoint   Endpoint types
 * @defgroup api-core-io-store      File and memory I/O
 * @defgroup api-core-io-monitor    Monitoring
 * @defgroup api-core-io-details    Implementation details
 * @}
 *
 * @todo 
 * - implement queue_depth handling (and rename method to "retention()")
 * - endpoint::stats needs some affection
 * - check isready() implementations: are all handling dir::any correctly?
 * - need to make sure, that get()/pull() always transmit valid packets
 *   (esp. queued packets must not just "disappear" into nirvana; may have a default "error" packet?)
 * - implement an io::catalogue, which could also handle stream IDs
 * - there was also the idea of "switchboard" for routing packets between endpoints.
 * */

 /**@addtogroup api-core-io-details
  * @{
  * @namespace vidre::io::detail    Implementation details for I/O.
  * @}
  * */

#include <vidre/core/config.hh>
#include <vidre/io/types.hh>
#include <vidre/io/endpoint.hh>
#include <vidre/io/packet.hh>
#include <vidre/io/file.hh>

#endif  // VIDRE_CORE_IO_HH
