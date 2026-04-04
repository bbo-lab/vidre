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
#ifndef VIDRE_DEVICE_BASE_HH
#define VIDRE_DEVICE_BASE_HH
#pragma once

#include <vidre/device/config.hh>
#include <vidre/settings/json.hh>
#include <vidre/cpp/util.hh>

#include <cstdint>

namespace vidre {

    namespace device {

        /**@addtogroup api-device-basics
         * */
        ///@{

        /** Device state.
         * */
        enum class state : uint32_t {
            undefined = 0,       ///< Device is in an undefined state.
            available = 1 << 0,  ///< Device is available.
            connected = 1 << 1,  ///< Device is connected to VidRe.
            armed = 1 << 2,      ///< Device is prepared for recording.
            running = 1 << 3,    ///< Device is active.

            failed = 1 << 8,  ///< Device experienced issues.
        };

        ///@cond
        inline state operator|(state lhs, state rhs) noexcept {
            return static_cast<state>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline state operator&(state lhs, state rhs) noexcept {
            return static_cast<state>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline state operator~(state val) noexcept {
            return static_cast<state>(~to_underlying(val));
        }
        ///@endcond

        /** General representation of a "device".
         *
         * Devices may be hard- or software devices. The latter applies, for example,
         * to filters, and the like.
         * 
         * To avoid complexity of the C++-API due to a possibly large feature set devices
         * may have, the device classes provide a "slow" and "fast" interaction. On the
         * "slow" side are functions retrieving general info, configuration and more 
         * (which will use JSON). On the "fast" side are a limited number of controls
         * (e.g. a cameras shutter time) and IO.
         * 
         * */
        class VIDRE_DEVICE_VISIBILITY base {
        public:
            base() = default;

        public:
            /** General device info.
             *
             * FIXME describe syntax!!!
             *
             * */
            virtual settings::json info() const noexcept;

            /** Get device status.
             * */
            virtual state status() const noexcept {
                return state::undefined;
            }

            // FIXME status logger

            /** Setup device.
             * */
            virtual void setup(json) = 0;


        };

        ///@}

    }  // namespace device

}  // namespace vidre

#endif  // VIDRE_DEVICE_BASE_HH
