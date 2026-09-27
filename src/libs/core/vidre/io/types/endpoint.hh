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
#ifndef VIDRE_CORE_IO_TYPES_ENDPOINT_HH
#define VIDRE_CORE_IO_TYPES_ENDPOINT_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/util/debug.hh>
#include <vidre/settings/json.hh>
#include <vidre/io/types/core.hh>

#include <format>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-types
         * */
        ///@{

        /** Endpoint.
         * */
        class VIDRE_CORE_VISIBILITY endpoint {
        public:
            class source;
            class sink;

            /** Syntactic sugar for std::make_shared<T>.
             * @tparam T The type of the endpoint to create, which must be derived from `endpoint`.
             * A syntactic convenience wrapper around `std::make_shared<T>`. Provided solely for readability
             * and consistency when constructing endpoints; no additional logic or validation is performed.
             * */
            template <class T, typename... Args>
            requires std::derived_from<T, endpoint>
            static shared<T> make(Args &&...args) {
                return std::make_shared<T>(std::forward<Args>(args)...);
            }

            /** Type-trait to check if a type is derived from endpoint.
             * */
            template <typename T>
            static constexpr bool isa_v = std::derived_from<T, endpoint>;

            /** Endpoint type.
             * @note The provided types are only for informational (and possibly visual) purposes and
             *       are always computed from the capabilities and configuration of the endpoint.
             * */
            enum class type {
                sink = 0x01,              ///< Endpoint is a sink (i.e., it can consume data).
                source = 0x02,            ///< Endpoint is a source (i.e., it can produce data).
                channel = sink | source,  ///< Endpoint is a channel (i.e., it can both produce and consume data).
                fanout = 0x04 | channel,  ///< Endpoint is a fan-out (i.e., it can distribute a single source to multiple sinks).
                fanin = 0x08 | channel,   ///< Endpoint is a fan-in (i.e., it can combine multiple sources into a single sink).
            };

            /** Endpoint capabilities.
             * These capabilities are used to determine how endpoints can be connected and how data is transmitted.
             * */
            struct VIDRE_CORE_VISIBILITY cap {
                /** Capabilities flags.
                 * These flags indicate the capabilities of an endpoint, such as supported modes and transmission methods.
                 * */
                enum flag_type : uint32_t {
                    none = 0,
                    any = 0xFFFF,  ///< Any mode or method (used as a wildcard).

                    // Transmission modes
                    chunk = 0x0001,   ///< Chunk mode (plain, binary data transmission).
                    packet = 0x0002,  ///< Packet mode (see `io::packet`).

                    // Connection capabilities
                    push = 0x0010,      ///< Supporting push method (where source pushes data to sink).
                    event = 0x0020,     ///< Supporting event-based transmission (sink may pull, event-driven).
                    pull = 0x0020,      ///< Supporting pull method (sink pulls data from source, event-driven).
                    nonblock = 0x0040,  ///< Source requires non-blocking push, sink support non-blocking mode.
                    multi = 0x0080,     ///< Endpoint can be connected to multiple other endpoints (e.g., for broadcasting).

                    // Behavioral flags for diagnostic purposes
                    periodic = 0x0100,  ///< Endpoint generates periodic data.
                    realtime = 0x0200,  ///< Time-critical, must not block (e.g., high-speed camera).

                    /**@todo The behavioral flags for diagnostic purposes might be extended in the future. They are mainly
                             intended for use with the IO monitor that observes and analyzes endpoint statistics which should
                             use this information to generate better diagnostics for the user. */
                };

                cap() = default;
                cap(const cap &) = default;
                cap &operator=(const cap &) = default;
                cap(cap &&) = default;
                cap &operator=(cap &&) = default;
                ~cap() = default;

                cap(const flag_type (&flgs)[2]) noexcept : flags{flgs[0], flgs[1]} {}
                cap(flag_type flgs_source, flag_type flgs_sink) noexcept : flags{flgs_source, flgs_sink} {}

                flag_type flags[2]{};  ///< Capability flags for source (index 0) and sink (index 1).

                /** Convert capability flags to endpoint type.
                 * @returns The endpoint type based on the capability flags.
                 * */
                operator type() const noexcept;

                /** Access capability flags by index.
                 * @param idx The index of the capability flags (0 for source, 1 for sink).
                 * @returns A reference to the capability flags at the specified index.
                 * @see endpoint::index::sink, endpoint::index::source
                 * */
                ///@{
                flag_type &operator[](size_type idx) noexcept {
                    return flags[idx & 1];
                }

                flag_type operator[](size_type idx) const noexcept {
                    return flags[idx & 1];
                }
                ///@}

                /** Check sink capability bits.
                 * @param bit The capability bit(s) to check.
                 * @returns @c true if the endpoint has the specified sink capability, @c false otherwise.
                 * */
                bool getsink(cap::flag_type bit) const noexcept;

                /** Check source capability bits.
                 * @param bit The capability bit(s) to check.
                 * @returns @c true if the endpoint has the specified source capability, @c false otherwise.
                 * */
                bool getsource(cap::flag_type bit) const noexcept;

                /** Get capabilities setup.
                 * @returns The capabilities setup in JSON format.
                 * */
                settings::json setup() const;

            private:
                // Internal helper for setup() above.
                settings::json setup(flag_type flgs) const;
            };

            /** Endpoint statistics.
             * Endpoints should update these statitics during data transmission,
             * except for `stamp`, which is set at `start()`.
             *
             * @internal
             * The I/O API provides an `io::monitor` that reads the stats and generates
             * overall and current speed measurements using this data.
             * @endinternal
             * */
            struct VIDRE_CORE_VISIBILITY stats {
                struct {
                    size_type transmitted{};  ///< Total number of packets that were available for transmission.
                    size_type dropped{};      ///< Total number of packets dropped by this endpoint.
                } packets;

                struct {
                    size_type transmitted{};  ///< Total number of bytes that were available for transmission.
                } bytes;

                size_type errors{};  ///< Total number of transmission errors encountered by this endpoint.

                /** Start statistics.
                 * */
                void start() noexcept {
                    packets = {0, 0};
                    bytes = {0};
                    stamp = chrono::monotonic::now();
                }

                /** Update statistics with the given packet.
                 * @param pkt The packet to update statistics for.
                 * @param result The result of the I/O operation associated with the packet.
                 * */
                void update(shared<io::packet> pkt, const io::result &result) noexcept;

                /** Update statistics from IO result only.
                 * @param result The result of the I/O operation to update statistics for.
                 * */
                void update(const io::result &result) noexcept;

            protected:
                chrono::timer::time_point stamp = chrono::monotonic::now();  ///< Timestamp.

            private:
                friend class monitor;
            };

        public:
            endpoint() = default;

            endpoint(const endpoint &) = delete;
            endpoint &operator=(const endpoint &) = delete;
            endpoint(endpoint &&) = delete;
            endpoint &operator=(endpoint &&) = delete;

            virtual ~endpoint() = default;

        protected:
            std::string tag_{};  ///< Endpoint tag (used by manager).
            uint32_t stream_{};  ///< Endpoint ID (used by manager and storage::raw).

            stats stats_{};  ///< Endpoint statistics (e.g., bytes transmitted, errors, etc.).

            cap cap_{};  ///< Assigned endpoint capabilities.

        public:
            /** Push a packet to the sink.
             * @param p The packet to store.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual result push(shared<packet> p) {
                throw exception::internal() << "endpoint does not implement sink interface";
            }

            /** Pull a packet from the source.
             * @returns Shared pointer to the retrieved packet.
             * */
            virtual shared<packet> pull() {
                throw exception::internal() << "endpoint does not implement source interface";
            }

        public:
            /** Check if the endpoint is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Get endpoint tag.
             * @returns The string tag of this endpoint.
             * */
            const std::string tag() const noexcept {
#if defined(DEBUG)
                if (tag_.empty())
                    return debug::tag(this);
#endif
                return tag_;
            }

            /** Set endpoint tag.
             * @param tag The new tag for this endpoint.
             * */
            void settag(const std::string &tag) noexcept {
                tag_ = tag;
            }

            /** Get endpoint stream ID.
             * @returns The stream ID of this endpoint.
             * */
            uint32_t stream() const noexcept {
                return stream_;
            }

            /** Get capabilities of the endpoint.
             * @note This method returns the capabilities of the /unconnected/ endpoint.
             * */
            virtual cap getcap() const noexcept {
                return {};
            }

            /** Check sink capabilities.
             * @param bit The capability bit to check (default: any).
             * @returns @c true if the endpoint has the specified sink capability, @c false otherwise.
             * @note If `bit` is not specified, this method checks if the endpoint is a sink.
             * */
            bool issink(cap::flag_type bit = cap::any) const noexcept {
                return cap_.getsink(bit);
            }

            /** Check if object provides a source interface.
             * @param bit The capability bit to check (default: any).
             * @returns @c true if the endpoint has the specified source capability, @c false otherwise.
             * @note If `bit` is not specified, this method checks if the endpoint is a source.
             * */
            bool issource(cap::flag_type bit = cap::any) const noexcept {
                return cap_.getsource(bit);
            }

            /** Get endpoint statistics.
             * @returns The statistics of this endpoint.
             * */
            const stats &getstats() const noexcept {
                return stats_;
            }

            /** Reset endpoint statistics.
             * This method resets the statistics of this endpoint.
             * @internal
             * The `io::monitor` uses this method to reset the statistics before
             * starting a new monitoring interval. The endpoint is only expected
             * to update the public fields in `io::endpoint::stats`.
             * @endinternal
             * */
            void clearstats() noexcept {
                stats_ = {};
            }

            /** Get endpoint setup.
             * @returns JSON configuration object for this endpoint.
             * @todo need to define a standard format...
             *
             * @todo - describe, that this method is also used by, e.g., the device class
             * @todo - standardize the format
             * */
            virtual settings::json setup() const {
                return {};
            }

            /** Setup endpoint.
             * @param config JSON configuration object for this endpoint.
             *
             * @see `setup() const`
             * */
            virtual void setup(const settings::json &config) {}

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

            /** Unlink this endpoint.
             * This method disconnects the endpoint from any connected sources or sinks.
             * */
            virtual status unlink() = 0;

            /** Connect source to sink.
             * */
            static status connect(shared<endpoint::source> src, shared<endpoint::sink> snk);

            /** Disconnect an endpoint.
             * @param ep The endpoint to disconnect.
             * @returns The status of the disconnection operation.
             * @note This method is synonymous with calling `unlink()` on the endpoint.
             * */
            static status disconnect(shared<io::endpoint> ep) {
                return ep->unlink();
            }

            /** Get packet manifest.
             * FIXME (the "manifest" contains the streams configuration data)
             * */
            virtual shared<packet> manifest() noexcept {
                return {};  // FIXME
            }

        private:
            friend class manager;
        };

        ///@cond
        inline constexpr endpoint::cap::flag_type operator|(endpoint::cap::flag_type lhs, endpoint::cap::flag_type rhs) {
            return static_cast<endpoint::cap::flag_type>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline constexpr endpoint::cap::flag_type operator&(endpoint::cap::flag_type lhs, endpoint::cap::flag_type rhs) {
            return static_cast<endpoint::cap::flag_type>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline constexpr endpoint::cap::flag_type operator~(endpoint::cap::flag_type m) {
            return static_cast<endpoint::cap::flag_type>(~to_underlying(m));
        }
        inline constexpr bool operator!(endpoint::cap::flag_type m) {
            return m == endpoint::cap::flag_type::none;
        }

        inline constexpr endpoint::cap::flag_type &operator|=(endpoint::cap::flag_type &lhs, endpoint::cap::flag_type rhs) {
            lhs = static_cast<endpoint::cap::flag_type>(to_underlying(lhs) | to_underlying(rhs));
            return lhs;
        }
        inline constexpr endpoint::cap::flag_type &operator&=(endpoint::cap::flag_type &lhs, endpoint::cap::flag_type rhs) {
            lhs = static_cast<endpoint::cap::flag_type>(to_underlying(lhs) & to_underlying(rhs));
            return lhs;
        }
        ///@endcond

        /** Source endpoint role.
         * */
        class VIDRE_CORE_VISIBILITY endpoint::source : public virtual endpoint {
        public:
            static constexpr size_type index = 0;

        public:
            source() = default;

            source(const source &) = delete;
            source &operator=(const source &) = delete;
            source(source &&) = delete;
            source &operator=(source &&) = delete;

            virtual ~source() = default;

        public:
            status unlink() override {
                return status::bad;
            }

        protected:
            virtual status attach(shared<endpoint::sink> snk, cap::flag_type cap) = 0;
            virtual status detach(const endpoint::sink *ptr) = 0;

        private:
            friend class endpoint;
            friend class sink;
            friend class io::sink;
        };

        /** Sink endpoint role.
         * */
        class VIDRE_CORE_VISIBILITY endpoint::sink : public virtual endpoint {
        public:
            static constexpr size_type index = 1;

        public:
            sink() = default;

            sink(const sink &) = delete;
            sink &operator=(const sink &) = delete;
            sink(sink &&) = delete;
            sink &operator=(sink &&) = delete;

            virtual ~sink() = default;

        public:
            status unlink() override {
                return status::bad;
            }

        protected:
            virtual status attach(shared<endpoint::source> src, cap::flag_type cap) = 0;
            virtual status detach(const endpoint::source *ptr) = 0;

        private:
            friend class endpoint;
            friend class source;
            friend class io::source;
        };

        ///@cond
        inline endpoint::cap::operator type() const noexcept {
            uint16_t retval = 0;
            retval |= to_underlying(type::sink) * !!static_cast<bool>(flags[sink::index]);
            retval |= to_underlying(type::source) * !!static_cast<bool>(flags[source::index]);
            retval |= to_underlying(type::fanout) * !!static_cast<bool>(flags[sink::index] & flag_type::multi);
            retval |= to_underlying(type::fanin) * !!static_cast<bool>(flags[source::index] & flag_type::multi);
            return static_cast<type>(retval);
        }

        inline bool endpoint::cap::getsink(cap::flag_type bit) const noexcept {
            return !!static_cast<bool>(flags[endpoint::sink::index] & bit);
        }

        inline bool endpoint::cap::getsource(cap::flag_type bit) const noexcept {
            return !!static_cast<bool>(flags[endpoint::source::index] & bit);
        }

        ///@endcond

        ///@}

    }  // namespace io

}  // namespace vidre

