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
#include <memory>
#include <string>
#include <vector>

namespace vidre {

    namespace device {

        /**@addtogroup api-device-basics
         * */
        ///@{

        /** Device state.
         * */
        enum class state : uint32_t {
            undefined = 0,        ///< Device is in an undefined state.
            available = 1 << 0,   ///< Device is available.
            disabled = 1 << 1,    ///< Device is disabled (possibly due to unrecoverable issues).
            connected = 1 << 2,   ///< Device is connected to VidRe.
            configured = 1 << 3,  ///< Device is configured and ready to be started.
            running = 1 << 4,     ///< Device is active.

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

        /** Abstract device.
         *
         * Devices may be hard- or software devices. The latter applies, for example,
         * to filters, and the like. Each device provides streams, ie. sinks
         * and sources, as well as generic configuration options. Specific implementations
         * will add more direct functionality.
         *
         * */
        class VIDRE_DEVICE_VISIBILITY base {
        public:
            struct enumerator;

        public:
            base() = default;
            base(state initial_state) : state_{initial_state} {}
            virtual ~base() = default;

        protected:
            state state_{state::undefined};

        public:
            /** Device class tag.
             * */
            virtual const char *tag() const noexcept {
                return "base";
            }

            /** General device info.
             *
             * FIXME describe syntax!!!
             *
             * */
            virtual settings::json info() const noexcept = 0;

            /** Get device status.
             * */
            state status() const noexcept {
                return state_;
            }

            // FIXME status logger

            /** Setup device.
             * */
            virtual void setup(settings::json) = 0;

            /** Get device ID.
             * The device ID is a unique identifier for the device, which can be
             * used to reference it in settings and other contexts. It should be,
             * if possible, rather a unique, serial number than a user-settable name.
             */
            virtual std::string id() const noexcept = 0;

            /** Get device name.
             * The device name is a human-readable name for the device, which should
             * be unique. It can be used for display purposes and may be user-settable.
             */
            virtual std::string name() const noexcept = 0;

            /** Set device name.
             * @param username The new name for the device. It should be unique, but this is not enforced.
             * @return `true` if the name was successfully set, `false` otherwise (e.g., if the does not support it).
             *
             * */
            virtual bool name(const std::string &username) = 0;
        };

        /** "Shared device".
         * Despite some overhead for reference counting, we will use shared pointers
         * to manage device instances. This alias is provided for convenience and
         * clarity in the codebase.
         * */
        template <typename Device>
        using shared = std::shared_ptr<Device>;

        /** Device list.
         * */
        template <typename Device>
        using list = std::vector<shared<Device>>;

        /** Hardware enumerator.
         * The enumerator, which must be implemented for each device type,
         * provides a method to discover available devices of that type. It
         * is provided here only for documentation purposes, and should be
         * implemented in the respective device implementation files.
         * */
        struct base::enumerator {
            list<base> find() noexcept {
                return {};
            }
        };

        ///@}

        namespace failure {

            //

        }  // namespace failure

    }  // namespace device

}  // namespace vidre

#endif  // VIDRE_DEVICE_BASE_HH
