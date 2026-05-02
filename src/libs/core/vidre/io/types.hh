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
#ifndef VIDRE_CORE_IO_TYPES_HH
#define VIDRE_CORE_IO_TYPES_HH
#pragma once

/**@addtogroup api-core-io-types
 * @brief Basic types and streaming fundamentals.
 * @details
 * @fileinfo
 *
 * The data flow in VidRe has to cover various forms of data (such as its main
 * focus: video images, see @ref api-video) as well as modes of transmission
 * (from fixed to variable sized data packets, as well as raw I/O).
 *
 * ## Overview
 *
 * ### Endpoints and transmission
 *
 * The basic building blocks for data transmission are:
 * - **source**: a data producer (from input devices, such as video cameras, or data files).
 * - **sink**: a data consumer (from output devices, such as audio, video, or, as above, data files).
 * - **channel**: a channel exposing both, source and sink interface, allowing for, e.g., filtering data.
 * - **arbiter**: arbiters are responsible for distributing a single source to multiple sinks.
 * - **merger**: mergers are responsible for combining multiple sources into a single sink.
 *
 * ### Flow control
 *
 * FIXME see https://chat.deepseek.com/a/chat/s/8cc10342-f684-4a3e-805a-82c0b7973e4c
 *
 * @see @ref api-core-io-flow
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/memory/base.hh>
#include <vidre/exception.hh>
#include <vidre/util/uuid.hh>
#include <vidre/util/events.hh>
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

        /** IO thread.
         *
         * This (base) class provides an extension of the standard `std::thread` with some
         * additional convenience methods, such as setting scheduling policies.
         *
         * */
        class VIDRE_CORE_VISIBILITY thread {
        public:
            /** Scoped thread state.
             *
             * FIXME
             *
             * Typical usage:
             * @code
             * void handler(thread::shared self, thread::shared receiver) {
             *     thread::scope thr(self.get());
             *     while (not self->terminated()) {
             *         // do work
             *     }
             * }
             * @endcode
             *
             * */
            struct VIDRE_CORE_VISIBILITY scope {
                scope() = default;
                scope(thread *t) : thread_(t) {
                    if (thread_)
                        thread_->running_ = true;
                }

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
                thread *thread_{};  ///< Pointer to the associated thread, or @c nullptr if not associated.
            };

            /** Shared thread.
             * */
            struct shared : public std::shared_ptr<thread> {
                shared() = default;
                shared(const shared &) = default;
                shared &operator=(const shared &) = default;
                shared(shared &&) = default;
                shared &operator=(shared &&) = default;
                shared(std::nullptr_t) : std::shared_ptr<thread>(nullptr) {}
                template <typename T, typename... Args>
                shared(Args &&...args) : std::shared_ptr<thread>(std::make_shared<T>(std::forward<Args>(args)...)) {}
                ~shared() = default;

                void terminate() const noexcept {
                    if (auto t = get())
                        t->terminate();
                }
            };

            template <typename T, typename... Args>
            static shared make(Args &&...args) {
                return std::make_shared<T>(std::forward<Args>(args)...);
            }

        public:
            thread() noexcept = default;
            thread(const thread &) = delete;
            thread &operator=(const thread &) = delete;
            thread(thread &&other) noexcept = default;
            thread &operator=(thread &&other) noexcept = default;
            virtual ~thread() = default;

            template <class F, class... Args>
            explicit thread(F &&f, Args &&...args) : impl_(std::forward<F>(f), std::forward<Args>(args)...) {}

        protected:
            std::thread impl_;        ///< The underlying thread implementation.
            bool running_ = false;    ///< Flag indicating whether the thread is running.
            bool terminate_ = false;  ///< Flag indicating whether the thread should terminate.

        public:
            /** Check if thread is running.
             * */
            bool running() const noexcept {
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
         * */
        class VIDRE_CORE_VISIBILITY endpoint {
        public:
            /** Capabilitie list.
             * */
            enum cap : uint32_t {
                none = 0,

                stream = 0x0001,        ///< Stream mode (e.g., for files or network sockets).
                packet = 0x0002,        ///< Packet mode (e.g., for video frames or audio samples).
                any = stream | packet,  ///< Any mode (default).

                loop = 0x0010,      ///< Endpoint is thread-based (e.g., for live video capture).
                                    /// The endpoint produces data in a seperate thread, and data
                                    /// availability is signaled via events.
                ondemand = 0x0020,  ///< Endpoint is on-demand (e.g., for memory buffers).
                                    /// The endpoint produces data on demand, i.e., when the consumer requests it.

                // FIXME
            };

            template <typename T>
            static constexpr bool isa_v = std::derived_from<T, endpoint>;

            /** Match capabilities between two endpoint types.
             * @param from The source endpoint type.
             * @param to The sink endpoint type.
             * @returns The matched type, or @c type::invalid if no match is found.
             * */
            static constexpr cap match(cap from, cap to) noexcept;

            /** Match capabilities between two endpoint types.
             * @tparam From The source endpoint type.
             * @tparam To The sink endpoint type.
             * @param from The source endpoint instance.
             * @param to The sink endpoint instance.
             * @returns The matched type, or @c type::invalid if no match is found.
             * */
            template <class From, class To>
            requires (isa_v<From> && isa_v<To>)
            static constexpr cap match(From *from, To *to) noexcept {
                return match(from->output(), to->input());
            }

        public:
            endpoint() = default;
            virtual ~endpoint() = default;

        protected:
            thread::shared thread_;  ///< Optional thread for thread-based endpoints.

        public: // general
            /** Sink capabilities (if applicable).
             * */
            virtual cap input() const noexcept {
                return cap::none;
            }

            /** Source capabilities (if applicable).
             * */
            virtual cap output() const noexcept {
                return cap::none;
            }

        public: // thread-related
            /** Check if threaded.
             * */
            bool isthread() const noexcept {
                return thread_.get();
            }
        };

        ///@cond
        inline constexpr endpoint::cap operator|(endpoint::cap lhs, endpoint::cap rhs) {
            return static_cast<endpoint::cap>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline constexpr endpoint::cap operator&(endpoint::cap lhs, endpoint::cap rhs) {
            return static_cast<endpoint::cap>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline constexpr endpoint::cap operator~(endpoint::cap m) {
            return static_cast<endpoint::cap>(~to_underlying(m));
        }
        inline constexpr bool operator!(endpoint::cap m) {
            return m == endpoint::cap::none;
        }

        inline constexpr endpoint::cap &operator|=(endpoint::cap &lhs, endpoint::cap rhs) {
            lhs = static_cast<endpoint::cap>(to_underlying(lhs) | to_underlying(rhs));
            return lhs;
        }
        inline constexpr endpoint::cap &operator&=(endpoint::cap &lhs, endpoint::cap rhs) {
            lhs = static_cast<endpoint::cap>(to_underlying(lhs) & to_underlying(rhs));
            return lhs;
        }

        inline constexpr endpoint::cap match(endpoint::cap from, endpoint::cap to) noexcept {
            auto common = from & to;

            if ((common & endpoint::cap::any) == endpoint::cap::none)
                return endpoint::cap::none;
            else if ((common & endpoint::cap::any) == endpoint::cap::any)
                common &= ~endpoint::cap::stream;

            return common;
        }

        ///@endcond

        /** Data sink skeleton.
         *
         * The sink class provides a skeleton for data sinks, such as files or filter inputs.
         * It may work as a data stream or a memory region (see @c sink::mode).
         *
         * Derived classes need to implement at least:
         * @code
         * class VISIBILITY derived_sink : public virtual io::sink {
         * public:
         *      virtual ~derived_sink();
         *
         * protected:
         *      offset_type write(memory::const_pointer buf, size_type count) override;
         * };
         * @endcode
         *
         * Here, the method @c write() is the low-level write method. Classes
         * implementing buffering also need to implement the @c put() method:
         *
         * @code
         * offset_type put(memory::const_pointer buf, size_type count) override;
         * @endcode
         *
         * @see sink::shared
         * */
        class VIDRE_CORE_VISIBILITY sink : public virtual endpoint {
        public:
            /** Shared pointer type for sinks.
             * This type is used for managing the lifetime of sink instances,
             * especially when they are shared across different components or threads.
             * @see sink::make
             * */
            using shared = std::shared_ptr<sink>;

            /** Create sink as shared pointer.
             * @tparam T Type of the sink to create.
             * @param args Arguments to forward to the sink's constructor.
             * @returns A shared pointer to the newly created sink.
             * @see sink::shared
             * */
            template <typename T, typename... Args>
            static shared make(Args &&...args) {
                return std::make_shared<T>(std::forward<Args>(args)...);
            }

        public:
            sink() = default;

            sink(const sink &) = delete;
            sink &operator=(const sink &) = delete;
            sink(sink &&) = delete;
            sink &operator=(sink &&) = delete;

            virtual ~sink() {}

        protected:
            event ready_{};  ///< Event for signaling sink ready state.

        protected:
            /** The low-level write method (if applicable).
             * @param buf Memory view containing the data to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual offset_type write(const memory::view &buf) noexcept {
                return invalid<offset_type>;
            }

            /** Signal that the sink is ready.
             * This method can be called by derived classes to indicate that the sink is ready
             * to accept data.
             * */
            void ready() {
                ready_.notify();
            }

        public:
            /** Check if the stream is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Configure sink.
             * @param config JSON configuration object.
             * @throw exception::logic_error if the configuration is invalid for this sink.
             * @note The configuration format is sink-specific and may be ignored by some sinks.
             * */
            virtual void setup(const settings::json &config) {}

            /** Transfer data from a memory view.
             * @param v Memory view containing the data to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual result put(const memory::view &v) {
                return write(v);
            }

            /** Put a value of type @c T.
             * @tparam T Type of the value to write.
             * @param v Value to write (in binary form).
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            template <typename T>
            requires (not std::derived_from<T, memory::view>)
            result put(const T &v) {
                return put({sizeof(T), reinterpret_cast<const memory::const_pointer>(&v)});
            }

            /** Write a packet.
             * @param p Pointer to the packet to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual result put(const packet::basic *p);

            /** Wait for sink ready signal.
             * @returns True if the sink is ready, false if the wait was interrupted.
             * */
            [[nodiscard]] event::result wait() {
                return ready_.wait();
            }

            /** Wait for sink ready signal, with a timeout.
             * @param timeout The maximum time to wait for the sink to become ready (milliseconds).
             * @returns True if the sink is ready, false if the wait timed out or was interrupted.
             * */
            [[nodiscard]] event::result wait(std::chrono::milliseconds timeout) {
                return ready_.wait(timeout);
            }
        };

        /** Data source.
         *
         * Data sources provide data to be read, such as files or filter outputs. They can
         * be either stream- or packet-based (see @c source::mode).
         *
         * @see source::shared
         * */
        class VIDRE_CORE_VISIBILITY source : public virtual endpoint {
        public:
            /** Shared pointer type for source.
             * This type is used for managing the lifetime of source instances,
             * especially when they are shared across different components or threads.
             * @see source::make
             * */
            using shared = std::shared_ptr<source>;

            /** Create source as shared pointer.
             * @tparam T Type of the source to create.
             * @param args Arguments to forward to the source's constructor.
             * @returns A shared pointer to the newly created source.
             * @see source::shared
             * */
            template <typename T, typename... Args>
            static shared make(Args &&...args) {
                return std::make_shared<T>(std::forward<Args>(args)...);
            }

        public:
            source() = default;

            source(const source &) = delete;
            source &operator=(const source &) = delete;
            source(source &&) = delete;
            source &operator=(source &&) = delete;

            virtual ~source() {}

        protected:
            event avail_{};      ///< Event for signaling data availability.
            sink::shared link_;  ///< Linked sink (if any).

        protected:
            /** Notify, that @a n packets are available.
             * */
            event::state notify(size_type n = 1) {
                return avail_.notify(n);
            }

            /** The low-level read method.
             * @param buf Memory area to store the retrieved data.
             * @returns Number of bytes actually read (@c invalid for read failure).
             * */
            virtual offset_type read(memory::area buf) {
                return invalid<offset_type>;
            }

        public:
            /** Check if the stream is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Configure source.
             * @param config JSON configuration object.
             * @throw exception::logic if the configuration is invalid for this source.
             * @note The configuration format is source-specific and may be ignored by some sources.
             * */
            virtual void setup(const settings::json &config) {}

            /** Retrieve data from the source.
             * @param v Memory area to store the retrieved data.
             * @throw exception::logic if source is packet based.
             * @throw exception::runtime for other errors
             * @returns Number of bytes actually read (@c invalid for read failure).
             * */
            virtual result get(memory::area v) {
                return read(v);
            }

            /** Poll for data availability (waits indefinitely).
             * @returns True if data is available, false if the wait was interrupted.
             * @see vidre::event
             * */
            [[nodiscard]] event::result poll() {
                return avail_.wait();
            }

            /** Poll for data availability with timeout.
             * @param timeout The maximum time to wait for data (milliseconds).
             * @returns True if data is available, false if the wait timed out or was interrupted.
             * @see vidre::event
             * */
            [[nodiscard]] event::result poll(std::chrono::milliseconds timeout) {
                return avail_.wait(timeout);
            }

            /** Retrieve a packet from the source.
             * @returns Pointer to the retrieved packet, or nullptr if no packet is available.
             * @throw exception::logic if source is packet based
             * @throw exception::runtime for other errors
             * */
            virtual const packet::basic *get() {
                throw exception::logic();
            }

            /** Stream.
             * */
            virtual void stream(sink::shared snk, thread::shared thr = {}) {}

        private:
        };

        ///@todo channel
        ///@todo arbiter
        ///@todo merger

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_TYPES_HH
