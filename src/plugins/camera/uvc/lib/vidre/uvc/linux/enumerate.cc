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
#include "../device.hh"
#include "uvc.hh"

using namespace vidre;
using namespace vidre::video;

device::list uvc::camera::enumerator::find() noexcept {
    device::list found;

    log::trace tr;
    tr << "Enumerating UVC cameras...\n";

    bool have_uvch = true;  // flag, if we parse for UVC metadata devices
    int lastcam = -1;       // last camera number found, used to find UVC metadata devices

    for (int dev = 0; dev < 64; ++dev) {
        std::filesystem::path devpath = std::format("/dev/video{}", dev);

        // check if device exists (and get rdev info)
        struct stat st;
        if (stat(devpath.c_str(), &st) < 0)
            continue;

        std::string rdev = std::format("{}:{}", major(st.st_rdev), minor(st.st_rdev));
        tr << std::format("  {} ({})\n", devpath.string(), rdev);

        // get capabilities and check if this is a UVC camera
        io::file fd = io::file(devpath, io::file::access::readwrite);
        if (!fd) {
            tr << std::format("  !!! failed to open device: {}\n", std::strerror(errno));
            continue;
        }

        v4l2_capability cap;
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
            tr << std::format("  !!! failed to query device capabilities: {}\n", std::strerror(errno));
            continue;
        }

        if (not(cap.device_caps & V4L2_CAP_STREAMING)) {
            tr << "  !!! not a streaming device, ignoring\n";
            continue;
        }

        if (cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) {
        }
        else if (cap.device_caps & V4L2_CAP_META_CAPTURE) {
        }
        else {
            tr << "  !!! not a video capture or metadata device, ignoring.\n";
            continue;
        }

        // get media information if available
        std::filesystem::path sysdir = std::format("/sys/dev/char/{}/device", rdev);
        std::string serial = {};
        for (const auto &entry : std::filesystem::directory_iterator(sysdir)) {
            auto filename = entry.path().filename().string();

            if (filename.find("media") == 0) {
                tr << std::format("         {}:\n", filename);

                io::file fd = io::file(std::format("/dev/{}", filename), io::file::access::readonly);
                if (fd.valid()) {
                    struct media_device_info mdi;
                    if (ioctl(fd, MEDIA_IOC_DEVICE_INFO, &mdi) == 0) {
                        tr << std::format("             driver:   {}\n", (const char *)mdi.driver);
                        tr << std::format("             model:    {}\n", (const char *)mdi.model);
                        tr << std::format("             serial:   {}\n", (const char *)mdi.serial);
                        tr << std::format("             bus_info: {}\n", (const char *)mdi.bus_info);
                        tr << std::format("             version:  {}.{}.{}\n", (mdi.media_version >> 16) & 0xFF,
                                          (mdi.media_version >> 8) & 0xFF, mdi.media_version & 0xFF);

                        serial = (const char *)mdi.serial;
                    }
                }

                break;
            }
        }

        if (cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) {
            tr << "    -> video capture device\n";
            uvc::camera *cam = new uvc::camera(device::state::available);
            std::string name = std::string((const char *)cap.card)  //
                               + " (" + std::string((const char *)cap.bus_info) + ")";

            cam->meta_["device"] = "camera/uvc";
            cam->meta_["name"] = name;
            cam->meta_["id"] = serial;
            cam->meta_["private"]["stream"] = {
                {"path", devpath.string()},
                {"rdev", rdev},
                {"card", (const char *)cap.card},
                {"bus_info", (const char *)cap.bus_info},
                {"driver", (const char *)cap.driver},
                {"serial", serial},
                {"version", std::format("{}.{}.{}", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF)},
            };

            found.emplace_back(cam);
            lastcam = found.size() - 1;
        }
        else if (cap.device_caps & V4L2_CAP_META_CAPTURE) {
            tr << "    -> metadata capture device\n";

            if (lastcam < 0) {
                // meta-capture should come after video capture devices,
                // if we haven't found any video capture device yet,
                // we won't parse for metadata devices any further
                have_uvch = false;
                tr << "    !!! device listed out of order\n";
            }

            if (!have_uvch) {
                tr << "    !!! ignoring metadata device\n";
                continue;
            }

            dynamic_cast<uvc::camera *>(found[lastcam].get())->meta_["private"]["meta"] = {
                {"path", devpath.string()},
                {"rdev", rdev},
                {"card", (const char *)cap.card},
                {"bus_info", (const char *)cap.bus_info},
                {"driver", (const char *)cap.driver},
                {"serial", serial},
                {"version", std::format("{}.{}.{}", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF)}};
        }
    }

    // query configuration and capabilities for found cameras
    for( auto &dev : found) 
        dynamic_cast<uvc::camera *>(dev.get())->query();

    return found;
}
