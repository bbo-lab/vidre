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
#ifndef VIDRE_UVC_CONFIG_HH
#define VIDRE_UVC_CONFIG_HH
#pragma once

#include <vidre/config.hh>
#include <vidre/device/camera.hh>

namespace vidre {

    namespace video {

        namespace uvc {

            /**@addtogroup api-support-uvc
             * */
            ///@{

            class VIDRE_UVC_VISIBILITY camera : public virtual device::camera {
            public:
                struct enumerator;
                
            public:
                camera() = default;
                virtual ~camera();

            public:
                const char *tag() const noexcept override {
                    return "camera/uvc";
                }

                settings::json info() const noexcept override {
                    // FIXME
                    return {};
                }

                void setup(settings::json) override {
                    // FIXME
                }

                bool connect() override {
                    // FIXME
                    return false;
                }

                void disconnect() override {
                    // FIXME
                }
            };

            ///@}

        }  // namespace uvc

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_UVC_CONFIG_HH
