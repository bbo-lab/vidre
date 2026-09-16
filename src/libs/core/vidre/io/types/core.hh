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
#ifndef VIDRE_CORE_IO_TYPES_CORE_HH
#define VIDRE_CORE_IO_TYPES_CORE_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/cpp/shared.hh>
#include <vidre/memory/base.hh>
#include <vidre/error.hh>
#include <vidre/sync/events.hh>
#include <vidre/util/uuid.hh>
#include <vidre/util/chrono.hh>
#include <vidre/thread.hh>

#include <bit>

#include <cstring>
#include <cassert>

namespace vidre {

    namespace io {

        // forward declarations
        class packet;
        class sink;
        class source;

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
             * @todo remove once proper buffer is implemented
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

        /** Dynamic size indicator.
         * This constant represents a dynamic or unspecified size, typically used
         * to indicate that the size of a buffer or container is not fixed at compile time.
         * */
        static constexpr size_type dynamic = size_type(-1);

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

        /** Cache for IO operations.
         * This cache is intended for use with synchronous IO operations, especially when
         * dealing with storage devices such as SSDs. The reason not to rely on the caching
         * behavior of the operating system is that this approach can provide more predictable
         * behavior (e.g., it avoids swap usage).
         * */
        class VIDRE_CORE_VISIBILITY cache {
        public:
            using size_type = memory::size_type;
            using value_type = memory::value_type;
            using pointer = memory::pointer;
            using const_pointer = memory::const_pointer;

        public:
            cache(size_type size = VIDRE_CACHE_SIZE, size_type n = VIDRE_CACHE_SLOTS) {
                allocate(size, n);
            }

            cache(const cache &) = delete;
            cache &operator=(const cache &) = delete;
            cache(cache &&) = delete;
            cache &operator=(cache &&) = delete;

            ~cache() {
                if (data_)
                    std::free(data_);
            }

        protected:
            size_type slots_{};  ///< Number of cache slots.
            size_type size_{};   ///< Size of each cache slot.

            event event_{};  ///< Event for signaling buffer availability.

            std::atomic<size_type> in_{};   ///< Current read position in the cache.
            std::atomic<size_type> out_{};  ///< Current write position in the cache.
            memory::pointer data_{};        ///< Pointer to the allocated cache memory.

        protected:
            /** Notify "slot ready" condition.
             * */
            void ready() noexcept {
                event_.notify();
            }

        public:
            /** Allocate buffer(s).
             * @param size Size of each buffer.
             * @param n Number of buffers (default is VIDRE_CACHE_SLOTS).
             * @throws exception::memory If memory allocation fails.
             *
             * @note @a size and @a n are rounded up to the next power of two if they are not already.
             * @note Using this method while the buffer is not empty leads to undefined behavior.
             *
             * This method initializes the buffer with the specified size and number of buffers.
             * Its configuration may change, for example, in the setup method of a class managing the buffer.
             * */
            void allocate(size_type size, size_type n = VIDRE_CACHE_SLOTS);

            /** Get cache size.
             * */
            size_type size() const noexcept {
                return slots_ * size_;
            }

            /** Get number of bytes stored.
             * */
            size_type count() const noexcept {
                return (in_ >= out_) ? (in_ - out_) : (size_ - (out_ - in_));
            }

            /** Get fill ratio.
             * */
            size_type fill_ratio() const noexcept {
                auto sz = size();
                auto cn = count();

                auto b = std::bit_width(sz) - 1;
                if (b < 32) {
                    sz <<= (32 - b);
                    cn <<= (32 - b);
                }
                else if (b > 32) {
                    sz >>= (b - 32);
                    cn >>= (b - 32);
                }

                return (1000 * cn) / sz;
            }

            /** Write data to buffer.
             * @param buf Memory area containing the data to write.
             * @param timeout Maximum time to wait for the buffer to become available.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            size_type put(const memory::view &v,
                          std::chrono::milliseconds timeout = std::chrono::milliseconds(VIDRE_CACHE_TIMEOUT_MS));

            template <typename T>
            requires (not std::derived_from<T, memory::view> && std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>)
            size_type put(const T &v, std::chrono::milliseconds timeout = std::chrono::milliseconds(VIDRE_CACHE_TIMEOUT_MS)) {
                static_assert(!std::is_pointer_v<T>, "put() does not support pointer types");
                return put({sizeof(T), reinterpret_cast<const memory::const_pointer>(&v)}, timeout);
            }

            template <typename T, size_t N>
            requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
            size_type put(const T (&arr)[N],
                          std::chrono::milliseconds timeout = std::chrono::milliseconds(VIDRE_CACHE_TIMEOUT_MS)) {
                static_assert(!std::is_pointer_v<T>, "put() does not support arrays of pointer types");
                return put({sizeof(T) * N, reinterpret_cast<const memory::const_pointer>(arr)}, timeout);
            }

        public:
            /** Wait for "slot" ready signal.
             * This method blocks until the next slot of data is ready for processing.
             * */
            auto wait() {
                return event_.wait();
            }

            /** Number of bytes available (in the current, readable slot).
             * */
            size_type available() const noexcept {
                size_type diff = in_ - out_;
                return std::min(diff, size_);
            }

            /** Get memory slot from the buffer.
             * @returns A memory view representing a (not necessarily full slot) currently available in the buffer.
             * */
            memory::view get() const noexcept {
                return {available(), data_ + out_};
            }

            /** Flush a memory slot.
             * This method marks the current memory slot as processed and advances the read position.
             * */
            void flush() noexcept {
                out_ += size_;
                if (out_ >= (size_ * slots_))
                    out_ = 0;
            }
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_TYPES_CORE_HH
