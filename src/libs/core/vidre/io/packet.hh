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
#ifndef VIDRE_CORE_IO_PACKET_HH
#define VIDRE_CORE_IO_PACKET_HH
#pragma once

/**@addtogroup api-core-io-packet
 * @brief Packet-based I/O.
 * @details
 * @fileinfo
 *
 * Packet-based I/O is designed for handling variable-sized data units, such as video
 * frames or audio samples, which may not fit well into a continuous byte-stream model
 * (e.g., TCP or raw file I/O). It provides a structured way to read and write packets
 * of data, each with its own header and payload. This allows for more efficient
 * processing and better support for multiplexed data streams.
 *
 * Here, for packets, we adopt the [type-length-value](https://en.wikipedia.org/wiki/Type%E2%80%93length%E2%80%93value)
 * encoding, with some of the underlying logic inspired by [ASN.1](https://en.wikipedia.org/wiki/ASN.1):
 * - the type is subdivided into a
 *   * major (describing the overall use of the packet) and a
 *   * minor number (describing the structure or disposition of the major type),
 * - the length of the data packet, followed by
 * - the value, or content, respectively.
 *
 * A JSON document, might, for example, be represented by an `packet::major::object`,
 * consisting of a sequence of a key-value pairs, consisting of a `packet::plain::string`
 * and a value (such as a packet::plain::number).
 *
 * The framework implements structures preferrably in a way, that would allow to
 * store these byte-wise in a file (cf. `storage::raw::magic`) but also takes into
 * account the majorly dynamic nature of packets.
 *
 * @see @ref vidre::storage::raw
 * @{
 * @namespace vidre::io::packet Packet-based I/O types.
 * @}
 * */
#include <vidre/core/config.hh>
#include <vidre/io/packet/base.hh>
#include <vidre/io/packet/value.hh>
#include <vidre/io/packet/object.hh>

#endif  // VIDRE_CORE_IO_PACKET_HH
