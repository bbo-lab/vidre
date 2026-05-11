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
#ifndef VIDRE_CORE_IO_TYPES_BASIC_HH
#define VIDRE_CORE_IO_TYPES_BASIC_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/memory/base.hh>
#include <vidre/exception.hh>
#include <vidre/util/uuid.hh>
#include <vidre/util/events.hh>
#include <vidre/util/chrono.hh>
#include <vidre/settings/json.hh>
#include <vidre/thread.hh>

#include <bit>
#include <utility>

#include <cerrno>
#include <cstring>
#include <cassert>

namespace vidre {

    namespace io {

        namespace detail {

            /**@addtogroup api-core-io-details
             * */
            ///@{

            /** Block buffer.
             * This simple buffer is mainly intended for use with sinks only,
             * especially when using synchronous file writes. There, writing
             * data not being a multiple of the underlying hardware's block size,
             * can be orders of magnitude slower.
             *
             * @see io::file::sink
             * */
            struct VIDRE_CORE_VISIBILITY buffer {
                using size_type = memory::size_type;
                using value_type = memory::value_type;
                using pointer = memory::pointer;
                using const_pointer = memory::const_pointer;

                buffer() = default;

                buffer(const buffer &) = delete;
                buffer &operator=(const buffer &) = delete;
                buffer(buffer &&) = default;
                buffer &operator=(buffer &&) = default;

                buffer(size_type n) noexcept {
                    setup(n);
                }

                ~buffer() {
                    if (data)
                        std::free(data);
                }

                size_type size{};  ///< The overall buffer size.
                size_type used{};  ///< Number of used bytes.
                pointer data{};    ///< Memory pointer.

                /** Setup buffer.
                 * @param len Number of bytes (rounded to the next power of 2).
                 * @return Number of bytes allocated, or 0 if it was already allocated
                 *         previously or no memory could be allocated.
                 *
                 * @note This method may only be called once.
                 * */
                size_type setup(size_type n) noexcept {
                    if (data)
                        return 0;

                    n = std::bit_ceil(n);
                    data = reinterpret_cast<pointer>(std::aligned_alloc(8, n));
                    if (data)
                        size = n;

                    return size;
                }

                /** Check if buffer is "usable".
                 * */
                operator bool() const noexcept {
                    return nullptr != data;
                }

                /** Check if buffer is empty.
                 * */
                bool empty() const noexcept {
                    return used == 0;
                }

                /** Check if buffer is full.
                 * */
                bool full() const noexcept {
                    return used == size;
                }

                /** "Flush" data.
                 * @returns Number of bytes "flushed".
                 * This routine merely sets the field @c used to zero.
                 * */
                size_type flush() noexcept {
                    size_type retval = used;
                    used = 0;
                    return retval;
                }

                /** Put data into buffer.
                 * @param src Pointer to source data.
                 * @param n Number of bytes to buffer.
                 * @returns Number of bytes actually buffered.
                 * */
                size_type put(const_pointer src, size_type n) noexcept {
                    assert(data != nullptr);

                    size_type space = size - used;
                    size_type done = std::min(space, n);
                    std::memcpy(data + used, src, done);
                    used += done;
                    return done;
                }

                /** Dress as memory area.
                 * */
                operator memory::area() const noexcept {
                    return {size, data};
                }
            };

            ///@}

        }  // namespace detail

        namespace packet {
            struct basic;  // forward declaration
        }

        class sink;     // forward declaration
        class source;   // forward declaration
        class channel;  // forward declaration
        class arbiter;  // forward declaration
        class merger;   // forward declaration

        class route;      // forward declaration
        class dashboard;  // forward declaration

        class monitor;  // forward declaration

        /**@addtogroup api-core-io-types
         * */
        ///@{

        /** Unsigned default size type.
         * */
        using size_type = std::size_t;

        /** Signed default offset type.
         *
         * Note that this type is also used as return type for read/write
         * operations, where a negative value (specifically @c invalid) indicates
         * an error.
         *
         * @see io::invalid
         * */
        using offset_type = std::make_signed_t<size_type>;

        /** Invalid offset value.
         * Typical usage:
         * @code
         * if (write(x,y,z) == io::invalid<io::offset_type>) {
         *     // handle error
         * }
         * @endcode
         * @see offset_type
         * */
        template <class T = offset_type>
        requires std::is_integral_v<T>
        inline constexpr T invalid = static_cast<T>(-1);

