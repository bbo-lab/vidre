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

    /** Video formats.
     * @ingroup api-device-impl-camera
     * */
    namespace video {

        /**@addtogroup api-device-impl-camera
         * */
        ///@{

        class VIDRE_DEVICE_VISIBILITY pixel {
        public:
            enum format : int32_t {
                undefined = -1,
                mono8,
                mono10,
                // FIXME add more formats, and make sure to support the ones that are actually used by the cameras we support
            };

        public:
            pixel() = default;
            pixel(std::string_view name) noexcept : pix_fmt_{parse(name)} {}

        protected:
            format pix_fmt_{format::undefined};

        protected:
            static format parse(std::string_view name) noexcept;

        public:
            operator format() const noexcept {
                return pix_fmt_;
            }

            pixel &operator=(format fmt) noexcept {
                pix_fmt_ = fmt;
                return *this;
            }

            pixel &operator=(std::string_view name) noexcept {
                pix_fmt_ = parse(name);
                return *this;
            }
        };

        ///@}

    }  // namespace video

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
            const char *tag() const noexcept override{
                return "camera";
            }

            settings::json info() const noexcept override = 0;
            void setup(settings::json) override = 0;
        };

        ///@}

    }  // namespace device

}  // namespace vidre

#endif  // VIDRE_DEVICE_CAMERA_HH