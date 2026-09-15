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
#ifndef VIDRE_CORE_VIDEO_FORMAT_GEOMETRY_HH
#define VIDRE_CORE_VIDEO_FORMAT_GEOMETRY_HH
#pragma once

/**@addtogroup api-video-format-geometry
 * @ingroup api-video-format
 * @details
 * @fileinfo
 *
 * */

#include <vidre/video/config.hh>
#include <vidre/error.hh>

#include <string>
#include <string_view>
#include <bit>

#include <cstdint>
#include <cstddef>

namespace vidre {

    namespace video {

        namespace format {

            /**@addtogroup api-video-format-geometry
             * */
            ///@{

            /** Position.
             * */
            struct __vidre_packed__(8) VIDRE_VIDEO_VISIBILITY position {
                uint32_t x;  ///< X coordinate of the top-left corner of the video frame.
                uint32_t y;  ///< Y coordinate of the top-left corner of the video frame.
            };

            /** Size.
             * */
            struct __vidre_packed__(8) VIDRE_VIDEO_VISIBILITY size {
                uint32_t w;  ///< Width of the video frame in pixels.
                uint32_t h;  ///< Height of the video frame in pixels.
            };

            /** Geometry.
             * @see video::position
             * @see video::size
             * */
            struct __vidre_packed__(8) VIDRE_VIDEO_VISIBILITY geometry
                : public position
                , public size {};

            ///@}

        }  // namespace format

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_FORMAT_GEOMETRY_HH
