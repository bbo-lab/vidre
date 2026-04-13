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
#ifndef VIDRE_CORE_VIDEO_FRAME_HH
#define VIDRE_CORE_VIDEO_FRAME_HH
#pragma once

/**@addtogroup api-core-video-frame
 * @ingroup api-core-video
 * @details
 * @fileinfo
 *
 * FIXME
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/storage.hh>
#include <vidre/video/common.hh>
#include <vidre/video/pixel.hh>

#include <cstdint>

namespace vidre {

    namespace video {

        /**@addtogroup api-core-video
         * */
        ///@}

#if 0
        /** Video frame.
         * */
        class VIDRE_CORE_VISIBILITY frame {
        public:
            using size_type = std::size_t;

            /** Per-frame information.
             *
             * This structure contains metadata associated with a video frame,
             * such as timestamps, geometry, and pixel format. The actual pixel
             * data of the frame is stored separately in the `data` member of
             * the `frame` class.
             *
             * The object is packed and, therefore, suitable to be stored in a
             * file fileformat. Mind endianness, though.
             *
             * */
            struct __vidre_packed__ VIDRE_CORE_VISIBILITY metadata {
                uint32_t frame_number{};                             ///< Frame number (if available).
                timestamp camera{};                                  ///< Timestamp from the camera (if available).
                timestamp system{};                                  ///< Timestamp from the system clock (steady_clock).)
                geometry layout{};                                   ///< Layout of the video frame (position and size).
                pixel_format::type format{pixel_format::undefined};  ///< Pixel format of the video frame.
            };

        public:
            frame() = default;

            frame(frame &&) noexcept = default;
            frame &operator=(frame &&) noexcept = default;

            frame(const frame &) = delete;
            frame &operator=(const frame &) = delete;

            frame(const metadata &meta, storage::view data) : meta_(meta), data_(data) {}

        protected:
            metadata meta_{};       ///< Per-frame information.
            storage::view data_{};  ///< Raw pixel data of the video frame.

        protected:
            size_type required_size() const noexcept {
                // FIXME calculate required size of data based on geometry and pixel format
                return {};
            }

        public:
        };
#endif

        ///@}

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_FRAME_HH