        /** IO task (thread).
         *
         * This (base) class provides an extension of the standard `std::thread` with some
         * additional convenience methods, such as setting scheduling policies.
         *
         * */
        class VIDRE_CORE_VISIBILITY task {
        public:
            /** Scoped task state.
             *
             * This helper struct is intended to be used within task handlers to
             * manage the task's running state and to provide a convenient method
             * for checking termination requests.
             *
             * @note This scope can be also used in non-threaded context.
             *
             * Typical usage:
             * @code
             * void handler(task::shared self, task::shared receiver) {
             *     task::scope thr(self.get());
             *     while (not self->terminated()) {
             *         // do work
             *     }
             * }
             * @endcode
             *
             * */
            struct VIDRE_CORE_VISIBILITY scope {
                scope() = delete;

                /** Constructor signalling running state.
                 * @param t Pointer to the associated task.
                 * */
                scope(task *t) : thread_(t) {
                    if (thread_)
                        thread_->running_ = true;
                }

                /** Destructor ending running state.
                 * */
                ~scope() {
                    if (thread_)
                        thread_->running_ = false;
                }

                /** Check if thread should terminate.
                 * */
                bool isterminated() const noexcept {
                    return thread_ && thread_->terminate_;
                }

            protected:
                task *thread_{};  ///< Pointer to the associated thread, or @c nullptr if not associated.
            };

            /** Shared thread.
             * */
            using shared = std::shared_ptr<task>;

            template <typename... Args>
            static shared make(Args &&...args) {
                return std::make_shared<task>(std::forward<Args>(args)...);
            }

        public:
            task() noexcept = default;
            task(const task &) = delete;
            task &operator=(const task &) = delete;
            task(task &&other) noexcept = default;
            task &operator=(task &&other) noexcept = default;
            virtual ~task() = default;

            template <class F, class... Args>
            explicit task(F &&f, Args &&...args) : impl_(std::forward<F>(f), std::forward<Args>(args)...) {}

        protected:
            std::thread impl_;        ///< The underlying thread implementation.
            bool running_ = false;    ///< Flag indicating whether the thread is running.
            bool terminate_ = false;  ///< Flag indicating whether the thread should terminate.

        public:
            /** Check if thread is running.
             * */
            bool isrunning() const noexcept {
                return running_;
            }

            /** Run the thread.
             * This method is the equivalent of joining the thread, i.e.,
             * it waits for the thread to finish execution.
             * */
            virtual status run() noexcept {
                if (not impl_.joinable())
                    return status::bad;
                impl_.join();
                return status::good;
            }

            /** Detach the thread.
             * This method detaches the thread, allowing it to run independently.
             * */
            status detach() {
                if (not impl_.joinable())
                    return status::bad;
                impl_.detach();
                return status::good;
            }

            /** Signal the thread to terminate.
             * */
            virtual void terminate() noexcept {
                terminate_ = true;
                //@todo should we also delete ourselves here? (if detached)
            }

            /** Set scheduling priority.
             * @param priority Priority value as a float (0.0 to 1.0, means minimum to maximum priority).
             * @param policy Scheduling policy (default: FIFO).
             * @return Status of the operation.
             * */
            status priority(float priority = 0.5f, schedule::policy policy = schedule::policy::fifo) {
                return schedule(impl_).priority(priority, policy);
            }

            /** Set deadline scheduling.
             * @param period The period of the thread.
             * @param runtime The runtime of the thread.
             * @param deadline The deadline of the thread.
             * @return Status of the operation.
             * */
            status set_deadline(std::chrono::nanoseconds period,   //
                                std::chrono::nanoseconds runtime,  //
                                std::chrono::nanoseconds deadline) {
                return schedule(impl_).set_deadline(period, runtime, deadline);
            }
        };

        /** IO operation result.
         *
         * This struct encapsulates the result of low-level IO operations, including
         * whether an error occurred and how many bytes were processed.
         *
         * Typical usage:
         * @code
         * io::result res;
         * res += sink.write(bufA, countA);
         * res += sink.write(bufB, countB);
         * return res;
         * @endcode
         *
         * */
        struct VIDRE_CORE_VISIBILITY result {
            bool error{};       ///< Flag indicating whether an error occurred.
            size_type count{};  ///< Number of bytes read/written, or @c invalid on failure.

            result() = default;
            result(const result &) = default;
            result &operator=(const result &) = default;
            result(result &&) = default;
            result &operator=(result &&) = default;

            result(offset_type n) noexcept : error(n == invalid<offset_type>), count(n * !error) {}

            /** Check if the operation was successful.
             * @returns @c true if the operation succeeded, @c false otherwise.
             * */
            operator bool() const noexcept {
                return !error;
            }

            /** Add another result to this result.
             * @param other The result to add.
             * @returns Reference to this result.
             *
             * This operator allows for easy accumulation of results, such as when
             * performing multiple read/write operations in a loop. If any operation
             * fails (i.e., has an error), the resulting error flag will be set to
             * @c true (accumulated), and the count will reflect the total number
             * of bytes successfully processed up to that point.
             * */
            result &operator+=(const result &other) noexcept {
                error = error || other.error;
                if (!error)
                    count += other.count;
                return *this;
            }