#if defined(DEBUG)
namespace std {

    template <>
    struct formatter<vidre::io::endpoint::cap::flag_type> : std::formatter<std::string_view> {
        auto format(vidre::io::endpoint::cap::flag_type value, std::format_context &ctx) const {
            using cap = vidre::io::endpoint::cap;

            std::string name;

            if (value & cap::chunk)
                name += "chk|";
            if (value & cap::packet)
                name += "pkt|";
            if (name.empty())
                name = "none:";
            else
                name.end()[-1] = ':';

            if (value & cap::push)
                name += "push,";
            if (value & cap::pull)
                name += "pull,";
            if (value & cap::nonblock)
                name += "n";
            if (value & cap::multi)
                name += "m";
            if (value & cap::periodic)
                name += "p";
            if (value & cap::realtime)
                name += "r";

            if (!name.empty() && name.back() == ',')
                name.pop_back();

            return std::formatter<std::string_view>::format(name, ctx);
        }
    };

}  // namespace std
#else
namespace std {

    template <>
    struct formatter<vidre::io::endpoint::cap::flag_type>
        : formatter<std::underlying_type_t<vidre::io::endpoint::cap::flag_type> > {
        using type = vidre::io::endpoint::cap::flag_type;
        using underlying = std::underlying_type_t<vidre::io::endpoint::cap::flag_type>;

        auto format(type value, format_context &ctx) const {
            return formatter<underlying>::format(static_cast<underlying>(value), ctx);
        }
    };

}  // namespace std
#endif

#endif  // VIDRE_CORE_IO_TYPES_ENDPOINT_HH
