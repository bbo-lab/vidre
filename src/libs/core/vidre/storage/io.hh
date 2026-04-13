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

        /** Unsigned default size type.
         * */
        using size_type = std::size_t;

        /** Signed default offset type.
         * */
        using offset_type = std::make_signed_t<size_type>;

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
            size_type size : bits_of<size_type>::value - 2 {};
            size_type writable : 1 {};
            size_type dynamic : 1 {};  // FIXME is this needed?

            constexpr extent() = default;
            constexpr extent(size_type n, bool writable_ = false, bool dynamic_ = false)
                : size(n), writable(writable_), dynamic(dynamic_) {}

            operator size_type() const noexcept {
                return size;
            }
        };

        template <>
        struct extent<std::endian::big> {
            size_type dynamic : 1 {};
            size_type writable : 1 {};
            size_type size : bits_of<size_type>::value - 1 {};

            constexpr extent() = default;
            constexpr extent(size_type n) : size(n) {}
            constexpr extent(size_type n, bool writable_ = false, bool dynamic_ = false)
                : dynamic(dynamic_), writable(writable_), size(n) {}

            operator size_type() const noexcept {
                return size;
            }
        };

        /** Data view.
         * */
        class VIDRE_CORE_VISIBILITY view {
        public:
            using size_type = storage::size_type;

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
            ///@{
            constexpr const_pointer data() const noexcept {
                return data_;
            }

            constexpr pointer data() {
                if (not size_.writable)
                    throw "invalid cast";
                return data_;
            }

            constexpr operator const_pointer() const noexcept {
                return data_;
            }

            constexpr operator pointer() {
                if (not size_.writable)
                    throw "invalid cast";
                return data_;
            }

            constexpr operator const void *() const noexcept {
                return static_cast<const void *>(data_);
            }

            constexpr operator void *() {
                if (not size_.writable)
                    throw "invalid cast";
                return static_cast<void *>(data_);
            }
            ///@}
        };

        /** Write buffer.
         * This simple buffer is mainly intended for use with sinks only,
         * especially when using synchronous file writes. There, writing
         * data not begin a multiple of the underlying hardware's block size,
         * can be orders of magnitude slower.
         * */
        struct VIDRE_CORE_VISIBILITY buffer {
            using size_type = storage::size_type;
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
            pointer data{};    ///< Memory pointer.

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
         *
         * The sink class provides a skeleton for data sinks, such as files or filter inputs.
         * It may work as a data stream or a memory region (see @c sink::mode).
         *
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
         *
         * Here, the method @c write() is the low-level write method. Classes
         * implementing buffering also need to implement the @c put() method:
         *
         * @code
         * offset_type put(const uint8_t *buf, size_type count) override;
         * @endcond
         *
         * Classes implementing the "region" mode need to implement the @c mode() method
         * and FIXME.
         * 
         * @todo document "shared"
         * @todo document "mode::region" interface
         * */
        class VIDRE_CORE_VISIBILITY sink {
        public:  
            using shared = std::shared_ptr<sink>;
            template <typename T, typename... Args>
            static shared make(Args &&...args) {
                return std::make_shared<T>(std::forward<Args>(args)...); 
            }

        public:
            /** Sink mode.
             * */
            enum mode_type {
                stream,  ///< Data is written as a stream of bytes.
                region   ///< Sink provides a memory region to write to (e.g., via memory mapping).
            };

        public:
            sink() = default;
            virtual ~sink() {}

        protected:
            /** The low-level write method.
             * */
            virtual offset_type write(const uint8_t *src, size_type count) = 0;

        public:
            /** Check if the stream is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Get sink mode.
             * */
            virtual mode_type mode() const noexcept {
                return mode_type::stream;
            }

            /** Transfer data.
             * */
            ///@{
            virtual offset_type put(size_type count, const uint8_t *src) {
                return write(src, count);
            }

            offset_type put(const view &v) {
                return put(v.size(), v.data());
            }

            template <typename T>
            offset_type put(const T &data) {
                return put(sizeof(T), &data);
            }

            template <typename T>
            offset_type put(const T *data, size_type nmemb) {
                return put(nmemb * sizeof(T), reinterpret_cast<const uint8_t *>(data));
            }
            ///@}

            // FIXME "mode::region" interface ... ???
        };

        /** Data source.
         * FIXME
         */
        class VIDRE_CORE_VISIBILITY source {
        public:  
            using shared = std::shared_ptr<source>;
            template <typename T, typename... Args>
            static shared make(Args &&...args) {
                return std::make_shared<T>(std::forward<Args>(args)...); 
            }

        public:
            /** Source mode.
             * */
            enum mode_type {
                stream,  ///< Data is read as a stream of bytes.
                region   ///< Source provides a memory region to read from (e.g., via memory mapping).
            };

        public:
            source() = default;

        protected:
            /** The low-level read method.
             * */
            virtual offset_type read(uint8_t *buf, size_type count) = 0;

        public:
            /** Check if the stream is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Get source mode.
             * */
            virtual mode_type mode() const noexcept {
                return mode_type::stream;
            }

            /** Transfer data.
             * */
            ///@{
            virtual offset_type get(size_type count, uint8_t *buf) {
                return read(buf, count);
            }

            offset_type get(view &v) {
                return get(v.size(), v.data());
            }

            template <typename T>
            offset_type get(T &data) {
                return get(sizeof(T), &data);
            }

            template <typename T>
            offset_type get(T *data, size_type nmemb) {
                return get(reinterpret_cast<uint8_t *>(data), nmemb * sizeof(T));
            }
            ///@}

            // FIXME "mode::region" interface ... ???
        };

        // FIXME filter? transformer? ... ?
        /* 
            Code suggestions from DeepSeek:

            file_source.read()
                .through(encryptor)
                .through(compressor)
                .into(file_sink);
        
            file_source | pipe(encryptor) | pipe(compressor) | file_sink;        
        
            Names suggested:
            - through
            - pipe
            - conduit
            - duplex
            ...
        */


        ///@}

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_IO_HH
