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
#ifndef VIDRE_CORE_VIDEO_HH
#define VIDRE_CORE_VIDEO_HH
#pragma once

/**@addtogroup api-core-video
 * @details
 * @fileinfo
 *
 * The core library provides a number of fundamental mechanisms for storing data
 * generically. File formats are, however, only supported through plugins.
 *
 * @{
 * @defgroup api-core-video-pixel Pixel formats
 * @defgroup api-core-video-frame Video frames
 * @defgroup api-core-video-io Video I/O
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/video/common.hh>
#include <vidre/video/pixel.hh>
#include <vidre/video/frame.hh>

#include <string>
#include <string_view>

namespace vidre {

    /** Video-related namespace.
     * @ingroup api-core-video
     * */
    namespace video {}  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_HH
