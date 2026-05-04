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
            /** Capabilitie list.
             * */
            enum class cap : uint16_t {
                none = 0,

                stream = 0x0001,        ///< Stream mode (e.g., for files or network sockets).
                packet = 0x0002,        ///< Packet mode (e.g., for video frames or audio samples).
                any = stream | packet,  ///< Any mode (default).

                task = 0x0010,      ///< Endpoint requires a thread (e.g., for live video capture).
                realtime = 0x0020,  ///< Endpoint requires real-time scheduling (e.g., for audio output).

                push = 0x0100,  ///< Source pushes data to sink.
                pull = 0x0200,  ///< Sink pulls data from source.
            };

            template <typename T>
            static constexpr bool isa_v = std::derived_from<T, endpoint>;

            /** Match capabilities between two endpoint types.
             * @param from The source endpoint type.
             * @param to The sink endpoint type.
             * @returns @c true if the capabilities are compatible, @c false otherwise.
             * */
            static constexpr bool match(cap &from, cap &to) noexcept;

            /** Match capabilities between two endpoint types.
             * @tparam From The source endpoint type.
             * @tparam To The sink endpoint type.
             * @param from The source endpoint instance.
             * @param to The sink endpoint instance.
             * @returns @c true if the capabilities are compatible, @c false otherwise.
             * */
            template <class From, class To>
            requires (isa_v<From> && isa_v<To>)
            static constexpr bool match(From *from, To *to) {
                if (not from || not to)
                    throw exception::logic() << "invalid endpoint(s)";

                return match(from->cap_, to->cap_);
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
            endpoint(cap c = cap::none) : cap_(c) {}
            virtual ~endpoint() = default;

        protected:
            // std::string name_{};  ///< Endpoint name (used by dashboard).
            cap cap_{};           ///< Endpoint capabilities.

        public:
            // /** Get endpoint name.
            //  * @returns The name of this endpoint.
            //  * */
            // const std::string &name() const noexcept {
            //     return name_;
            // }

            // /** Set enpoint name.
            //  * @param name The name to set for this endpoint.
            //  * */
            // void name(const std::string &nm) noexcept {
            //     name_ = nm;
            // }

            /** Get capabilities.
             * @returns The capabilities of this endpoint.
             * */
            cap getcap() const noexcept {
                return cap_;
            }

            /** Set capability bit.
             * @param cap The capability bit to set.
             * */
            void setcap(cap c) noexcept {
                cap_ = static_cast<cap>(to_underlying(cap_) | to_underlying(c));
            }

            /** Clear capability bit.
             * @param cap The capability bit to clear (default: all bits).
             * */
            void clearcap(cap c = static_cast<cap>(-1)) noexcept {
                cap_ = static_cast<cap>(to_underlying(cap_) & ~to_underlying(c));
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

        private:
            friend class route;
            friend class dashboard;
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

        inline constexpr bool endpoint::match(endpoint::cap &from, endpoint::cap &to) noexcept {
            auto common = from & to;

            if ((common & endpoint::cap::any) == endpoint::cap::none)
                return false;

            // select stream/packet mode
            if (!!((common & endpoint::cap::any) & endpoint::cap::packet))
                common = endpoint::cap::packet;  // packet mode
            else
                common = endpoint::cap::stream;  // stream mode

            from &= endpoint::cap::any;
            from |= common;
            to &= endpoint::cap::any;
            to |= common;

            // push/pull mode
            if (!!(from & endpoint::cap::push))
                to &= ~endpoint::cap::pull;
            else if (!!(to & endpoint::cap::pull))
                from &= ~endpoint::cap::push;

            return true;
        }
        ///@endcond

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_TYPES_BASIC_HH
