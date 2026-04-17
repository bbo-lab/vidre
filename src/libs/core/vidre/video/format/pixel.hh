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

/**@addtogroup api-core-video-format-pixel
 * @ingroup api-core-video-format
 * @details
 * @fileinfo
 *
 * Encoding and decoding of pixel formats, and conversion between them.
 *
 * @see FFmpeg's [pixfmt.h](https://ffmpeg.org/doxygen/trunk/pixfmt_8h.html)
 * */

#include <vidre/core/config.hh>
#include <vidre/video/format/geometry.hh>
#include <vidre/storage.hh>
#include <vidre/exception.hh>
#include <vidre/util/status.hh>

#include <string>
#include <string_view>
#include <bit>

#include <cstdint>
#include <cstddef>

#if defined(__linux__)
// this is only used during developement, get rid of this dependency later on...
#include <linux/videodev2.h>
#endif

namespace vidre {

    namespace video {

        namespace format {

            /**@addtogroup api-core-video-format-pixel
             * */
            ///@{

            /** Pixel formats.
             * There are a multitude of pixel formats, and they are often encoded in a compact way. This class provides a unified
             * interface to encode and decode pixel formats, and to convert between them.
             * */
            class __vidre_packed__(8) VIDRE_CORE_VISIBILITY pixel {
            public:
                /** Supported pixel format conventions.
                 * */
                enum class convention : uint8_t {
                    vidre = 0,       ///< VidRe's internal pixel format convention (default).
                    fourcc = 1,      ///< FourCC code (default).
                    usb3vision = 2,  ///< USB3 Vision pixel format code (see [USB3 Vision
                                     ///< standard](FIXME)).
                    gigevision = 3,  ///< GigE Vision pixel format code (see [GigE Vision
                                     ///< standard](FIXME)).
                };

            public:
                pixel() noexcept = default;

            protected:
                convention convention_{convention::vidre};
                uint8_t reserved_[3]{};
                uint32_t code_{};

            public:
                /** Pixel-format name.
                 * */
                const char *name() const noexcept;

                /** Get JSON representation of the pixel format.
                 * */
                settings::json json() const noexcept;
            };

            ///@}

        }  // namespace format

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_PIXEL_HH
