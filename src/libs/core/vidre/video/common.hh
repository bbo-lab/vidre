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
#ifndef VIDRE_CORE_VIDEO_COMMON_HH
#define VIDRE_CORE_VIDEO_COMMON_HH
#pragma once

/**@addtogroup api-core-video
 * @details
 * @fileinfo
 *
 * The core library provides a number of fundamental mechanisms for storing data
 * generically. File formats are, however, only supported through plugins.
 *
 * */

#include <vidre/core/config.hh>

#include <chrono>
#include <cstdint>

namespace vidre {

    namespace video {

        /**@addtogroup api-core-video
         * */
        ///@{

        /** Position.
         * */
        struct __attribute__((packed, aligned(8))) VIDRE_CORE_VISIBILITY position {
            uint32_t x;  ///< X coordinate of the top-left corner of the video frame.
            uint32_t y;  ///< Y coordinate of the top-left corner of the video frame.
        };

        /** Size.
         * */
        struct __attribute__((packed, aligned(8))) VIDRE_CORE_VISIBILITY size {
            uint32_t w;  ///< Width of the video frame in pixels.
            uint32_t h;  ///< Height of the video frame in pixels.
        };

        /** Geometry.
         * @see video::position
         * @see video::size
         * */
        struct __attribute__((packed, aligned(8))) VIDRE_CORE_VISIBILITY geometry
            : public position
            , public size {};

        /** Timestamp.
         * To avoid calculations (esp. divisions) on the critical path of frame
         * acquisition, timestamps are represented as a value and a resolution
         * (in Hz). The actual timestamp in seconds can be calculated as
         * `value / resolution`. The default resolution is 1 ns (1 GHz), which
         * allows for high-precision timestamps without needing to use
         * floating-point values.
         *
         * @see std::chrono::duration for a more flexible representation of time intervals, if needed.
         * */
        struct __attribute__((packed, aligned(8))) VIDRE_CORE_VISIBILITY timestamp {
            uint64_t value;                      ///< Timestamp value.
            uint64_t resolution = {1000000000};  ///< Timestamp resolution (Hz). Default is 1 ns (1 GHz).

            template <class Rep, class Period = std::ratio<1> >
            operator std::chrono::duration<Rep, Period>() const noexcept {
                using namespace std::chrono;
                return duration<Rep, Period>(static_cast<Rep>(value) * Period::den / resolution);
            }
        };

        ///@}

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_COMMON_HH