            /** Add the result of a read/write operation.
             * @param n The number of bytes read/written, or @c invalid on failure.
             * @returns Reference to this result.
             *
             * @see `operator+=(const result &other)`
             * */
            result &operator+=(offset_type n) noexcept {
                if (n == invalid<offset_type>)
                    error = true;
                else if (!error)
                    count += n;

                return *this;
            }
        };

        /** Endpoint capabilities.
         * @todo Add an "info" struct + method (e.g. transmitted byte count, buffer fill ratio, etc.).
         * */
        class VIDRE_CORE_VISIBILITY endpoint {
        public:
            /** Shared pointer type for endpoints.
             * This type is used for managing the lifetime of endpoint instances,
             * especially when they are shared across different components or threads.
             * */
            using shared = std::shared_ptr<endpoint>;

            /** Type-trait to check if a type is derived from endpoint.
             * */
            template <typename T>
            static constexpr bool isa_v = std::derived_from<T, endpoint>;

            /** Index for source.
             * @internal
             * When arrays are used for storing connections, capabilities, etc.,
             * this index can be used to access the source-related data.
             * @endinternal
             * */
            static constexpr size_type source = 0;
            /** Index for sink.
             * @see source
             * */
            static constexpr size_type sink = 1;

            /** Endpoint type.
             * */
            enum class type {
                sink = 0x01,               ///< Endpoint is a sink (i.e., it can consume data).
                source = 0x02,             ///< Endpoint is a source (i.e., it can produce data).
                channel = sink | source,   ///< Endpoint is a channel (i.e., it can both produce and consume data).
                merger = 0x04 | channel,   ///< Endpoint is a merger (i.e., it can combine multiple sources into a single sink).
                arbiter = 0x08 | channel,  ///< Endpoint is an arbiter (i.e., it can distribute a single source to multiple sinks).
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
                    any = 0xFFFF,  ///< Any mode or method (default).

                    packet = 0x0002,        ///< Packet mode (see `io::packet`).
                    chunk = 0x0001,         ///< Chunk mode (plain, binary data transmission).
                    both = chunk | packet,  ///< Both modes.
                    multi = 0x0004,         ///< Endpoint can be connected to multiple other endpoints (e.g., for broadcasting).

                    task = 0x0010,      ///< Endpoint requires a thread (e.g., for live video capture).
                    realtime = 0x0020,  ///< Endpoint requires real-time scheduling (e.g., for audio output).

                    push = 0x0100,   ///< Source directly puts data to sink.
                    pull = 0x0200,   ///< Sink directly gets data from source.
                    event = 0x0400,  ///< Endpoint uses event-based transmission (e.g., for packet mode).
                };

                cap() = default;
                cap(const cap &) = default;
                cap &operator=(const cap &) = default;
                cap(cap &&) = default;
                cap &operator=(cap &&) = default;
                ~cap() = default;

                cap(const flag_type (&flgs)[2]) noexcept : flags{flgs[0], flgs[1]} {}
                cap(flag_type flgs_source, flag_type flgs_sink) noexcept : flags{flgs_sink, flgs_source} {}

                flag_type flags[2]{};  ///< Capability flags for source (index 0) and sink (index 1).

                /** Convert capability flags to endpoint type.
                 * @returns The endpoint type based on the capability flags.
                 * */
                operator type() const noexcept {
                    uint16_t retval = 0;
                    retval |= to_underlying(type::sink) * !!static_cast<bool>(flags[sink]);
                    retval |= to_underlying(type::source) * !!static_cast<bool>(flags[source]);
                    retval |= to_underlying(type::merger) * !!static_cast<bool>(flags[sink] & flag_type::multi);
                    retval |= to_underlying(type::arbiter) * !!static_cast<bool>(flags[source] & flag_type::multi);
                    return static_cast<type>(retval);
                }

                /** Access capability flags by index.
                 * @param idx The index of the capability flags (0 for source, 1 for sink).
                 * @returns A reference to the capability flags at the specified index.
                 * @see endpoint::sink, endpoint::source
                 * */
                ///@{
                flag_type &operator[](size_type idx) noexcept {
                    return flags[idx & 1];
                }

                flag_type operator[](size_type idx) const noexcept {
                    return flags[idx & 1];
                }
                ///@}

                /** Get capabilities setup.
                 * @returns The capabilities setup in JSON format.
                 * */
                settings::json setup() const;

            private:
                // Internal helper for setup() above.
                settings::json setup(flag_type flgs) const;
            };

            /** Endpoint linkage.
             * */
            struct VIDRE_CORE_VISIBILITY linkage {
                shared data[2]{};  ///< Connected endpoints (0 for source, 1 for sink).

