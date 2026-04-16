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

    namespace detail {

        namespace video {

            /**@addtogroup api-core-detail
             * */
            ///@{

            // FIXME

            ///@}

        }  // namespace video

    }  // namespace detail

    namespace video {

        namespace format {

            /**@addtogroup api-core-video-format-pixel
             * */
            ///@{

            /** Pixel formats.
             *
             * */
            class VIDRE_CORE_VISIBILITY pixel {
            public:
            //     /** Supported pixel format conventions.
            //      * */
            //     enum class convention {
            //         fourcc,      ///< FourCC code (default).
            //         usb3vision,  ///< USB3 Vision pixel format code (see [USB3 Vision
            //                      ///< standard](FIXME)).
            //         gigevision,  ///< GigE Vision pixel format code (see [GigE Vision
            //                      ///< standard](FIXME)).
            //     };

            //     /** Generate pixel format from a string representation.
            //      * @tparam Conv The pixel format convention to use (default: FourCC).
            //      * @param name The string representation of the pixel format.
            //      * @return A pixel object representing the specified format.
            //      * */
            //     template <convention Conv = convention::fourcc>
            //     pixel make(const std::string &name) noexcept;

            //     /** Preferred pixel formats.
            //      * */
            //     struct preferred {
            //         static constexpr fourcc gray8 = fourcc::grey;
            //         static constexpr fourcc gray10 = fourcc::y10;
            //         static constexpr fourcc gray12 = fourcc::y12;
            //         static constexpr fourcc gray14 = fourcc::y14;
            //         static constexpr fourcc gray16 = fourcc::y16;

            //         static constexpr fourcc rgb24 = fourcc::rgb24;
            //         static constexpr fourcc rgb32 = fourcc::rgba32;
            //     };

            // public:
            //     pixel() noexcept = default;
            //     explicit pixel(fourcc format) noexcept : format_(format) {}

            // protected:
            //     fourcc format_{};

            // public:
            //     /** Check if valid.
            //      * */
            //     bool valid() const noexcept {
            //         return format_ != fourcc::invalid;
            //     }

            //     /** Convert to FourCC code.
            //      * */
            //     operator fourcc() const noexcept {
            //         return format_;
            //     }

            //     /** Comparison. */
            //     bool operator==(const pixel &other) const noexcept {
            //         return format_ == other.format_;
            //     }

            //     /** Get the name of the pixel format.
            //      * @return A string representing the name of the pixel format.
            //      * */
            //     std::string name(convention conv = convention::fourcc) const noexcept;
            };

            // /** FIXME pixel-format converter
            //  * */
            // template <auto Input, auto Output>
            // struct convert {
            //     storage::size_type size(format::size) const noexcept;
            //     status operator()(format::size, const uint8_t *, uint8_t *) noexcept;
            // };

            ///@}

        }  // namespace format

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_PIXEL_HH
