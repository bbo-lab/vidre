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
#ifndef VIDRE_DEVICE_HH
#define VIDRE_DEVICE_HH
#pragma once

            /** Start/stop order.
             * @returns The start/stop order of this endpoint (0 for highest), if defined (default: undefined).
             *
             * @note Endpoints with lower order values are started _later_ but stopped _earlier_.
             *       (cf. with camera devices where a camera that triggers other cameras must be
             *       started last, so that neighboring cameras will not miss the trigger signal.)
             *
             * @internal
             * An undefined return value is generally interpreted in a way that automatically starts the
             * endpoint on attachment and stops it on detachment.
             * @endinternal
             * */
            virtual std::optional<unsigned> order() const noexcept {
                return {};
            }


/**@addtogroup api-device
 * @details
 * @fileinfo
 *
 * FIXME
 * 
 * @{
 * @namespace vidre::device Devices namespace.
 * @defgroup api-device-basics     Fundamentals
 * @defgroup api-device-impl       Device implementations
 * @defgroup api-device-details    Implementation details
 * @}
 * */

#include <vidre/device/config.hh>
#include <vidre/device/base.hh>
#include <vidre/device/camera.hh>

#endif  // VIDRE_DEVICE_CONFIG_HH
