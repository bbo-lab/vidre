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

        namespace file {

            /**@addtogroup api-core-details
             * */
            ///@{

            using descriptor = int;
            using size_type = std::size_t;
            using offset_type = std::make_signed_t<size_type>;

            /** Default block size (if otherwise it cannot be retrieved from the OS).
             * */
#define VIDRE_STORAGE_BLOCKSIZE 4096

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

            enum whence_type : int {
                begin = SEEK_SET,
                current = SEEK_CUR,
                end = SEEK_END
            };

            inline descriptor open(const char *filename, int flags) noexcept {
                return ::open(filename, flags);
            }

            inline descriptor open(const char *filename, int flags, mode_t mode) noexcept {
                return ::open(filename, flags, mode);
            }

            inline void close(descriptor fd) noexcept {
                ::close(fd);
            }

            inline offset_type read(descriptor fd, uint8_t *buf, std::size_t count) noexcept {
                return ::read(fd, buf, count);
            }

            inline offset_type read(descriptor fd, char *buf, std::size_t count) noexcept {
                return ::read(fd, buf, count);
            }

            inline offset_type write(descriptor fd, const uint8_t *buf, std::size_t count) noexcept {
                return ::write(fd, buf, count);
            }

            inline offset_type write(descriptor fd, const char *buf, std::size_t count) noexcept {
                return ::write(fd, buf, count);
            }

            inline offset_type seek(descriptor fd, offset_type off, whence_type whence) noexcept {
                return ::lseek(fd, off, to_underlying(whence));
            }

            enum allocation_type : int {
                allocate_range = FALLOC_FL_ALLOCATE_RANGE,
                keep_size = FALLOC_FL_KEEP_SIZE,
                punch_hole = FALLOC_FL_PUNCH_HOLE,
                no_hide_stale = FALLOC_FL_NO_HIDE_STALE,
                collapse_range = FALLOC_FL_COLLAPSE_RANGE,
                zero_range = FALLOC_FL_ZERO_RANGE,
                insert_range = FALLOC_FL_INSERT_RANGE,
                unshare_range = FALLOC_FL_UNSHARE_RANGE
            };

            inline bool allocate(descriptor fd, allocation_type mode, offset_type length, offset_type offset) noexcept {
                return 0 == fallocate(fd, static_cast<int>(mode), offset, length);
            }

            inline size_type blocksize(descriptor fd) {
                struct stat st;
                if (fstat(fd, &st) < 0)
                    return VIDRE_STORAGE_BLOCKSIZE;
                return st.st_blksize;
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

            /** Relative position flag.
             * */
            enum whence_type : int {
                begin = detail::file::whence_type::begin,
                current = detail::file::whence_type::current,
                end = detail::file::whence_type::end,
            };

            /** Allocation type.
             *
             * @see FIXME
             * */
            enum allocation_type : int {
                allocate_range = detail::file::allocation_type::allocate_range,
                keep_size = detail::file::allocation_type::keep_size,
                punch_hole = detail::file::allocation_type::punch_hole,
                no_hide_stale = detail::file::allocation_type::no_hide_stale,
                collapse_range = detail::file::allocation_type::collapse_range,
                zero_range = detail::file::allocation_type::zero_range,
                insert_range = detail::file::allocation_type::insert_range,
                unshare_range = detail::file::allocation_type::unshare_range
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

            /** Seek position in file.
             * @param off       Offset (bytes).
             * @param whence    From where (default: begin).
             * @return Offset from beginning, @c -1 on error.
             * */
            offset_type seek(offset_type off, whence_type whence = begin) noexcept {
                return detail::file::seek(fd_, off, static_cast<detail::file::whence_type>(whence));
            }

            /** Get current position in file.
             * */
            offset_type position() noexcept {
                return seek(0, current);
            }

            /** File size.
             * */
            offset_type size() noexcept {
                auto curpos = position();
                auto retval = seek(0, end);
                seek(curpos, begin);
                return retval;
            }

            /** Reserve space on disk.
             * @todo needs implementation
             * */
            bool allocate(std::size_t len, offset_type offset = -1);

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

            /** Allocate file space.
             * @param mode The allocation mode.
             * @param length Number of bytes to allocate.
             * @param offset Position from where to allocate.
             * @returns `true` on success, `false` otherwise
             * */
            bool allocate(allocation_type mode, offset_type length, offset_type offset) noexcept {
                return detail::file::allocate(fd_, static_cast<detail::file::allocation_type>(mode), length, offset);
            }

            /** Reserve space on disk (at end of file).
             * This method on reserves space, optimizing append workloads.
             * */
            bool reserve(offset_type amount) noexcept {
                return allocate(keep_size, amount, size());
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
                    while (count > 0) {
                        size_type buffered = buf_.append(buf, count);
                        if (buf_.full())
                            flush();
                        count -= buffered;
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

        /** A file sink.
         *
         * FIXME
         *
         * */
        class VIDRE_CORE_VISIBILITY filesink : public virtual sink {
        public:
            /** Open mode.
             * */
            enum mode_type : uint32_t {
                none = 0,
                truncate = (1 << 0),    ///< Truncate file if it exists, otherwise create or append to the file.
                synchronous = (1 << 1), /**< Transfer data synchronously to the underlying hardware.
                                         * Using this option includes block-wise buffering to reduce workload on
                                         * the underlying hardware.
                                         * */
                unbuffered = (1 << 2)   /**< Disable automatic buffering. */
            };

        public:
            filesink() = default;
            virtual ~filesink() {
                flush();
            }

        protected:
            fileio fd_{};

        protected:
            // Low-level write method.
            offset_type write(const uint8_t *buf, size_type count) noexcept override {
                return fd_.write(buf, count);
            }

        public:
            bool valid() const noexcept override {
                return fd_.valid();
            }

            /** Open file.
             * @param filename The file name to open.
             * @param mode     File flags (default: synchronous with block-wise buffering).
             * @param permission File read/write permissions (POSIX style).
             * @return @c true on success
             * @return @c false otherwise
             */
            bool open(const std::filesystem::path &filename, mode_type mode = synchronous,
                      fileio::perms permissions = fileio::default_perms) noexcept {
                fileio::flag_type f = fileio::writeonly | fileio::create | fileio::close_on_exec;

                if (mode & mode_type::truncate)
                    f |= fileio::truncate;
                else
                    f |= fileio::append;

                if (mode & mode_type::synchronous)
                    f |= fileio::synchronous;

                bool retval = fd_.open(filename, f, permissions);
                if (not retval)
                    return false;

                if ((mode & mode_type::synchronous) and not(mode & mode_type::unbuffered))
                    buf_.setup(detail::file::blocksize(fd_));

                return true;
            }

            // FIXME setbuf(size_type n)

            /** Close file.
             * */
            void close() noexcept {
                if (buf_)
                    flush();
                fd_.close();
                return;
            }
        };

        ///@cond
        inline filesink::mode_type operator|(filesink::mode_type lhs, filesink::mode_type rhs) {
            return static_cast<filesink::mode_type>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline filesink::mode_type operator|=(filesink::mode_type lhs, filesink::mode_type rhs) {
            return static_cast<filesink::mode_type>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline filesink::mode_type operator&(filesink::mode_type lhs, filesink::mode_type rhs) {
            return static_cast<filesink::mode_type>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline filesink::mode_type operator&=(filesink::mode_type lhs, filesink::mode_type rhs) {
            return static_cast<filesink::mode_type>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline filesink::mode_type operator~(filesink::mode_type rhs) {
            return static_cast<filesink::mode_type>(~to_underlying(rhs));
        }
        ///@endcond

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
