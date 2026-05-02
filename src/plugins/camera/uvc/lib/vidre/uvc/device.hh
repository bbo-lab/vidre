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
#ifndef VIDRE_UVC_DEVICE_HH
#define VIDRE_UVC_DEVICE_HH
#pragma once

/**@defgroup api-support-uvc UVC support library
 * @ingroup api-support
 * @details
 * @fileinfo
 *
 * FIXME
 *
 * @{
 * @namespace vidre::video::uvc            UVC camera support.
 * @defgroup api-support-uvc-details       Implementation details
 * @}
 * */

/**@addtogroup api-support-uvc-details
 * @{
 * @namespace vidre::video::uvc::detail UVC support implementation details.
 * @}
 * */

#include <vidre/uvc/config.hh>
#include <vidre/device/camera.hh>
#include <vidre/io.hh>

namespace vidre {

    namespace video {

        namespace uvc {

            /**@addtogroup api-support-uvc
             * */
            ///@{

            /** UVC camera.
             *
             * FIXME
             *
             * */
            class VIDRE_UVC_VISIBILITY camera
                : public virtual device::camera
                , public virtual io::source {
            public:
                struct enumerator;

            public:
                camera() = default;
                camera(device::state initial_state) noexcept : device::camera(initial_state), io::source() {}
                virtual ~camera();

            protected:
#if VIDRE_HOST_LINUX
                io::file vid_;  ///< File handle for the video device (e.g., /dev/video0).
                io::file hdr_;  ///< File handle for the payload header device (e.g., /dev/video1).
#else
                FIXME(Windows and APPLE support);
#endif

                io::offset_type read(memory::area) override;

                void query() noexcept;

            public:
                bool valid() const noexcept override {
                    return vid_.valid();
                }

                cap output() const noexcept override {
                    return cap::packet | cap::task | cap::realtime;
                }

                io::result get(memory::area v) override {
                    // not allowed
                    return io::invalid<io::offset_type>;
                }

                const io::packet::basic *get() override;

                bool connect() override;

                void disconnect() override;

                void setup(const settings::json & = {}) override;

                void prepare() override;

                void start() override;

                void stop() override;

            private:
                friend struct enumerator;
            };

            struct camera::enumerator {
                device::list find() noexcept;
            };

            ///@}

        }  // namespace uvc

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_UVC_DEVICE_HH
