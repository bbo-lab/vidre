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
#include <vidre/exception.hh>
#include <vidre/log.hh>

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
         *
         * Any device follow the sequence:
         *
         * available -> connected -> configured -> running
         *
         * @todo flag::error is present, but what to do with it?
         * */
        class VIDRE_DEVICE_VISIBILITY state {
        public:
            enum flag : uint32_t {
                undefined = 0,        ///< Device is in an undefined state.
                available = 1 << 0,   ///< Device is available.
                disabled = 1 << 1,    ///< Device is disabled (possibly due to unrecoverable issues).
                error = 1 << 2,       ///< Device experienced issues.
                connected = 1 << 3,   ///< Device is connected to VidRe.
                configured = 1 << 4,  ///< Device is configured and ready to be started.
                running = 1 << 5,     ///< Device is active.
            };

            template<typename T>
            static constexpr bool compatible_v = std::derived_from<T, state> or std::derived_from<T, state::flag>;

            state() noexcept : flags_(flag::undefined) {}
            state(flag f) noexcept : flags_(f) {}

        protected:
            uint32_t flags_;  ///< Device state flags.

        public:
            operator bool() const noexcept {
                return flags_ != static_cast<uint32_t>(flag::undefined);
            }   

            bool operator!() const noexcept {
                return flags_ == static_cast<uint32_t>(flag::undefined);
            }

            bool operator==(flag f) const noexcept {
                return flags_ == static_cast<uint32_t>(f);
            }   

            bool operator==(state s) const noexcept {
                return flags_ == s.flags_;
            }

            bool has(flag f) const noexcept {
                return (flags_ & static_cast<uint32_t>(f)) != 0;
            }

            state &set(flag f) noexcept {
                flags_ |= static_cast<uint32_t>(f);
                return *this;
            }

            state &clear(flag f) noexcept {
                flags_ &= ~static_cast<uint32_t>(f);
                return *this;
            }

            state &toggle(flag f) noexcept {
                flags_ ^= static_cast<uint32_t>(f);
                return *this;
            }

            state &reset() noexcept {
                flags_ = static_cast<uint32_t>(flag::undefined);
                return *this;
            }
          
            state &operator&=(flag f) noexcept {
                flags_ &= static_cast<uint32_t>(f);
                return *this;
            }   

            state &operator&=(const state &f) noexcept {
                flags_ &= f.flags_;
                return *this;
            }   

            state &operator|=(flag f) noexcept {
                flags_ |= static_cast<uint32_t>(f);
                return *this;
            }   

            state &operator|=(const state &f) noexcept {
                flags_ |= f.flags_;
                return *this;
            }   
        };

        ///@cond
        template<class LHS, class RHS>
        requires (state::compatible_v<LHS> && state::compatible_v<RHS>)
        inline LHS operator|(LHS lhs, RHS rhs) noexcept {
            return (lhs |= rhs);
        }
        template<class LHS, class RHS>
        requires (state::compatible_v<LHS> && state::compatible_v<RHS>)
        inline LHS operator&(LHS lhs, RHS rhs) noexcept {
            return (lhs &= rhs);
        }
        inline state::flag operator~(state::flag val) noexcept {
            return static_cast<state::flag>(~to_underlying(val));
        }
        ///@endcond

        /** Device skeleton.
         *
         * Devices may be hard- or software devices. The latter applies, for example,
         * to filters, and the like. Each device provides streams, ie. sinks
         * and sources, as well as generic configuration options. Specific implementations
         * will add more direct functionality.
         *
         * Stages:
         * 1. available: The device is available for use, discovered by the enumerator
         *               (see state::flag::available).
         * 2. connected: The device is connected to VidRe, ready for querying and configuration
         *               (see state::flag::connected).
         * 3. setup: The device configuration is setup, but not yet configured.
         * 4. prepare: The device is prepared, which includes internal buffers, signals and streams.
         *             At this stage, the device is configured (see state::flag::configured).
         * 5. start/stop: The device can be started and stopped (see state::flag::running).
         *
         * Device configuration is stored internally as a JSON object, which is also
         * returned by the `info()` method. This allows for a flexible and extensible
         * way to store and retrieve device information, and also allows for user-defined
         * configuration options. The general options are defined as follows:
         * - `device`:  A string that specifies the device class or type (e.g., "camera/uvc").
         * - `name`:    Human-readable name for the device, which should be unique.
         *              It can be used for display purposes and may be user-settable.
         * - `id`:      Unique identifier for the device, which can be used to reference
         *              it in settings and other contexts. If the device does not have a
         *              unique ID, it should return an empty string.
         * - `private`: Map of device- or operating-system specific configuration options and
         *              information. The contents of this object are defined by the specific
         *              device implementation and may include details such as hardware parameters,
         *              stream configurations, and other relevant information.
         * - `setup`:   The actual device configuration (see `setup()` method).
         * - `config`:  List of configuration options for the device. This is used to generate
         *              user interfaces for configuring the device.
         * */
        class VIDRE_DEVICE_VISIBILITY base {
        public:
            struct enumerator;

        public:
            base() = default;
            base(state initial_state) : state_{initial_state} {}
            virtual ~base() = default;

        protected:
            state state_{state::undefined};  ///< Current device state.
            settings::json meta_{};          ///< Device metadata and configuration.

        public:
            /** Device class tag.
             * */
            std::string tag() const noexcept {
                return meta_["device"].is_string() ? meta_["device"].get<std::string>().c_str() : "unknown";
            }

            /** Get device status.
             * */
            state status() const noexcept {
                return state_;
            }

            /** Get device metadata.
             * */
            virtual settings::json meta() const noexcept {
                return meta_;
            }

            /** Connect device.
             * @return `true` if the device was successfully connected, `false` otherwise.
             * */
            virtual bool connect() = 0;

            /** Disconnect device.
             * */
            virtual void disconnect() = 0;

            /** Setup device.
             *
             * This method is used to configure the device with specific settings. The exact
             * configuration options depend on the device implementation.
             *
             * There are three cases for how this method can be used:
             * 1. The device is not connected
             * 2. The device is connected but not running
             * 3. The device is running
             *
             * @note The `setup()` method should be called after connecting the device. It
             *       must be called at least once before starting the device. Depending on
             *       the device implementation, it may be possible to call `setup()` also
             *       in the running state with a subset of options.
             *
             * @see prepare()
             * */
            virtual void setup(const settings::json & = {}) = 0;

            /** Prepare device for streaming.
             * @see state::flag::configured
             * */
            virtual void prepare() = 0;

            /** Start streaming.
             * @note This method silently returns if the device is not configured or already running.
             * */
            virtual void start() = 0;

            /** Stop streaming.
             * @note This method silently returns if the device is not running.
             * */
            virtual void stop() = 0;

            /** Get device ID.
             * The device ID is a unique identifier for the device, which can be
             * used to reference it in settings and other contexts.
             *
             * If the device does not have a unique ID, it should return an empty string.
             * */
            std::string id() const noexcept {
                return meta_["id"].is_string() ? meta_["id"].get<std::string>() : "";
            }

            /** Get device name.
             * The device name is a human-readable name for the device, which should
             * be unique. It can be used for display purposes and may be user-settable.
             * */
            std::string name() const noexcept {
                return meta_["name"].is_string() ? meta_["name"].get<std::string>() : "";
            }

            /** Set device name.
             * @param username The new name for the device. It should be unique, but this is not enforced.
             * @return `true` if the name was successfully set, `false` otherwise (e.g., if the does not support it).
             * */
            virtual bool name(const std::string &username);
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
        using list_type = std::vector<shared<Device>>;

        using list = list_type<base>;

        /** Hardware enumerator.
         * The enumerator, which must be implemented for each device type,
         * provides a method to discover available devices of that type. It
         * is provided here only for documentation purposes, and should be
         * implemented in the respective device implementation files.
         * */
        struct base::enumerator {
            list find() noexcept {
                return {};
            }
        };

        ///@}

        /** Exception and error namespace.
         * @ingroup api-device-basics
         * */
        namespace error {
            using string = vidre::error::string;
            using info = vidre::error::info;

            /** Device id info.
             * */
            class VIDRE_DEVICE_VISIBILITY id : public virtual error::info {
            public:
                id() noexcept {}
                virtual ~id() {}

                id(const std::string &id) noexcept : id_(id) {}

                template <class Device>
                requires std::derived_from<Device, base>
                id(const shared<Device> &device) noexcept : id_(device->id()) {}

                template <class Device>
                requires std::derived_from<Device, base>
                id(const Device &device) noexcept : id_(device.id()) {}

                template <class Device>
                requires std::derived_from<Device, base>
                id(const Device *device) noexcept : id_(device->id()) {}

            protected:
                error::string id_{};

            public:
                const char *tag() noexcept override {
                    return "device/id";
                }

                void make(error::string &emsg) noexcept override {
                    emsg += "    id: " + id_;
                }
            };

            /** Device name info.
             * */
            class VIDRE_DEVICE_VISIBILITY name : public virtual error::info {
            public:
                name() noexcept {}
                virtual ~name() {}

                name(const std::string &name) noexcept : name_(name) {}

                template <class Device>
                requires std::derived_from<Device, base>
                name(const shared<Device> &device) noexcept : name_(device->name()) {}

                template <class Device>
                requires std::derived_from<Device, base>
                name(const Device &device) noexcept : name_(device.name()) {}

                template <class Device>
                requires std::derived_from<Device, base>
                name(const Device *device) noexcept : name_(device->name()) {}

            protected:
                error::string name_{};

            public:
                const char *tag() noexcept override {
                    return "device/name";
                }

                void make(error::string &emsg) noexcept override {
                    emsg += "    name: " + name_;
                }
            };

        }  // namespace error

        /** Device exception.
         * */
        class VIDRE_CORE_VISIBILITY exception : public virtual vidre::exception::runtime {
        public:
            exception(const std::source_location &src = std::source_location::current()) noexcept
                : vidre::exception::runtime(src) {}
            virtual ~exception() {}

            template <class Device>
            requires std::derived_from<Device, base>
            exception(const shared<Device> &device, const std::source_location &src = std::source_location::current()) noexcept
                : vidre::exception::runtime(src), tag_(error::string(device->tag()) + " error") {
                *this << error::id(device);
            }

            template <class Device>
            requires std::derived_from<Device, base>
            exception(const Device &device, const std::source_location &src = std::source_location::current()) noexcept
                : vidre::exception::runtime(src), tag_(error::string(device->tag()) + " error") {
                *this << error::id(device);
            }

            template <class Device>
            requires std::derived_from<Device, base>
            exception(const Device *device, const std::source_location &src = std::source_location::current()) noexcept
                : vidre::exception::runtime(src), tag_(error::string(device->tag()) + " error") {
                *this << error::id(device);
            }

        protected:
            error::string tag_{"device error"};

        public:
            const char *what() const noexcept override;
        };

    }  // namespace device

}  // namespace vidre

#endif  // VIDRE_DEVICE_BASE_HH
