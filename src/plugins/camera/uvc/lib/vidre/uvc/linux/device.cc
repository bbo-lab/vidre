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

uvc::camera::~camera() {
    disconnect();
}

io::offset_type uvc::camera::read(memory::area &) {
    // FIXME
    return io::invalid<io::offset_type>;
}

const io::packet::base *uvc::camera::get() {
    // FIXME
    return nullptr;
}

bool uvc::camera::connect() {
    if ((state_ & device::state::connected) != device::state::undefined)
        return false;

    auto vid_path = meta_["private"]["stream"]["path"].get<std::string>();
    vid_.open(vid_path, io::file::access::readwrite);
    if (!vid_.valid()) {
        log::error() << "failed to open UVC video device " << vid_path;
        return false;
    }

    if (meta_["private"]["meta"].is_object()) {
        auto meta_path = meta_["private"]["meta"]["path"].get<std::string>();
        hdr_.open(meta_path, io::file::access::readwrite);
        if (!hdr_.valid()) {
            log::error() << "failed to open UVC metadata device " << meta_path << " (ignoring)";
        }
    }

    state_ |= device::state::connected;
    return true;
}

void uvc::camera::disconnect() {
    if ((state_ & device::state::connected) == device::state::undefined)
        return;

    if (vid_.valid())
        vid_.close();
    if (hdr_.valid())
        hdr_.close();

    state_ &= ~device::state::connected;
}

void uvc::camera::setup(const settings::json &cfg) {
    if ((state_ & device::state::connected) == device::state::undefined) {
        // FIXME proper error messages...
        log::error() << "device is not connected";
        throw device::exception();
        return;
    }

    // note: configured means, buffers are allocated
    bool isconfigured = (state_ & device::state::configured) != device::state::undefined;

    // note: only a subset of options can be changed while running
    bool isrunning = (state_ & device::state::running) != device::state::undefined;

    if (cfg.is_object()) {
        if (isrunning) {
            // only known options while running
            for (auto &[key, value] : cfg.items()) {
                auto it = device::camera::options.find(key.c_str());
                if (it != device::camera::options.end()) {
                    auto opt = it->second;
                    if (!device::camera::online[opt])
                        continue;

                    switch (opt) {
                        case device::camera::exposure:
                            // FIXME
                            break;
                        case device::camera::gain:
                            // FIXME
                            break;
                        case device::camera::geometry:
                            // FIXME
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        else {
            // UVC configuration requires to set some parameters in a specific order:
            // 1. pixel-format
            // 2. geometry (width, height)
            // 3. other options (e.g., exposure, gain)
        }
    }
    else if (!cfg.is_null()) {
        // we just ignore it for now, but maybe we should throw an error here?
    }

    if (not isconfigured) {
        // FIXME allocate buffers, etc.
        state_ |= device::state::configured;
    }
}

void uvc::camera::start() {
    if ((state_ & device::state::configured) == device::state::undefined) {
        // FIXME proper error messages...
        log::error() << "device is not configured";
        throw device::exception();
        return;
    }

    // FIXME
}

void uvc::camera::stop() {
    if ((state_ & device::state::running) == device::state::undefined)
        return;

    // FIXME
}

void uvc::camera::query() noexcept {
    bool isconnected = (state_ & device::state::connected) != device::state::undefined;
    if (!isconnected)
        connect();

    // FIXME

    if (!isconnected)
        disconnect();
}