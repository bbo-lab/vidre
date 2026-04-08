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
#endif

#include <cerrno>
#include <cstring>

namespace vidre {

    namespace detail {

        namespace file {

            /**@addtogroup api-core-details
             * */
            ///@{

            using descriptor = int;
            using size_type = std::size_t;
            using offset_type = std::make_signed_t<size_type>;

#if VIDRE_HOST_WINDOWS
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

            inline offset_type read(descriptor fd, uint8_t *buf, std::size_t count) {
                return ::_read(fd, buf, count);
            }

            inline offset_type write(descriptor fd, const uint8_t *buf, std::size_t count) {
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
                close_on_exec = O_CLOEXEC,
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

            inline offset_type read(descriptor fd, uint8_t *buf, std::size_t count) {
                return ::read(fd, buf, count);
            }

            inline offset_type read(descriptor fd, char *buf, std::size_t count) {
                return ::read(fd, buf, count);
            }

            inline offset_type write(descriptor fd, const uint8_t *buf, std::size_t count) {
                return ::write(fd, buf, count);
            }

            inline offset_type write(descriptor fd, const char *buf, std::size_t count) {
                return ::write(fd, buf, count);
            }

            static constexpr std::size_t path_max = PATH_MAX;

#endif

            ///@}

        }  // namespace file

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
            using size_type = std::size_t;
            using offset_type = std::make_signed_t<size_type>;

            using descriptor = detail::file::descriptor;

        public:
            /** Maximum length of a path.
             * */
            static constexpr size_type path_max = detail::file::path_max;

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
                close_on_exec = detail::file::flag_type::close_on_exec,
                // FIXME...
            };

            using perms = std::filesystem::perms;

            static constexpr perms default_perms = perms::owner_read | perms::owner_write | perms::group_read | perms::others_read;

        public:
            fileio() = default;

            fileio(const fileio &) = delete;
            fileio(fileio &&other) noexcept : fd_(std::exchange(other.fd_, -1)) {}

            fileio(const char *filename, flag_type flags = readonly, perms permissions = default_perms) noexcept {
                open(filename, flags, permissions);
            }

            fileio(const std::string &filename, flag_type flags = readonly, perms permissions = default_perms) noexcept
                : fileio(filename.c_str(), flags, permissions) {}

            fileio(const std::filesystem::path &filename, flag_type flags = readonly, perms permissions = default_perms) noexcept
                : fileio(filename.c_str(), flags, permissions) {}

            ~fileio() {
                if (valid())
                    close();
            }

            void swap(fileio &other) noexcept {
                std::swap(fd_, other.fd_);
            }

        protected:
            descriptor fd_{};

        public:
            /** Check if valid.
             * */
            bool valid() const noexcept {
                return fd_ >= 0;
            }

            /** Convert to descriptor.
             * */
            operator descriptor() const noexcept {
                return fd_;
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
                return valid();
            }

            /** Create a temporary file.
             * */
            bool temporary(std::string &filename) {
                fd_ = mkstemp(filename.data());  // per standard, data is zero-terminated
                return valid();
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

            /** Reserve space on disk.
             * @todo needs implementation
             * */
            bool allocate(std::size_t len, ::off_t offset = ::off_t(-1));

            /** Write data to file.
             * */
            offset_type write(const uint8_t *buf, std::size_t count) noexcept {
                return detail::file::write(fd_, buf, count);
            }

            /** Read data from file.
             * */
            offset_type read(uint8_t *buf, std::size_t count) noexcept {
                return detail::file::read(fd_, buf, count);
            }
        };