                /** Access connected endpoints by index.
                 * @param idx The index of the connected endpoint (0 for source, 1 for sink).
                 * @returns A reference to the connected endpoint at the specified index.
                 * @see endpoint::sink, endpoint::source
                 * */
                ///@{
                shared &operator[](size_type idx) noexcept {
                    return data[idx & 1];
                }

                shared operator[](size_type idx) const noexcept {
                    return data[idx & 1];
                }
                ///@}
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

            protected:
                chrono::timer::time_point stamp = chrono::monotonic::now();  ///< Timestamp.

            private:
                friend class monitor;
            };

        protected:
            /** Match capabilities between two endpoint types.
             * @param from The source endpoint type.
             * @param to The sink endpoint type.
             * @returns @c true if the capabilities are compatible, @c false otherwise.
             * */
            static bool match(cap::flag_type &from, cap::flag_type &to) noexcept;

        public:
            /** Match capabilities between two endpoint types.
             * @tparam From The source endpoint type.
             * @tparam To The sink endpoint type.
             * @param from The source endpoint instance.
             * @param to The sink endpoint instance.
             * @returns @c true if the capabilities are compatible, @c false otherwise.
             * */
            template <class From, class To>
            requires (isa_v<From> && isa_v<To>)
            static bool match(From *from, To *to) {
                if (not from || not to)
                    throw exception::logic() << "invalid endpoint(s)";

                // reset cap_ before matching
                from->cap_[endpoint::source] = from->getcap()[endpoint::source];
                to->cap_[endpoint::sink] = to->getcap()[endpoint::sink];

                return match(from->cap_[endpoint::source], to->cap_[endpoint::sink]);
            }

            /** Match capabilities between two endpoint types.
             * @tparam From The source endpoint type.
             * @tparam To The sink endpoint type.
             * @param from The source endpoint instance.
             * @param to The sink endpoint instance.
             * @returns @c true if the capabilities are compatible, @c false otherwise.
             * */
            template <class From, class To>
            requires (isa_v<From> && isa_v<To>)
            static bool match(std::shared_ptr<From> from, std::shared_ptr<To> to) {
                return match(from.get(), to.get());
            }

        public:
            endpoint() = default;
            endpoint(const endpoint &) = delete;
            endpoint &operator=(const endpoint &) = delete;
            endpoint(endpoint &&) = default;
            endpoint &operator=(endpoint &&) = default;

            virtual ~endpoint() = default;

        protected:
            std::string tag_{};  ///< Endpoint tag (used by dashboard).
            uint32_t stream_{};  ///< Endpoint ID (used by dashboard and storage::raw).

            stats stats_{};  ///< Endpoint statistics (e.g., bytes transmitted, errors, etc.).

            cap cap_{};        ///< Matched endpoint capabilities.
            shared parent_{};  ///< Parent endpoint (if applicable).

        public:
            /** Check if the endpoint is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Get endpoint tag.
             * @returns The tag of this endpoint.
             * */
            const std::string &tag() const noexcept {
                return tag_;
            }

            /** Get endpoint stream ID.
             * @returns The stream ID of this endpoint.
             * */
            uint32_t stream() const noexcept {
                return stream_;
            }

            /** Get endpoint capabilities.
             * */
            virtual cap getcap() const noexcept = 0;

            /** Check sink capabilities.
             * @param bit The capability bit to check (default: any).
             * @returns @c true if the endpoint has the specified sink capability, @c false otherwise.
             * */
            bool issink(cap::flag_type bit = cap::any) const noexcept {
                return !!static_cast<bool>(cap_.flags[endpoint::sink] & bit);
            }

            /** Check if object provides a source interface.
             * @param bit The capability bit to check (default: any).
             * @returns @c true if the endpoint has the specified source capability, @c false otherwise.
             * */
            bool issource(cap::flag_type bit = cap::any) const noexcept {
                return !!static_cast<bool>(cap_.flags[endpoint::source] & bit);
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
             * */
            virtual settings::json setup() const {
                return {};
            }

            /** Setup endpoint.
             * @param config JSON configuration object for this endpoint.
             * */
            virtual void setup(const settings::json &config) {}

            /** Get packet header.
             * FIXME
             * */
            virtual packet::basic *header() noexcept {
                return nullptr;
            }

            /** Find all essential sources.
             *  */
            virtual std::vector<endpoint *> sources() noexcept;

        protected:
            virtual bool push(shared) {
                throw exception::logic() << "endpoint does not support push transmission";
            }
            virtual bool pull() {
                throw exception::logic() << "endpoint does not support pull transmission";
            }
            virtual bool step() {
                throw exception::logic() << "endpoint does not support events";
            }

        private:
            friend class route;
            friend class dashboard;
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

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_TYPES_BASIC_HH
