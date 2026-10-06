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
#ifndef VIDRE_DEVICE_CAMERA_LINUX_UVC_HH
#define VIDRE_DEVICE_CAMERA_LINUX_UVC_HH
#pragma once

#include <vidre/device/config.hh>
#include <vidre/device/camera.hh>
#include <vidre/storage/file.hh>
#include <vidre/video.hh>
#include <vidre/log.hh>

#include <linux/videodev2.h>

namespace vidre {

    /*
    Notes:
    see "UVC Payload Header Metadata"
    -> packet timing

    /dev/video0 (camera) + /dev/video1 (UVC metadata)


    use https://libuvc.github.io/libuvc/
    */

    class uvc : public device::camera {
    public:
        struct enumerator {
            device::list<device::camera> find() noexcept;
        };

    public:
        uvc() = default;
        uvc(device::state initial_state) : device::camera(initial_state) {}
        virtual ~uvc() {
            // FIXME release camera
        }

    protected:
        std::filesystem::path dev_stream_{};  ///< /dev/video? device for video stream
        std::filesystem::path dev_meta_{};    ///< /dev/video? device for UVC metadata stream

        settings::json info_{};  ///< Camera information.

    private:
        storage::file::io fd_[2]{};  // [0] for video stream, [1] for metadata stream

    protected:
        bool query_stream(int fd);

        // Query device parameters.
        void query();

    public:
        const char *tag() const noexcept override {
            return "camera/uvc";
        }

        settings::json info() const noexcept override;
        void setup(settings::json) override;

        bool connect() override;
        void disconnect() override;
    };

}  // namespace vidre

#endif  // VIDRE_DEVICE_CAMERA_LINUX_UVC_HH
