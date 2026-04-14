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
#ifndef VIDRE_CORE_VIDEO_PIXEL_HH
#define VIDRE_CORE_VIDEO_PIXEL_HH
#pragma once

/**@addtogroup api-core-video-pixel
 * @ingroup api-core-video
 * @details
 * @fileinfo
 * 
 * Encoding and decoding of pixel formats, and conversion between them.
 * 
 * @see FFmpeg's [ ixfmt.h](https://ffmpeg.org/doxygen/trunk/pixfmt_8h.html)
 * */
 
#include <vidre/core/config.hh>
#include <vidre/video/common.hh>
#include <vidre/storage.hh>
#include <vidre/exception.hh>

#include <string>
#include <string_view>
#include <bit>

#include <cstdint>
#include <cstddef>

#include <linux/videodev2.h>

namespace vidre {

    namespace video {

        /**@addtogroup api-core-video-pixel
         * */
        ///@{



// #define VIDRE_PIXEL_FORMAT(NUM, COLOR, PACKED, BITS) ((NUM << 8) | ((!!COLOR) << 7) | ((!!PACKED) << 6) | (BITS & 0x7F))

// #define VIDRE_PIXEL_FORMAT_NUMBER(fmt) ((fmt >> 8) & 0xFFFFFF)
// #define VIDRE_PIXEL_FORMAT_COLOR(fmt)  ((fmt >> 7) & 0x1)
// #define VIDRE_PIXEL_FORMAT_PACKED(fmt) ((fmt >> 6) & 0x1)
// #define VIDRE_PIXEL_FORMAT_BITS(fmt)   (fmt & 0x3F)

//         /** Pixel format.
//          * */
//         class VIDRE_CORE_VISIBILITY pixel_format {
//         public:
//             // clang-format off
//             enum type : int32_t {
//                 undefined = -1,

//                 // Major formats (used for analysis tools):
//                 mono8       = VIDRE_PIXEL_FORMAT(0x00, 0, 0, 8),
//                 mono10      = VIDRE_PIXEL_FORMAT(0x01, 0, 0, 10),
//                 mono12      = VIDRE_PIXEL_FORMAT(0x02, 0, 0, 12),
//                 mono16      = VIDRE_PIXEL_FORMAT(0x03, 0, 0, 16),
//                 rgb24       = VIDRE_PIXEL_FORMAT(0x04, 1, 0, 24),
//                 rgba32      = VIDRE_PIXEL_FORMAT(0x05, 1, 0, 32),

//                 // Specific video formats:
//                 bgr24       = VIDRE_PIXEL_FORMAT(0x06, 1, 0, 24),
//                 bgra32      = VIDRE_PIXEL_FORMAT(0x08, 1, 0, 32),
//                 yuv420p     = VIDRE_PIXEL_FORMAT(0x09, 1, 0, 12),
//                 yuyv422     = VIDRE_PIXEL_FORMAT(0x0A, 1, 1, 12),
//                 yuv422p     = VIDRE_PIXEL_FORMAT(0x0B, 1, 0, 16),
//                 yuv444p     = VIDRE_PIXEL_FORMAT(0x0C, 1, 0, 24),
//                 // FIXME see ffmpeg/util/pixelformat.hh

//                 // FIXME add more formats, and make sure to support the ones that are actually used by the cameras we support
//             };
//             // clang-format on

//             /** Transform function.
//              * @note This transform function is not expected to check for
//              *       buffer sizes, so the caller is responsible for ensuring that
//              *       the destination buffer is large enough to hold the converted data.
//              * */
//             using transform_function = bool (*)(const storage::view &src, storage::view &dst) noexcept;

//         public:
//             pixel_format() = default;
//             pixel_format(type fmt, bool packed) noexcept : pix_fmt_{fmt}, packed_{packed} {}
//             pixel_format(std::string_view name, bool packed) noexcept : pix_fmt_{parse(name)}, packed_{packed} {}

//         protected:
//             type pix_fmt_{type::undefined};  ///< Pixel format of the video frame.
//             bool packed_{false};             ///< Whether the pixel data is packed. 
//             // FIXME pix_fmt_ contains PACKED!!!!

//         protected:
//             static type parse(std::string_view name) noexcept;

//         public:
//             /** Convert to @c format.
//              * */
//             operator type() const noexcept {
//                 return pix_fmt_;
//             }

//             /** Assign a new format.
//              * */
//             pixel_format &operator=(type fmt) noexcept {
//                 pix_fmt_ = fmt;
//                 return *this;
//             }

//             /** Assign a new format by name.
//              * */
//             pixel_format &operator=(std::string_view name) noexcept {
//                 pix_fmt_ = parse(name);
//                 return *this;
//             }

//             /** Check if packed.
//              * */
//             bool is_packed() const noexcept {
//                 return packed_;
//             }

//             /** Check if equal.
//              * */
//             bool operator==(const pixel_format &other) const noexcept {
//                 return pix_fmt_ == other.pix_fmt_ && packed_ == other.packed_;
//             }
//             /** Check if not equal.
//              * */
//             bool operator!=(const pixel_format &other) const noexcept {
//                 return !(*this == other);
//             }

//             /** Get a converter for target format.
//              * @see transform_function
//              * */
//             transform_function convert(const pixel_format &target) const;
//         };

        ///@}

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_PIXEL_HH
