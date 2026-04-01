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
#ifndef VIDRE_CORE_STORAGE_HH
#define VIDRE_CORE_STORAGE_HH
#pragma once

/**@defgroup api-storage-ccv CCV
 * @ingroup api-storage
 * @details
 * @fileinfo
 *
 * The CCV file format was developed by David S. Greenberg¹ during his
 * time at BBO (MPI f. Neurobiol. of Behavior — caesar, Bonn, Germany).
 * 
 * **General structure**:
 * 
 *      file-header
 *      (
 *          image-data         uint8_t[+]                                   0.13
 *          image-padding      uint8_t[*]                                   0.20
 *          tail-information                                                0.13
 *      )*
 * 
 * The <tt>file-header</tt>:
 * 
 *      header-size             uint32_t                                    0.12
 *      camera-type             lv-string                                   0.12 (e.g. "Basler\0", or "Aptina\0") 
 *                                                                          0.20 (this should be "global" or "rolling")
 *      version                 double                                      0.12
 *      image-type              lv-string                                   0.12 (e.g. "Mono8") 
 *      bytes-per-pixel         uint32_t                                    0.12
 *      bits-per-pixel          uint32_t                                    0.12
 *      frame-bytes-on-disk     uint32_t                                    0.12
 *      w,h                     uint32_t[2]                                 0.12
 *      frame-rate              double                                      0.12
 *      bits-are-packed         uint8_t                                     0.13 (boolean) 
 *      frame-count             uint32_t                                    0.12
 *      offset                  uint32_t[2]                                 0.13
 *      sensor-size             uint32_t[2]                                 0.13
 *      clock-speed             uint64_t                                    0.13
 *      exposure-time-ms        double                                      0.13
 *      gain                    double                                      0.13
 *      endianness              uint8_t                                     0.20
 *      endianness-padding      uint8_t[3]                                  0.20
 *      system-identification   lv-string                                   0.20
 * 
 * with <tt>lv-string</tt>
 * 
 *      (
 *          string-length           uint8_t                                 0.13
 *          zero-terminated-string  uint8_t[]                               0.13
 *      )
 * 
 * replaced in version 0.20 with:
 * 
 *      (
 *          field-length            uint8_t                                 0.20
 *          zero-terminated-string  uint8_t[]                               0.20
 *      )
 * 
 * The <tt>tail-information</tt> depends on camera type:
 * 
 * @c Aptina:
 *      
 *      frame-index             uint32_t                                    0.13
 *      computer-time           double                                      0.13
 * 
 * @c Basler:
 *      
 *      frame-index             uint32_t                                    0.13
 *      computer-time           double                                      0.13
 *      camera-time             uint64_t                                    0.13
 *      flag-data               uint8_t                                     0.13
 * 
 * @c general:
 * 
 *      frame-index             uint32_t                                    0.13
 *      computer-time           double                                      0.13
 *      device-timestamp        uint64_t                                    0.20
 *      flag-data               uint32_t                                    0.20
 * 
 * The <tt>frame-index</tt> can start at 0 or 1.
 * The <tt>computer-time</tt> in seconds is based on a performance counter (on Windows®: QueryPerformanceCounter()/GetComputerClockRate()).
 * The <tt>camera-time</tt> is a time-stamp, provided by the camera (available for "Basler" camera only, 0 for "Aptina" at the moment of writing).
 * The <tt>flag-data</tt> is a boolean field indicating if the image grab failed.
 * 
 * The <tt>image-data</tt> depends on <i>image-type</i> (usually Mono8 or Mono10). When <i>bits-are-packed</i>.
 * Example: Mono10 data may be received from the camera as 16-bit values, for storage this can be packed to 10-bit values.
 * 
 * __New in version 0.20__:
 * 
 * For the use with memory alignment sensitive architectures, fields are padded to 4 bytes (int32).
 * 
 * <tt>field-length </tt>: the field-length must be 4- or 8-byte aligned (see @c vidre::storage::alignment).
 * 
 * <tt>endianness</tt>: @c b for "big endian", "l" for little endian.
 * 
 * <tt>system-identification</tt>: 
 * 
 * @TBD{JS suggests, this is a JSON string containing at least}
 *
 *      { 
 *          {"plugin" : "full/plugin/name"},
 *          {"camera" : 
 *              { 
 *                  {"manufacturer", "Manufacturer"},
 *                  {"user-name" : "User-name if applicable"},
 *                  {"serial-number" : "91238740981723"}
 *              }
 *          }
 *      }
 *
 * @TBD{... but may well also contain all settings?}
 * 
 * <tt>image-padding</tt>: choosen, so the the tail-information starts at memory alignment boundary.
 * @note Assuming a cropped Mono10 video is recorded with dimension 133×133 pixels. For packing (<i>bits-are-packed</i>),
 *       this means that 28302.4 bytes are used. Therefore, 1.6 bytes must be padded (<i>image-paddeing</i>).
 * 
 * <tt>flag-data</tt>: 
 * 
 * @TBD{to be decided}
 * 
 * ¹ Helmholz Centre Hereon, Geesthacht, Germany
 * */

///@cond
#if not defined(VIDRE_CCV_VISIBILITY)
#define VIDRE_CCV_VISIBILITY VIDRE_API_IMPORT
#endif
///@endcond

#include <vidre/core/storage.hh>

namespace vidre {

    namespace storage {

        /**@addtogroup api-storage-ccv
         * */
        ///@{

        /** CCV file.
         * */
        class VIDRE_CCV_VISIBILITY ccv {
        public:
        };

        ///@}

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_HH
