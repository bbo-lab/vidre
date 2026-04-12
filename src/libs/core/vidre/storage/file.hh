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
#ifndef VIDRE_CORE_STORAGE_FILE_HH
#define VIDRE_CORE_STORAGE_FILE_HH
#pragma once

/**@defgroup api-core-storage-file File handling.
 * @ingroup api-core-storage
 * @fileinfo
 * @details
 *
 * Apart for data sinks and sources, this module provides basic file handling
 * functionality mostly pulled in from standard libraries (see
 * [std::filesystem](https://en.cppreference.com/w/cpp/filesystem.html))
 * for convenience.
 * */

#include <vidre/core/config.hh>
#include <vidre/storage/io.hh>
#include <vidre/exception.hh>

#include <filesystem>
#include <utility>

#include <cerrno>
#include <cstring>
#include <cassert>

#if VIDRE_HOST_WINDOWS
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#endif

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

            // FIXME seek, allocate, blocksize, filename, mmap, munmap...

            static constexpr std::size_t path_max = MAX_PATH;
#else
            enum flag_type : int {
                invalid = -1,

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
                asynchrounous = O_ASYNC,
                direct = O_DIRECT,
                datasync = O_DSYNC,
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

            inline offset_type filesize(descriptor fd) {
                struct stat st;
                if (fstat(fd, &st) < 0)
                    return -1;
                return st.st_size;
            }

            inline std::string filename(descriptor fd) {
#if defined(__linux__)
                char path[PATH_MAX];
                ssize_t len = readlink((std::string("/proc/self/fd/") + std::to_string(fd)).c_str(), path, sizeof(path) - 1);
                if (len != -1) {
                    path[len] = 0;
                    return std::string(path);
                }
                return {};
#elif defined(F_GETPATH)
                char path[PATH_MAX];
                if (fcntl(fd, F_GETPATH, path) != -1)
                    return std::string(path);
                return {};
#endif
            }

            static constexpr std::size_t path_max = PATH_MAX;
#endif

            ///@}

        }  // namespace file

    }  // namespace detail

    namespace storage {

        /** File related functionality.
         * @ingroup api-core-storage-file
         * */
        namespace file {

            /**@addtogroup api-core-storage-file
             * */
            ///@{

            /** File path.
             * */
            using path = std::filesystem::path;

            /** Remove file.
             * */
            inline bool remove(const path &filename) noexcept {
                return std::filesystem::remove(filename);
            }

            /** Trivial file IO.
             * */
            class VIDRE_CORE_VISIBILITY io {
            public:
                using size_type = std::size_t;
                using offset_type = std::make_signed_t<size_type>;

                using descriptor = detail::file::descriptor;

            public:
                /** Maximum length of a path.
                 * */
                static constexpr size_type path_max = detail::file::path_max;

                /** File flags.
                 * */
                enum flag_type : int {
                    invalid = detail::file::flag_type::invalid,  ///< Invalid flag (for error handling).

                    readonly = detail::file::flag_type::readonly,    ///< Open for reading only.
                    writeonly = detail::file::flag_type::writeonly,  ///< Open for writing only.
                    readwrite = detail::file::flag_type::readwrite,  ///< Open for reading and writing.

                    append = detail::file::flag_type::append,        ///< Append to the end of the file.
                    create = detail::file::flag_type::create,        ///< Create the file if it does not exist.
                    exclusive = detail::file::flag_type::exclusive,  ///< Ensure that this call creates the file.
                    truncate = detail::file::flag_type::truncate,    ///< Truncate the file to zero length.

                    non_block = detail::file::flag_type::non_block,          ///< Open in non-blocking mode.
                    synchronous = detail::file::flag_type::synchronous,      ///< Open in synchronous mode (like @c direct, but in
                                                                             ///< addition writes file metadata).
                    close_on_exec = detail::file::flag_type::close_on_exec,  ///< Close the file on exec.
                    asynchrounous = detail::file::flag_type::asynchrounous,  ///< Open in asynchronous mode.
                    direct = detail::file::flag_type::direct,                ///< Open in direct mode (no caching).
                    datasync = detail::file::flag_type::
                        datasync,  ///< Open in datasync mode (like synchronous, but ensure flushed caches as well (see
                                   ///< [fdatasync](https://www.man7.org/linux/man-pages/man3/fdatasync.3p.html))).

                    // FIXME...
                };

                /** Relative position flag.
                 * */
                enum whence_type : int {
                    begin = detail::file::whence_type::begin,
                    current = detail::file::whence_type::current,
                    end = detail::file::whence_type::end,
                };

                /** File permissions (POSIX style).
                 * */
                using perms = std::filesystem::perms;

                /** Default permissions (o+rw,g+r,u+r).
                 * */
                static constexpr perms default_perms =
                    perms::owner_read | perms::owner_write | perms::group_read | perms::others_read;

            public:
                io() = default;

                io(const io &) = delete;
                io(io &&other) noexcept : fd_(std::exchange(other.fd_, -1)) {}

                io(const char *filename, flag_type flags = readonly, perms permissions = default_perms) noexcept {
                    open(filename, flags, permissions);
                }

                io(const std::string &filename, flag_type flags = readonly, perms permissions = default_perms) noexcept
                    : io(filename.c_str(), flags, permissions) {}

                io(const std::filesystem::path &filename, flag_type flags = readonly, perms permissions = default_perms) noexcept
                    : io(filename.c_str(), flags, permissions) {}

                ~io() {
                    if (valid())
                        close();
                }

                void swap(io &other) noexcept {
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
                    return detail::file::filesize(fd_);
                }

                /** Reserve space on disk.
                 * @param len Number of bytes to reserve.
                 * @param offset Position from where to reserve (default: end of file).
                 * @returns `true` on success, `false` otherwise
                 * This method reserves space, optimizing append workloads.
                 * */
                bool reserve(std::size_t len, offset_type offset = -1) {
                    if (offset < 0)
                        offset = size();
                    return detail::file::allocate(fd_, detail::file::allocation_type::keep_size, len, offset);
                }

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
            inline io::flag_type operator|(io::flag_type lhs, io::flag_type rhs) {
                return static_cast<io::flag_type>(to_underlying(lhs) | to_underlying(rhs));
            }
            inline io::flag_type operator|=(io::flag_type lhs, io::flag_type rhs) {
                return static_cast<io::flag_type>(to_underlying(lhs) | to_underlying(rhs));
            }
            inline io::flag_type operator&(io::flag_type lhs, io::flag_type rhs) {
                return static_cast<io::flag_type>(to_underlying(lhs) & to_underlying(rhs));
            }
            inline io::flag_type operator&=(io::flag_type lhs, io::flag_type rhs) {
                return static_cast<io::flag_type>(to_underlying(lhs) & to_underlying(rhs));
            }
            inline io::flag_type operator~(io::flag_type rhs) {
                return static_cast<io::flag_type>(~to_underlying(rhs));
            }
            ///@endcond

            /** A file sink.
             *
             * FIXME
             *
             * */
            class VIDRE_CORE_VISIBILITY sink : public virtual storage::sink {
            public:
                /** Open mode.
                 * */
                enum openmode_type : uint32_t {
                    direct = (1 << 0),  ///< Transfer data directly to the underlying hardware (avoid caching, default).

                    truncate = (1 << 4),  ///< Truncate file if it exists, otherwise append to the file.

                    unbuffered = (1 << 8),  ///< Disable buffering.
                };

            public:
                sink() = default;
                virtual ~sink() {
                    flush();
                }

                sink(const sink &) = delete;
                sink(sink &&other) noexcept = default;

                sink(const std::filesystem::path &filename, openmode_type mode = direct,
                     io::perms permissions = io::default_perms) {
                    if (!open(filename, mode, permissions)) {
                        throw exception::runtime()
                            << error::message("Failed to open file: " + filename.string()) << error::system();
                    }
                }

            protected:
                io fd_{};       ///< Underlying file I/O.
                buffer buf_{};  ///< Internal block buffer.

            protected:
                // Write buffer to disk and flush it.
                offset_type flush();

                // Low-level write method.
                offset_type write(const uint8_t *src, size_type count) noexcept override {
                    return fd_.write(src, count);
                }

            public:
                /** Check if valid (i.e., usable).
                 * */
                bool valid() const noexcept override {
                    return fd_.valid();
                }

                /** Set buffer size.
                 * @param len Buffer size in bytes (should be a power of 2).
                 * @return Number of bytes allocated, or 0 if it was already allocated
                 *         previously or no memory could be allocated.
                 * This method may only be called once.
                 * */
                size_type setbuf(size_type len) noexcept {
                    return buf_.setup(len);
                }

                /** Open file.
                 * @param filename The file name to open.
                 * @param mode     File flags (default: direct with block-wise buffering).
                 * @param permission File read/write permissions (POSIX style, default: 0644).
                 * @return @c true on success
                 * @return @c false otherwise
                 */
                bool open(const std::filesystem::path &filename, openmode_type mode = direct,
                          io::perms permissions = io::default_perms) noexcept;

                /** Close file.
                 * */
                void close() noexcept;

                /** Put data into the sink.
                 * */
                offset_type put(size_type count, const uint8_t *src) override;
            };

            ///@cond
            inline sink::openmode_type operator|(sink::openmode_type lhs, sink::openmode_type rhs) {
                return static_cast<sink::openmode_type>(to_underlying(lhs) | to_underlying(rhs));
            }
            inline sink::openmode_type operator|=(sink::openmode_type lhs, sink::openmode_type rhs) {
                return static_cast<sink::openmode_type>(to_underlying(lhs) | to_underlying(rhs));
            }
            inline sink::openmode_type operator&(sink::openmode_type lhs, sink::openmode_type rhs) {
                return static_cast<sink::openmode_type>(to_underlying(lhs) & to_underlying(rhs));
            }
            inline sink::openmode_type operator&=(sink::openmode_type lhs, sink::openmode_type rhs) {
                return static_cast<sink::openmode_type>(to_underlying(lhs) & to_underlying(rhs));
            }
            inline sink::openmode_type operator~(sink::openmode_type rhs) {
                return static_cast<sink::openmode_type>(~to_underlying(rhs));
            }
            ///@endcond

            ///@}

        }  // namespace file

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_FILE_HH
