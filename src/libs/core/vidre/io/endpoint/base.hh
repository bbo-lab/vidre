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
#include <vidre/io/types.hh>

#include <format>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-endpoint
         * */
        ///@{

        /** Endpoint.
         *
         * This represents a generic endpoint in the I/O system, which can be a
         * source, sink, channel, fan-in, or fan-out. Also, any device that
         * represents a data producer or consumer within the I/O system.
         * can—and in fact is—represented as an endpoint.
         *
         * @todo example
         * */
        class VIDRE_CORE_VISIBILITY endpoint {
        public:
            // forward declarations
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

            /** Direction of data flow.
             * Represents the direction of data flow for an endpoint.
             * @note The direction is usually only used for informational and validation
             *       purposes, and otherwise not provided as quality of an endpoint.
             * */
            enum class dir : unsigned int {
                local = 0,           ///< No direction, just "endpoint local".
                source = 1,          ///< Direction towards the source.
                sink = 2,            ///< Direction towards the sink.
                any = source | sink  ///< Any direction, i.e. both towards source and sink.
            };

            /** Endpoint type.
             * @note The provided types are only for informational (and possibly visual) purposes and
             *       are always computed from the capabilities and configuration of the endpoint.
             * */
            enum class type {
                source = 0x01,            ///< Endpoint is a source (i.e., it can produce data).
                sink = 0x02,              ///< Endpoint is a sink (i.e., it can consume data).
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

                flag_type source{};  ///< Source capability flags.
                flag_type sink{};    ///< Sink capability flags.

                /** Convert capability flags to endpoint type.
                 * @returns The endpoint type based on the capability flags.
                 * */
                operator endpoint::type() const noexcept;

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
             *
             * @todo Once we have a monitoring system in place, this might require reconsideration...
             * */
            struct VIDRE_CORE_VISIBILITY stats {
                struct {
                    size_type stashed{};
                    size_type committed{};
                    size_type errors{};

                    size_type pending() const noexcept {
                        /*

                        chatGPT:
                            Yes, that makes the semantics considerably cleaner:

                            ```cpp
                            struct packet {
                                uint64_t stashed{};
                                uint64_t committed{};
                                uint64_t errors{};
                            };
                            ```

                            Then you don't have to overload `stashed - committed` with two meanings. In particular, you can define a
                        useful accounting relationship such as:

                            ```text
                            stashed = committed + errors + pending
                            ```

                            and therefore:

                            ```cpp
                            pending = stashed - committed - errors
                            ```

                            assuming every failed packet increments `errors` exactly once and is no longer pending.

                            That also gives the monitor three immediately understandable quantities: **accepted work, successfully
                        completed work, failed work**. I like this more than trying to encode failure indirectly in the other two
                        counters.


                        @todo need to rethink - for a device, if the camera fails to capture a frame, how should that be reflected
                        in the statistics? Do we have to increment stashed and errors, or is there a better way to account for
                        failed captures? Also, there's an "errors" field a level higher (see below)
                        */
                        return stashed - committed - errors;
                    }
                } packets;

                struct {
                    // @todo see above, maybe here, an additional field "dropped" is enough?
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

            /** Connect source to sink.
             * */
            static status connect(shared<endpoint::source> src, shared<endpoint::sink> snk);

            /** Disconnect an endpoint.
             * @param ep The endpoint to disconnect.
             * @returns The status of the disconnection operation.
             *
             * @note This method is synonymous with calling `unlink()` on the endpoint.
             *
             * */
            static status disconnect(shared<io::endpoint> ep) {
                return ep->unlink();
            }

        protected:
            /** Helper for calling detach on source endpoints.
             * */
            static status detach(shared<endpoint::source> src, const endpoint::sink *snk);

            /** Helper for calling detach on sink endpoints.
             * */
            static status detach(shared<endpoint::sink> snk, const endpoint::source *src);

        public:
            endpoint() = default;

            endpoint(const endpoint &) = delete;
            endpoint &operator=(const endpoint &) = delete;
            endpoint(endpoint &&) = delete;
            endpoint &operator=(endpoint &&) = delete;

            virtual ~endpoint() = default;

        protected:
            std::string tag_{};  ///< Endpoint tag (should be a unique identifier describing the endpoint, e.g. "camera").
            uint32_t stream_{};  ///< Endpoint stream ID (FIXME clarify usage).

            cap cap_{};  ///< Assigned endpoint capabilities.

            stats stats_{};  ///< Endpoint statistics (e.g., bytes transmitted, errors, etc.).

        public:
            /** Push a packet to the sink (if endpoint implements the sink role).
             *
             * @note The return value is only indicative of success or failure of
             *       local delivery (fans, for example, will only indicate if queueing
             *       the packet succeeded, which is no indication of actual delivery).
             *
             * @param p The packet to store.
             * @returns Status of the write operation.
             * @see endpoint::sink
             * */
            virtual status put(shared<packet> p) {
                throw exception::internal() << "endpoint does not implement sink interface";
            }

            /** Pull a packet from the source (if endpoint implements the source role).
             *
             * @note The return value is only indicative of success or failure of
             *       local retrieval (fans, for example, will only indicate if a packet
             *       is available for pulling, which is no indication of actual delivery).
             *
             * @returns Shared pointer to the retrieved packet (nullptr if no packet is available).
             * @see endpoint::source
             * */
            virtual shared<packet> get() {
                throw exception::internal() << "endpoint does not implement source interface";
            }

        public:
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

            /** Get implementation capabilities of the endpoint.
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
                return cap_.sink & bit;
            }

            /** Check if object provides a source interface.
             * @param bit The capability bit to check (default: any).
             * @returns @c true if the endpoint has the specified source capability, @c false otherwise.
             * @note If `bit` is not specified, this method checks if the endpoint is a source.
             * */
            bool issource(cap::flag_type bit = cap::any) const noexcept {
                return cap_.source & bit;
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

            /** Endpoint is ready for data transmission.
             *
             * Readiness indicates whether data can be transmitted in the specified
             * direction. For regular I/O endpoints this generally requires the relevant
             * connections to be established and the connected endpoints to be ready as
             * well. Derived implementations may impose additional requirements; devices,
             * for example, may become ready only after acquisition has been started.
             *
             * @param direction Direction for which readiness is queried.
             * @returns @c true if the endpoint is ready in the specified direction,
             *          @c false otherwise.
             * */
            virtual bool isready(dir direction = dir::any) const noexcept {
                return false;
            }

            /** Unlink this endpoint.
             *
             * This method disconnects the endpoint from any connected sources and/or sinks. 
             * With the exception of `io::channel`, that reconnects the source with the sink,
             * multi-point endpoints will leave connected sources and sinks dangling!
             *
             * @returns Status of the unlink operation. `status::ok` if successful, `status::bad` otherwise.
             * */
            virtual status unlink() = 0;

            /** Get packet manifest.
             * @todo the "manifest" contains the streams or rather the devices configuration data
             * */
            virtual shared<packet> manifest() noexcept {
                return {};  // FIXME
            }

        private:
            friend class manager;
        };

        ///@cond
        inline constexpr endpoint::dir operator|(endpoint::dir lhs, endpoint::dir rhs) {
            return static_cast<endpoint::dir>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline constexpr endpoint::dir operator&(endpoint::dir lhs, endpoint::dir rhs) {
            return static_cast<endpoint::dir>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline constexpr endpoint::dir operator~(endpoint::dir m) {
            return static_cast<endpoint::dir>(~to_underlying(m));
        }
        inline constexpr bool operator!(endpoint::dir m) {
            return m == endpoint::dir::local;
        }

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
         * This class serves as the base for endpoints that act as data sources
         * in the I/O framework. Adopters need to provide the `attach()` and `detach()`
         * methods to manage connections with sinks.
         * */
        class VIDRE_CORE_VISIBILITY endpoint::source : public virtual endpoint {
        public:
            /** Type-trait to check if a type represents an endpoint::source role.
             * */
            template <typename T>
            static constexpr bool isa_v = std::derived_from<T, endpoint::source>;

        public:
            source() = default;

            source(const source &) = delete;
            source &operator=(const source &) = delete;
            source(source &&) = delete;
            source &operator=(source &&) = delete;

            virtual ~source() = default;

        protected:
            /** Attach a sink to this source.
             * @param snk The sink to connect to.
             * @param cap The capabilities of the connection.
             * @returns `status::ok` on success, `status::bad` on failure (e.g. when already connected)
             *
             * @note This method is called from io::connect (or endpoint::connect), which initially matches and selects the
             *       source and sink capabilities. This attach method then sets up the actual connection. If additional
             *       connection logic is required, the method can be overridden.
             * */
            virtual status attach(shared<endpoint::sink> snk, cap::flag_type cap) = 0;

            /** Detach a sink from this source.
             * @param ptr The sink to detach.
             * @returns `status::ok` on success (including when already disconnected),
             *          `status::bad` on failure (e.g. when still active).
             * */
            virtual status detach(const endpoint::sink *ptr) = 0;

            /** Get (new) branch for a fan-out connection.
             * @returns The branch endpoint for the fan-out connection.
             * */
            virtual shared<endpoint::source> branch_source() {
                throw exception::internal() << "branch_source() must be overridden by fan-out sources";
            }

        private:
            friend class endpoint;
            friend class endpoint::sink;
        };

        /** Sink endpoint role.
         * @see `endpoint::source`
         * */
        class VIDRE_CORE_VISIBILITY endpoint::sink : public virtual endpoint {
        public:
            /** Type-trait to check if a type represents an endpoint::sink role.
             * */
            template <typename T>
            static constexpr bool isa_v = std::derived_from<T, endpoint::sink>;

        public:
            sink() = default;

            sink(const sink &) = delete;
            sink &operator=(const sink &) = delete;
            sink(sink &&) = delete;
            sink &operator=(sink &&) = delete;

            virtual ~sink() = default;

        protected:
            /** Attach a source to this sink.
             * @param src The source to connect to.
             * @param cap The capabilities of the connection.
             * @returns `status::ok` on success, `status::bad` on failure (e.g. when already connected)
             *
             * @note This method is called from io::connect (or endpoint::connect), which initially matches and selects the
             *       source and sink capabilities. This attach method then sets up the actual connection. If additional
             *       connection logic is required, the method can be overridden.
             * */
            virtual status attach(shared<endpoint::source> src, cap::flag_type cap) = 0;

            /** Detach a source from this sink.
             * @param ptr The source to detach.
             * @returns `status::ok` on success (including when already disconnected),
             *          `status::bad` on failure (e.g. when still active).
             * */
            virtual status detach(const endpoint::source *ptr) = 0;

            /** Get (new) branch for a fan-in connection.
             * @returns The branch endpoint for the fan-in connection.
             * */
            virtual shared<endpoint::sink> branch_sink() {
                throw exception::internal() << "branch_sink() must be overridden by fan-in sinks";
            }

        private:
            friend class endpoint;
            friend class endpoint::source;
        };

        ///@cond
        inline endpoint::cap::operator endpoint::type() const noexcept {
            uint16_t retval = 0;
            retval |= to_underlying(type::sink) * !!static_cast<bool>(sink);
            retval |= to_underlying(type::source) * !!static_cast<bool>(source);
            retval |= to_underlying(type::fanout) * !!static_cast<bool>(sink & flag_type::multi);
            retval |= to_underlying(type::fanin) * !!static_cast<bool>(source & flag_type::multi);
            return static_cast<type>(retval);
        }

        inline status endpoint::detach(shared<endpoint::source> src, const endpoint::sink *snk) {
            return src->detach(snk);
        }

        inline status endpoint::detach(shared<endpoint::sink> snk, const endpoint::source *src) {
            return snk->detach(src);
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
