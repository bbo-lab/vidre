/* VidRe
 * Copyright (C) 2024-2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_FFMPEG_HH
#define VIDRE_FFMPEG_HH
#pragma once

/**@defgroup api-support-ffmpeg FFmpeg C++ API
 * @ingroup api-support
 * @details
 * @fileinfo
 *
 * @{
 * @defgroup api-support-ffmpeg-overview Overview
 * @defgroup api-support-ffmpeg-format Format
 * @defgroup api-support-ffmpeg-codec Codec
 * @defgroup api-support-ffmpeg-util Utilities
 * @defgroup api-support-ffmpeg-details Implementations details
 * @}
 * */

#include <vidre/ffmpeg/config.hh>
#include <vidre/ffmpeg/util.hh>
#include <vidre/ffmpeg/format.hh>

namespace vidre {

    /** FFmpeg namespace
     * @ingroup api-support-ffmpeg
     */
    namespace ffmpeg {}

}  // namespace vidre

#endif  // VIDRE_FFMPEG_HH
