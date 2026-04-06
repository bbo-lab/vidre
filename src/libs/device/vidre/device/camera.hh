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
#ifndef VIDRE_DEVICE_CAMERA_HH
#define VIDRE_DEVICE_CAMERA_HH
#pragma once

/**@defgroup api-device-impl-camera Cameras
 * @ingroup api-device-impl
 * @details
 * @fileinfo
 *
 * FIXME
 *
 * */

#include <vidre/device/config.hh>
#include <vidre/device/base.hh>

namespace vidre {

    namespace device {

        /**@addtogroup api-device-impl-camera
         * */
        ///@{

        /** Camera device.
         * */
        class VIDRE_DEVICE_VISIBILITY camera : public virtual base {
        public:
            camera() = default;
            camera(state initial_state) : base(initial_state) {}

            /** Destructor.
             * @internal
             * Implementations should ensure that the camera is properly released and any resources are freed.
             * @endinternal
             * */
            virtual ~camera() {}

        protected:
        public:
            const char *tag() const noexcept override {
                return "camera";
            }
        };

        ///@}

    }  // namespace device

}  // namespace vidre

#endif  // VIDRE_DEVICE_CAMERA_HH