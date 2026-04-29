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
#ifndef VIDRE_CORE_STORAGE_RAW_HH
#define VIDRE_CORE_STORAGE_RAW_HH
#pragma once

/**@addtogroup api-core-storage-raw
 * @details
 * @fileinfo
 * 
 * The raw file format uses [Type-Length-Value](https://en.wikipedia.org/wiki/Type%E2%80%93length%E2%80%93value)
 * (TLV) encoding, inspired by the design principles of ASN.1, which is also
 * used internally for packet transmissions. 
 *
 * The primary goals are:
 *
 *   - Minimal computational overhead during recording
 *   - Self-describing structure (cf. BER encoding, [X.690](https://en.wikipedia.org/wiki/X.690))
 *   - Storage size is intentionally traded off for speed
 *
 * The raw file format is designed to be flexible and extensible, allowing for
 * the inclusion of various types of data streams (e.g., video, audio, metadata)
 * while maintaining a consistent structure. The use of TLV encoding facilitates the
 * addition of new data types and structures without breaking compatibility.
 *
 * Yet, the raw file format is not intended to be a general-purpose container
 * format. It is specifically tailored for the needs of VidRe and may not be
 * suitable for all use cases. For example, it may not be the best choice for
 * long-term archival storage or for sharing data with external parties.
 *
 * Therefore, VidRe also provides tools for converting raw files to more widely
 * used formats (e.g., FFV1, MP4, AVI) for archival and sharing purposes. These tools
 * are designed to preserve the integrity of the original data while providing
 * compatibility with a broader range of software and platforms.
 *
 * **File layout**
 *
 * Each "raw" file consists of a preamble and a content header, followed by a 
 * sequence of data packets. 
 *
 * The preamble contains a magic number and version information, which are used 
 * to identify the file format and ensure compatibility:
 * @code{.unparsed}
 * <magic>              // 8 bytes
 * <version>            // 8 bytes (4 bytes major, 4 bytes minor)
 * @endcode
 * Note that the magic number is designed to be read and written as a 64-bit integer, 
 * which also provides information about the endianness of the file.
 * 
 * The content header following the preamble contains metadata about the data streams
 * recorded in the file, such as video frames, audio samples, and any associated metadata.
 * Each stream has it's own definition of what is contained in the content header:
 * @code{.unparsed}
 * <isodate>             // ISO 8601 date string (reference time and file creation time)
 * <timestamp>           // Associated reference timestamp
 * <stream header>*      // Definitions of the data streams (e.g., video, audio, metadata)
 * @endcode
 * 
 * @{
 * @namespace vidre::storage::raw
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/util/version.hh>
#include <vidre/util/chrono.hh>
#include <vidre/io/file.hh>

#include <bit>
#include <optional>
#include <cassert>
#include <cstring>

namespace vidre {

    namespace storage {

        namespace raw {

            /**@addtogroup api-core-storage-raw */
            ///@{

            /** Data and packet alignment.
             * @internal
             * To ensure efficient access on 64-bit architectures, all data is 
             * aligned to 8 bytes. This means that the size of the header and 
             * each chunk is a multiple of 8 bytes, and any padding is added 
             * as necessary.
             * @endinternal
             * */
            static constexpr std::size_t alignment = 8U;

            /** File magic number.
             * The magic number is expected to be written as byte sequence, therefore,
             * also giving information about the endianness of the file.
             * */
            static constexpr uint64_t magic = 0x4F424265'52646956;  ///< File magic ("VidReBBO").

            /** Version layout.
             * */
            struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY version_type {
                uint32_t major{};
                uint32_t minor{};
            };

            /** Current version.
             *
             * Versioning scheme:
             * The **major** number will only change if an incompatible change
             * occurs in the general format (also including type IDs). The
             * **minor** number increases when additional functionality is
             * added.
             *
             * */
            static constexpr version_type version = {1, 0};



            ///@}

        }  // namespace raw

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_RAW_HH
