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
 * focus: video images, see @ref api-core-video-format) as well as modes of
 * transmission (from fixed to variable sized data packets, as well as raw I/O).
 * FIXME
 *
 * ## Overview
 *
 * FIXME
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/memory/base.hh>
#include <vidre/exception.hh>
#include <vidre/util/uuid.hh>
#include <vidre/util/events.hh>
#include <vidre/settings/json.hh>

#include <ankerl/unordered_dense.h>

#include <bit>
#include <utility>
#include <filesystem>

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
                size_type append(const_pointer src, size_type n) noexcept {
                    assert(data != nullptr);

                    size_type space = size - used;
                    size_type done = std::min(space, n);
                    std::memcpy(data + used, src, done);
                    used += done;
                    return done;
                }
            };

            ///@}

        }  // namespace detail

    }  // namespace io

    namespace io {

        namespace packet {
            struct base;  // forward declaration
        }

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
         * if (write(x,y,z) == io::invalid) {
         *     // handle error
         * }
         * @endcode
         * @see offset_type
         * */
        template <class T = offset_type>
        requires std::is_integral_v<T>
        inline constexpr T invalid = static_cast<T>(-1);

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

        /** Stream mode.
         * */
        struct VIDRE_CORE_VISIBILITY mode {
            enum type : int {
                any,     ///< Any mode (default).
                stream,  ///< Stream mode (e.g., for files or network sockets).
                packet   ///< Packet mode (e.g., for video frames or audio samples).
            };
        };

        /** Business state.
         *
         * This class, eventually only used by the `sink`, provides a simple mechanism
         * for signaling when a component is "busy" (e.g., processing data) and when
         * it is ready to accept new data.
         *
         * The consumer (sink) can call `busy::clear()` to signal that it is ready for new data,
         * and the producer (source) can call `busy::wait()` to wait until the consumer is ready.
         * This can be useful for flow control, especially when dealing with components
         * that have variable processing times or when implementing backpressure mechanisms.
         *
         * Note, however, that producers (especially isochronous ones) may ignore the busy state
         * and continue producing data, which may lead to data loss if the consumer cannot keep up
         * (which is, in fact, intended behavior).
         *
         * */
        class VIDRE_CORE_VISIBILITY busy {
        public:
            class scope;

        public:
            busy() = default;

        protected:
            event ev_{};  ///< Event for signaling when the busy state is cleared.

        protected:
            /** Clear busy state.
             * */
            void done() {
                ev_.notify();
            }

        public:
            /** Wait for busy state to be cleared.
             * @return @c event::result containing the status and the notification count.
             * */
            [[nodiscard]] event::result wait() {
                return ev_.wait();
            }

            /** Wait for busy state to be cleared (with timeout).
             * @param timeout The maximum time to wait for the busy state to be cleared.
             * @return @c event::result containing the status and the notification count.
             * */
            [[nodiscard]] event::result wait(std::chrono::milliseconds timeout) {
                return ev_.wait(timeout);
            }
        };

        /** Busy scope.
         * */
        class VIDRE_CORE_VISIBILITY busy::scope {
        public:
            scope(busy &b) : busy_(b) {}
            scope(busy *b) : busy_(*b) {}
            ~scope() {
                busy_.done();
            }

        protected:
            busy &busy_;
        };

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
        class VIDRE_CORE_VISIBILITY sink : public busy {
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
            /** The low-level write method (if applicable).
             * @param buf Memory view containing the data to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual offset_type write(const memory::view &buf) noexcept {
                return invalid<offset_type>;
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

            template <typename T>
            requires (not std::derived_from<T, memory::view>)
            result put(const T &v) {
                return put({sizeof(T), reinterpret_cast<const memory::const_pointer>(&v)});
            }

            /** Write a packet.
             * @param p Pointer to the packet to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual result put(const packet::base *p) {
                busy::scope inprogress(this);
                return invalid<offset_type>;
            }

        private:
        };

        /** Data source.
         *
         * Data sources provide data to be read, such as files or filter outputs. They can
         * be either stream- or packet-based (see @c source::mode).
         *
         * @see source::shared
         * */
        class VIDRE_CORE_VISIBILITY source {
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
            event ev_{};  ///< Event for signaling data availability.

        protected:
            /** Notify, that @a n packets are available.
             * */
            event::state notify(size_type n = 1) {
                return ev_.notify(n);
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

            /** Retrieve the mode of the source.
             * @returns The mode of the source (stream or packet).
             * */
            virtual operator mode::type() const noexcept {
                return mode::stream;
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

            /** Wait (indefinitely) for data to become available.
             * @returns True if data is available, false if the wait was interrupted.
             * @see vidre::event
             * */
            [[nodiscard]] event::result wait() {
                return ev_.wait();
            }

            /** Wait for data to become available, with a timeout.
             * @param timeout The maximum time to wait#include <vidre/cpp/util.hh> for data (milliseconds).
             * @returns True if data is available, false if the wait timed out or was interrupted.
             * @see vidre::event
             * */
            [[nodiscard]] event::result wait(std::chrono::milliseconds timeout) {
                return ev_.wait(timeout);
            }

            /** Retrieve a packet from the source.
             * @returns Pointer to the retrieved packet, or nullptr if no packet is available.
             * @throw exception::logic if source is packet based
             * @throw exception::runtime for other errors
             * */
            virtual const packet::base *get() {
                throw exception::logic();
            }

            // FIXME needs a way to connect to the next sink or pipe...

        private:
        };

        /** Connect a sink to a source.
         * */
        // std::thread::id connect(source::shared &src, sink::shared &snk) noexcept {
        //     // FIXME


        //     return std::this_thread::get_id();
        // }
#if 0
        /** Data pipe.
         *
         * A data pipe is a combination of a source and a sink, allowing data to flow
         * from one component to another. It can be used to connect different parts
         * of a processing pipeline.
         *
         * @see source::shared
         * */
        class VIDRE_CORE_VISIBILITY pipe
            : public virtual sink
            , public virtual source {
        public:
            /** Shared pointer type for pipe.
             * This type is used for managing the lifetime of pipe instances,
             * especially when they are shared across different components or threads.
             * @see pipe::make
             * */
            using shared = std::shared_ptr<pipe>;

            /** Create pipe as shared pointer.
             * @tparam T Type of the pipe to create.
             * @param args Arguments to forward to the pipe's constructor.
             * @returns A shared pointer to the newly created pipe.
             * @see pipe::shared
             * */
            template <typename T, typename... Args>
            static shared make(Args &&...args) {
                return std::make_shared<T>(std::forward<Args>(args)...);
            }

        public:
            pipe() = default;

            pipe(const pipe &) = delete;
            pipe &operator=(const pipe &) = delete;
            pipe(pipe &&) = default;
            pipe &operator=(pipe &&) = default;

            virtual ~pipe() {}

        public:
            /** Configure pipe.
             * @param config JSON configuration object.
             * @throw exception::logic if the configuration is invalid for this pipe.
             * @note The configuration format is pipe-specific and may be ignored by some pipes.
             * */
            void setup(const settings::json &config) override {}

            // FIXME
        };
#endif

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_TYPES_HH
