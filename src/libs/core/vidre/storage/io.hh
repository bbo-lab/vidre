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

#include <bit>
#include <filesystem>

#include <fcntl.h>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

namespace vidre {

    namespace detail {

        namespace file {

            /**@addtogroup api-core-details
             * */
            ///@{

            using descriptor = int;

#if defined(_WIN32)
            enum flag_type : int {
                readonly = _O_RDONLY,
                writeonly = _O_WRONLY,
                readwrite = _O_RDWR,

                append = _O_APPEND,
                create = _O_CREAT,
                exclusive = _O_EXCL,
                truncate = _O_TRUNC,
                non_block = _O_NONBLOCK,
                synchronous = _O_SYNC,
                // FIXME...
            };

            inline descriptor open(const char *filename, int flags) {
                return ::_open(filename, flags);
            }

            inline descriptor open(const char *filename, int flags, mode_t mode) {
                return ::_open(filename, flags, mode);
            }

            inline void close(descriptor fd) {
                ::_close(fd);
            }

            inline std::ssize_t read(descriptor fd, uint8_t *buf, std::size_t count) {
                return ::_read(fd, buf, count);
            }

            inline std::ssize_t write(descriptor fd, const uint8_t *buf, std::size_t count) {
                return ::_write(fd, buf, count);
            }

            static constexpr std::size_t path_max = MAX_PATH;
#else
            enum flag_type : int {
                readonly = O_RDONLY,
                writeonly = O_WRONLY,
                readwrite = O_RDWR,

                append = O_APPEND,
                create = O_CREAT,
                exclusive = O_EXCL,
                truncate = O_TRUNC,
                non_block = O_NONBLOCK,
                synchronous = O_SYNC,
                // FIXME...
            };

            inline descriptor open(const char *filename, int flags) {
                return ::open(filename, flags);
            }

            inline descriptor open(const char *filename, int flags, mode_t mode) {
                return ::open(filename, flags, mode);
            }

            inline void close(descriptor fd) {
                ::close(fd);
            }

            inline std::ptrdiff_t read(descriptor fd, uint8_t *buf, std::size_t count) {
                return ::read(fd, buf, count);
            }

            inline std::ptrdiff_t read(descriptor fd, char *buf, std::size_t count) {
                return ::read(fd, buf, count);
            }

            inline std::ptrdiff_t write(descriptor fd, const uint8_t *buf, std::size_t count) {
                return ::write(fd, buf, count);
            }

            inline std::ptrdiff_t write(descriptor fd, const char *buf, std::size_t count) {
                return ::write(fd, buf, count);
            }

            static constexpr std::size_t path_max = PATH_MAX;

            
#endif

            ///@}

        }  // namespace file

    }  // namespace detail

    namespace storage {

        /**@addtogroup api-core-storage-io
         * */
        ///@{

        /** In-/output "direction".
         * */
        enum class mode : uint8_t {
            none = 0,    ///< Undefined state.
            input = 1,   ///< Input direction (is a sink).
            output = 2,  ///< Output direction (is a source).
            bidi = 3     ///< Bi-directional.
        };

        inline mode operator|(mode lhs, mode rhs) {
            return static_cast<mode>(to_underlying(lhs) | to_underlying(rhs));
        }

        inline mode operator&(mode lhs, mode rhs) {
            return static_cast<mode>(to_underlying(lhs) & to_underlying(rhs));
        }

        inline mode operator~(mode flags) {
            return static_cast<mode>(~to_underlying(flags));
        }

        inline bool operator!(mode flags) {
            return !to_underlying(flags);
        }

        /** Trivial file IO.
         * */
        class VIDRE_CORE_VISIBILITY fileio {
        public:
            /** Maximum length of a path.
             * */
            static constexpr std::size_t path_max = detail::file::path_max;

            /** File flags.
             * @see open()
             * */
            enum flag_type : int {
                readonly = detail::file::flag_type::readonly,
                writeonly = detail::file::flag_type::writeonly,
                readwrite = detail::file::flag_type::readwrite,
                append = detail::file::flag_type::append,
                create = detail::file::flag_type::create,
                exclusive = detail::file::flag_type::exclusive,
                truncate = detail::file::flag_type::truncate,
                non_block = detail::file::flag_type::non_block,
                synchronous = detail::file::flag_type::synchronous,
                // FIXME...
            };

            using perms = std::filesystem::perms;

            static constexpr perms default_perms = perms::owner_read | perms::owner_write | perms::group_read | perms::others_read;

        public:
            fileio() = default;

        protected:
            detail::file::descriptor fd_{};

