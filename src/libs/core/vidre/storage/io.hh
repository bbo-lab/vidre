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
#ifndef VIDRE_CORE_STORAGE_IO_HH
#define VIDRE_CORE_STORAGE_IO_HH
#pragma once

/**@defgroup api-core-storage-io In-/output.
 * @ingroup api-core-storage
 * @fileinfo
 * @details
 *
 * In principle, the functionality provided by this module resembles the
 * class POSIX read/write. To provide advanced capabilities, such as
 * memory mapping, the interface, however, provides FIXME
 *
 * @note Even though filed under "storage", the I/O-functionality also applies
 *       to the concept of "filters".
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/exception.hh>

#include <bit>
#include <utility>
#include <filesystem>

#include <fcntl.h>
#if VIDRE_HOST_WINDOWS
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#endif

#include <cerrno>
#include <cstring>
#include <cassert>

namespace vidre {

    namespace detail {

        /**@addtogroup api-core-details
         * */
        ///@{

        /** Check if type has a "data()" method.
         * */
        template <typename T>
        concept has_data_member = requires (T t) {
            { t.data() } -> std::convertible_to<const void *>;
        };

        /** Check if type has a "data()" method.
         * */
        template <typename T>
        concept has_size_method = requires (T t) {
            { t.size() } -> std::convertible_to<std::size_t>;
        };

        ///@}

    }  // namespace detail

    namespace storage {

        /**@addtogroup api-core-storage-io
         * */
        ///@{

        /** Data extent.
         * The extent of data describes the byte-size, and as well,
         * carries flags for dynamic (re-)allocation and writability
         * ("non-constness").
         * @see view
         * */
        template <std::endian E = std::endian::native>
        struct extent;

        template <>
        struct extent<std::endian::little> {
            std::size_t size : bits_of<std::size_t>::value - 2 {};
            std::size_t writable : 1 {};
            std::size_t dynamic : 1 {};

            constexpr extent() = default;
            constexpr extent(std::size_t n, bool writable_ = false, bool dynamic_ = false)
                : size(n), writable(writable_), dynamic(dynamic_) {}

            operator std::size_t() const noexcept {
                return size;
            }
        };

        template <>
        struct extent<std::endian::big> {
            std::size_t dynamic : 1 {};
            std::size_t writable : 1 {};
            std::size_t size : bits_of<std::size_t>::value - 1 {};

            constexpr extent() = default;
            constexpr extent(std::size_t n) : size(n) {}
            constexpr extent(std::size_t n, bool writable_ = false, bool dynamic_ = false)
                : dynamic(dynamic_), writable(writable_), size(n) {}

            operator std::size_t() const noexcept {
                return size;
            }
        };

        /** Data view.
         * */
        class VIDRE_CORE_VISIBILITY view {
        public:
            using size_type = std::size_t;
            using pointer = uint8_t *;
            using const_pointer = const uint8_t *;

        public:
            constexpr view() = default;
            constexpr view(view &&other) = default;
            constexpr view &operator=(view &&other) = default;
            constexpr view(const view &other) = default;
            constexpr view &operator=(const view &other) = default;

            template <typename T>
            requires std::is_integral_v<T>
            view(std::size_t n, T *ptr) noexcept : size_(n, true), data_(reinterpret_cast<pointer>(const_cast<T *>(ptr))) {}

            template <typename T, std::size_t N>
            requires std::is_integral_v<T>
            constexpr view(T (&ptr)[N]) noexcept : size_(N, true), data_(reinterpret_cast<pointer>(ptr)) {}

            template <typename T>
            requires (std::is_integral_v<T> && std::is_unsigned_v<T>)
            constexpr view(std::size_t n, const T *ptr) noexcept : size_(n), data_(const_cast<pointer>(ptr)) {}

            template <typename T, std::size_t N>
            requires std::is_integral_v<T>
            constexpr view(T const (&ptr)[N]) noexcept : size_(N), data_(const_cast<pointer>(ptr)) {}

        protected:
            extent<> size_{};
            pointer data_{};

        public:
            /** Get size of stored data.
             * */
            constexpr size_type size() const noexcept {
                return size_;
            }

            /** Get pointer to data.
             * */
            constexpr const_pointer data() const noexcept {
                return data_;
            }

            constexpr operator const_pointer() const noexcept {
                return data_;
            }

            constexpr operator const void *() const noexcept {
                return static_cast<const void *>(data_);
            }

            constexpr operator pointer() {
                if (not size_.writable)
                    throw "invalid cast";
                return data_;
            }

            constexpr operator void *() {
                if (not size_.writable)
                    throw "invalid cast";
                return static_cast<void *>(data_);
            }
        };

        /** Write buffer.
         * This simple buffer is mainly intended for use with sinks only,
         * especially when using synchronous file writes. There, writing
         * data not begin a multiple of the underlying hardware's block size,
         * can be orders of magnitude slower.
         * */
        struct VIDRE_CORE_VISIBILITY buffer {
            using size_type = std::size_t;
            using value_type = uint8_t;
            using pointer = value_type *;
            using const_pointer = const value_type *;

            buffer() = default;
            ~buffer() {
                if (data)
                    std::free(data);
            }

            size_type size{};  ///< The overall buffer size.
            size_type used{};  ///< Number of used bytes.
            pointer data;      ///< Memory pointer.

            /** Setup buffer.
             * @param len Number of bytes (should be a power of 2).
             * @return Number of bytes allocated, or 0 if it was already allocated
             *         previously or no memory could be allocated.
             *
             * Allocate internal buffer with a size rounded to the next power
             * of two.
             *
             * @note This method may only be called once.
             * */
            size_type setup(size_type len) noexcept {
                if (data)
                    return 0;

                len = std::bit_ceil(len);
                data = reinterpret_cast<pointer>(std::aligned_alloc(8, len));
                if (data)
                    size = len;

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

        /** Data sink skeleton.
         * Derived classes need to implement at least:
         * @code
         * class VISIBILITY derived_sink {
         * public:
         *      virtual ~derived_sink();
         *
         * protected:
         *      offset_type write(const uint8_t *buf, size_type count) override;
         * };
         * @endcode
         * */
        class VIDRE_CORE_VISIBILITY sink {
        public:
            using size_type = std::size_t;
            using offset_type = std::make_signed_t<size_type>;

        public:
            sink() = default;
            virtual ~sink() {}

        protected:
            buffer buf_{};  ///< Internal block buffer.

        protected:
            /** Write buffer to disk.
             * @throw vidre::exception::memory
             * @returns number of bytes written to underlying hardware
             * */
            offset_type flush() {
                if (buf_.used) {
                    auto retval = write(buf_.data, buf_.used);

                    if (retval != buf_.used)
                        // if the underlying hardware can't take it, we assume,
                        // it's out of memory...
                        throw exception::memory();
                }
                return buf_.flush();
            }

            /** The actual write method.
             * This method needs overrides in derived classes.
             * */
            virtual offset_type write(const uint8_t *buf, size_type count) = 0;

        public:
            /** Check if the stream is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Transfer data.
             * */
            ///@{
            offset_type put(const uint8_t *buf, size_type count) {
                if (buf_) {
                    // using buffer
                    offset_type retval = count;
                    offset_type pos = 0;
                    while (count > 0) {
                        size_type buffered = buf_.append(buf + pos, count);
                        if (buf_.full())
                            flush();
                        count -= buffered;
                        pos += buffered;
                    }

                    return retval;
                }
                else
                    // direct write
                    return write(buf, count);
            }

            offset_type put(const view &v) {
                return put(v.data(), v.size());
            }

            template <typename T>
            offset_type put(const T &data) {
                return put(view{sizeof(T), &data});
            }

            template <typename T>
            offset_type put(const T *data, size_type nmemb) {
                return put(view{nmemb * sizeof(T), reinterpret_cast<const uint8_t *>(data)});
            }
            ///@}

            /** Set buffer.
             * @param size Buffer size (must be 2^n).
             * */
            size_type setbuf(size_type size) noexcept {
                return buf_.setup(size);
            }
        };

        /** Data source.
         */
        class VIDRE_CORE_VISIBILITY source {
        public:
            source() = default;

        public:
        };

        ///@}

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_IO_HH