        ///@cond
        inline fileio::flag_type operator|(fileio::flag_type lhs, fileio::flag_type rhs) {
            return static_cast<fileio::flag_type>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline fileio::flag_type operator|=(fileio::flag_type lhs, fileio::flag_type rhs) {
            return static_cast<fileio::flag_type>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline fileio::flag_type operator&(fileio::flag_type lhs, fileio::flag_type rhs) {
            return static_cast<fileio::flag_type>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline fileio::flag_type operator&=(fileio::flag_type lhs, fileio::flag_type rhs) {
            return static_cast<fileio::flag_type>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline fileio::flag_type operator~(fileio::flag_type rhs) {
            return static_cast<fileio::flag_type>(~to_underlying(rhs));
        }
        ///@endcond

        /** Data "volume".
         * The "volume" of data describes the byte-size, and as well,
         * carries flags for dynamic (re-)allocation and writability
         * ("non-constness").
         * @see view
         * */
        template <std::endian E = std::endian::native>
        struct volume;

        template <>
        struct volume<std::endian::little> {
            std::size_t size : bits_of<std::size_t>::value - 2 {};
            std::size_t writable : 1 {};
            std::size_t dynamic : 1 {};

            constexpr volume() = default;
            constexpr volume(std::size_t n, bool writable_ = false, bool dynamic_ = false)
                : size(n), writable(writable_), dynamic(dynamic_) {}

            operator std::size_t() const noexcept {
                return size;
            }
        };

        template <>
        struct volume<std::endian::big> {
            std::size_t dynamic : 1 {};
            std::size_t writable : 1 {};
            std::size_t size : bits_of<std::size_t>::value - 1 {};

            constexpr volume() = default;
            constexpr volume(std::size_t n) : size(n) {}
            constexpr volume(std::size_t n, bool writable_ = false, bool dynamic_ = false)
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
            volume<> size_{};
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

        /** Data sink.
         * */
        class VIDRE_CORE_VISIBILITY sink {
        public:
            using size_type = std::size_t;
            using offset_type = std::make_signed_t<size_type>;

            /** Flags for opening a "sink" if applicable.
             * */
            struct VIDRE_CORE_VISIBILITY flag {
                bool truncate = true;       ///< Truncate file if it exists (otherwise append).
                bool synchronous = true;    ///< Use synchronous IO with underlying file.
                size_type buffer_size = 0;  ///< Buffer-size.
            };

        public:
            sink() = default;
            virtual ~sink() noexcept {
                if (buf_) {
                    delete buf_;
                }
            }

        protected:
            flag flag_{};
            uint8_t *buf_{};
            size_type buflen_{};

        protected:
            offset_type flush() noexcept {
                offset_type n = buflen_;
                if (buflen_) {
                    auto retval = write(buf_, buflen_);
                    if (retval != n) {
                        abort_with_message("internal error: unhandled wirte return value...");
                    }

                    buflen_ = 0;
                    return retval;
                }
                return n;
            }

            virtual offset_type write(const uint8_t *buf, size_type count) noexcept = 0;

        public:
            virtual mode dir() const noexcept {
                return mode::input;
            }

            virtual bool valid() const noexcept {
                return false;
            }

            offset_type put(const uint8_t *buf, size_type count) noexcept {
                if (buf_) {
                    offset_type retval = count;

                    while (count > 0) {
                        // copy to buffer
                        auto space = flag_.buffer_size - buflen_;
                        space = std::min(space, count);
                        std::memcpy(buf_ + buflen_, buf, space);
                        buflen_ += space;

                        // write to disk
                        if (buflen_ == flag_.buffer_size) {
                            if (flush() != flag_.buffer_size) {
                                abort_with_message("internal error: unhandled wirte return value...");
                            }
                        }

                        count -= space;
                    }

                    return retval;
                }
                else
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
        };

        /** FIXME
         * */
        class VIDRE_CORE_VISIBILITY filesink : public virtual sink {
        public:
            using size_type = std::size_t;

        public:
            filesink() = default;
            virtual ~filesink() {
                flush();
            }

        protected:
            fileio fd_{};

        protected:
            offset_type write(const uint8_t *buf, size_type count) noexcept override {
                return fd_.write(buf, count);
            }

        public:
            mode dir() const noexcept override {
                return mode::output;
            }

            bool valid() const noexcept override {
                return fd_.valid();
            }

            /** Open file.
             * @param filename The file name to open.
             * @param flag     File flags.
             * @param permission File read/write permissions (POSIX style).
             * @return @c true on success
             * @return @c false otherwise
             */
            bool open(const std::filesystem::path &filename, sink::flag flags = {},
                      fileio::perms permissions = fileio::default_perms) noexcept {
                flag_ = flags;

                fileio::flag_type f = fileio::writeonly | fileio::create | fileio::close_on_exec;
                if (flag_.synchronous)
                    f |= fileio::synchronous;
                if (flag_.truncate)
                    f |= fileio::truncate;

                if (flag_.buffer_size)
                    buf_ = new uint8_t[flag_.buffer_size];

                return fd_.open(filename, f, permissions);
            }

            /** Close file.
             * */
            void close() noexcept {
                if (buf_)
                    flush();
                fd_.close();
                return;
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
