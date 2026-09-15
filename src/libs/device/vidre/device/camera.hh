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
#ifndef VIDRE_DEVICE_CAMERA_HH
#define VIDRE_DEVICE_CAMERA_HH
#pragma once

#include <vidre/device/config.hh>
#include <vidre/device/base.hh>
#include <unordered_map>

namespace vidre {

    namespace device {

        /**@addtogroup api-device-impl
         * */
        ///@{

        /** Camera device.
         *
         * Camera devices are a specific type of device that provide video capture
         * capabilities. They typically interface with hardware cameras and manage
         * the acquisition of video frames.
         *
         * */
        class VIDRE_DEVICE_VISIBILITY camera : public virtual device::base {
        public:
            /** Common options.
             * These options must be supported by all camera implementations.
             * */
            enum option : uint32_t {
                undefined = 0,  ///< No option specified.
                exposure,       ///< Camera exposure time (in microseconds, 0.0 for automatic).
                gain,           ///< Camera gain (in dB).
                geometry,       ///< Pixel geometry.
                MAXOPTIONS
            };
            static std::unordered_map<const char *, option> options;  ///< Name-option map.
            static std::array<bool, MAXOPTIONS> online;                       ///< Options settable while running.

        public:
            camera() = default;
            camera(state initial_state) : device::base(initial_state) {}
            virtual ~camera() = default;
        };

        ///@}

    }  // namespace device

}  // namespace vidre

#endif  // VIDRE_DEVICE_CAMERA_HH