        public:
            /** Check if valid.
             * */
            bool isvalid() const noexcept {
                return fd_ >= 0;
            }

            /** Open file.
             * @param filename The file name to open.
             * @param flag     File flags.
             * @param permission File read/write permissions (POSIX style).
             * @return @c true on success
             * @return @c false otherwise
             */
            bool open(const std::filesystem::path &filename, flag_type flags = readonly,
                      perms permissions = default_perms) noexcept {
                fd_ = detail::file::open(filename.c_str(), flags, static_cast<mode_t>(permissions));
                return isvalid();
            }

            /** Create a temporary file.
             * */
            bool temporary(std::string &filename) {
                fd_ = mkstemp(filename.data());  // per standard, data is zero-terminated
                return isvalid();
            }
            bool temporary() {
                std::string filename = "vidre-tmp-XXXXXX";
                // FIXME unlink!!!
                return temporary(filename);
            }

            /** Close the file.
             * */
            void close() noexcept {
                detail::file::close(fd_);
                fd_ = -1;
            }

            /** Write data to file.
             * */
            std::ptrdiff_t write(const uint8_t *buf, std::size_t count) noexcept {
                return detail::file::write(fd_, buf, count);
            }

            /** Read data from file.
             * */
            std::ptrdiff_t read(uint8_t *buf, std::size_t count) noexcept {
                return detail::file::read(fd_, buf, count);
            }
        };

        ///@cond
        inline fileio::flag_type operator|(fileio::flag_type lhs, fileio::flag_type rhs) {
            return static_cast<fileio::flag_type>(to_underlying(lhs) | to_underlying(rhs));
        }
        ///@endcond

        // ================================================================================================================
        // ================================================================================================================
        // ================================================================================================================
        // ================================================================================================================

        /** Data "volume".
         * */
        template <std::endian E = std::endian::native>
        struct volume;

        template <>
        struct volume<std::endian::little> {
            std::size_t size : bits_of<std::size_t>::value - 2 {};
            std::size_t managed : 1 {};
            std::size_t dynamic : 1 {};

            volume() = default;
            volume(std::size_t n) : size(n) {}

            operator std::size_t() const noexcept {
                return size;
            }
        };

        template <>
        struct volume<std::endian::big> {
            std::size_t dynamic : 1 {};
            std::size_t managed : 1 {};
            std::size_t size : bits_of<std::size_t>::value - 1 {};

            volume() = default;
            volume(std::size_t n) : size(n) {}

            operator std::size_t() const noexcept {
                return size;
            }
        };

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

        /** Data view.
         * */
        class VIDRE_CORE_VISIBILITY view {
        public:
            using size_type = std::size_t;
            using pointer = uint8_t *;
            using const_pointer = const uint8_t *;

        public:
            view() = default;
            view(view &&other) = default;
            view(const view &other) = default;

            view(std::size_t n, pointer ptr) noexcept : size_(n), data_(ptr) {}
            view(std::size_t n, const_pointer ptr) noexcept : size_(n), data_(const_cast<pointer>(ptr)) {}

            template <typename T>
            view(std::size_t n, const T *ptr) noexcept : size_(n), data_(const_cast<pointer>(ptr)) {}

        protected:
            volume<> size_{};
            pointer data_{};

        public:
            /** Get size of stored data.
             * */
            size_type size() const noexcept {
                return size_;
            }

            /** Get pointer to data.
             * */
            const_pointer data() const noexcept {
                return data_;
            }

            operator void *() {
                if (not size_.managed)
                    throw "FIXME access violation";
                return static_cast<void *>(data_);
            }

            constexpr operator const void *() const noexcept {
                return static_cast<const void *>(data_);
            }

            operator uint8_t *() {
                if (not size_.managed)
                    throw "FIXME access violation";
                return data_;
            }

            constexpr operator const uint8_t *() const noexcept {
                return data_;
            }
        };

        /** Data sink.
         * */
        class VIDRE_CORE_VISIBILITY sink {
        public:
            using size_type = std::size_t;

        public:
            sink() = default;

        public:
            virtual mode dir() const noexcept {
                return mode::input;
            }

            virtual size_type put(const view &) = 0;

            template <typename T>
            size_type put(const T &data) {
                return put(view{sizeof(T), &data});
            }

            template <typename T>
            size_type put(const T *data, size_type nmemb) {
                return put(view{nmemb * sizeof(T), data});
            }
        };

        /** Data source.
         */
        class VIDRE_CORE_VISIBILITY source {
        public:
            source() = default;

        public:
            virtual mode dir() const noexcept {
                return mode::input;
            }
        };

        ///@}

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_IO_HH
