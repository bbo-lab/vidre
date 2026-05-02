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
#ifndef VIDRE_CORE_IO_FILE_HH
#define VIDRE_CORE_IO_FILE_HH
#pragma once

/**@addtogroup api-core-io-file
 * @brief File based I/O.
 * @details
 * @fileinfo
 *
 * */

/**@addtogroup api-core-io-details
 * @{
 * @namespace vidre::io::detail::file    File based I/O.
 * @}
 * @todo UTF-8 support on Windows (see https://chat.deepseek.com/a/chat/s/cbb1b2b5-8943-4e15-9caf-956ec9e969fc)
 * */

#include <vidre/core/config.hh>
#include <vidre/io/types.hh>
#include <vidre/os/memory.hh>

#if VIDRE_HOST_WINDOWS
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#endif

#include <algorithm>

namespace vidre {

    namespace io {

        namespace detail {

            namespace file {

                /**@addtogroup api-core-io-details
                 * */
                ///@{

                /** Default block size (if otherwise it cannot be retrieved from the OS).
                 * */
#define VIDRE_IO_BLOCKSIZE 4096

                using size_type = vidre::io::size_type;
                using offset_type = vidre::io::offset_type;

#if VIDRE_HOST_WINDOWS
                // see below, the implementation is dictated by POSIX...
                using descriptor = HANDLE;

                // FIXME see https://chat.deepseek.com/a/chat/s/f177940c-6867-47e0-909a-de767477eec0
                //       and https://chat.deepseek.com/a/chat/s/bd0af659-b0ea-4d8b-b6ad-0cc295a385cc
#else  // not VIDRE_HOST_WINDOWS
                using descriptor = int;
                static constexpr descriptor invalid_descriptor = -1;

                static constexpr std::size_t path_max = PATH_MAX;

                enum class access_mode : int {
                    readonly = O_RDONLY,
                    writeonly = O_WRONLY,
                    readwrite = O_RDWR,
                };

                enum class disposition : int {
                    append = O_APPEND,
                    create = O_CREAT,
                    exclusive = O_EXCL,
                    truncate = O_TRUNC,
                };

                enum class hint : int {
                    none = 0,
                    close_on_exec = O_CLOEXEC,
                    non_block = O_NONBLOCK,
                    synchronous = O_SYNC,
#if defined(O_DIRECT)
                    direct = O_DIRECT,
#else
                    direct = 0,
#endif
                    datasync = O_DSYNC,
                };

                inline bool valid(descriptor fd) noexcept {
                    return fd >= 0;
                }

                inline std::error_code last_error() noexcept {
                    return std::error_code(errno, std::generic_category());
                }

                inline descriptor dup(descriptor fd) noexcept {
                    return ::dup(fd);
                }

                inline descriptor open(const char *filename, int access, int disposition, int hint = 0) noexcept {
                    return ::open(filename, access | disposition | hint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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

                enum class whence : int {
                    begin = SEEK_SET,
                    current = SEEK_CUR,
                    end = SEEK_END
                };

                inline offset_type seek(descriptor fd, offset_type off, int from) noexcept {
                    return ::lseek(fd, off, from);
                }

                inline offset_type filesize(descriptor fd) {
                    struct stat st;
                    if (fstat(fd, &st) < 0)
                        return -1;
                    return st.st_size;
                }

                inline size_type blocksize(descriptor fd) {
                    struct stat st;
                    if (fstat(fd, &st) < 0)
                        return VIDRE_IO_BLOCKSIZE;
                    return st.st_blksize;
                }

                inline std::string filename(descriptor fd) {
#if defined(F_GETPATH)
                    char path[PATH_MAX];
                    if (fcntl(fd, F_GETPATH, path) != -1)
                        return std::string(path);
                    return {};
#elif defined(__linux__)
                    char path[PATH_MAX];
                    ssize_t len = readlink((std::string("/proc/self/fd/") + std::to_string(fd)).c_str(), path, sizeof(path) - 1);
                    if (len != -1) {
                        path[len] = 0;
                        return std::string(path);
                    }
                    return {};
#else
                    static_assert(false, "filename retrieval not implemented for this platform");
                    return {};
#endif
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

                inline bool allocate(descriptor fd, int mode, offset_type length, offset_type offset) noexcept {
                    return 0 == fallocate(fd, mode, offset, length);
                }

                inline descriptor fileno(FILE *f) noexcept {
                    return ::fileno(f);
                }

#endif  // not VIDRE_HOST_WINDOWS

                ///@}

            }  // namespace file

        }  // namespace detail

    }  // namespace io

    namespace io {

        /**@addtogroup api-core-io-file
         * */
        ///@{

        class VIDRE_CORE_VISIBILITY file {
        public:
            class sink;    // forward declaration
            class source;  // forward declaration

            using descriptor = io::detail::file::descriptor;
            using size_type = io::detail::file::size_type;
            using offset_type = io::detail::file::offset_type;

            /** Maximum length of a path.
             * */
            static constexpr size_type path_max = io::detail::file::path_max;

            /** File access.
             * */
            enum class access {
                readonly = to_underlying(io::detail::file::access_mode::readonly),    ///< Open file for reading only.
                writeonly = to_underlying(io::detail::file::access_mode::writeonly),  ///< Open file for writing only.
                readwrite = to_underlying(io::detail::file::access_mode::readwrite)   ///< Open file for reading and writing.
            };

            /** File disposition.
             * @note Not all combinations of these flags are valid, and some may
             *       be ignored by the underlying OS.
             * */
            enum class disposition {
                create = to_underlying(io::detail::file::disposition::create),        ///< Create file if it does not exist.
                exclusive = to_underlying(io::detail::file::disposition::exclusive),  ///< Fail if file already exists.
                append = to_underlying(io::detail::file::disposition::append),        ///< Append to end of file.
                truncate = to_underlying(io::detail::file::disposition::truncate)  ///< Truncate file to zero length if it exists.
            };

            /** Additional file hints.
             * @note Not all combinations of these flags are valid, and some may
             *       be ignored by the underlying OS. If the underlying OS does
             *       not support any of these flags, they will be silently ignored.
             *
             * @internal
             * Unsupported flags are defined as 0, so that they can be used without
             * the need for conditional compilation.
             * @endinternal
             * */
            enum class hint {
                none = 0,
                close_on_exec = to_underlying(io::detail::file::hint::close_on_exec),  ///< Set close-on-exec flag (POSIX only).
                non_block = to_underlying(io::detail::file::hint::non_block),      ///< Open file in non-blocking mode (POSIX only).
                synchronous = to_underlying(io::detail::file::hint::synchronous),  ///< Open file in synchronous mode.
                direct = to_underlying(io::detail::file::hint::direct),            ///< Open file with direct I/O.
                datasync = to_underlying(io::detail::file::hint::datasync)         ///< Open file with data synchronization.
            };

            /** Seek options.
             * */
            enum class whence {
                begin = to_underlying(io::detail::file::whence::begin),      ///< Seek from beginning of file.
                current = to_underlying(io::detail::file::whence::current),  ///< Seek from current position in file.
                end = to_underlying(io::detail::file::whence::end)           ///< Seek from end of file.
            };

            /** Allocation modes.
             * */
            enum class allocation {
                allocate_range =
                    to_underlying(io::detail::file::allocation_type::allocate_range),  ///< Allocate space for a range of the file.
                keep_size =
                    to_underlying(io::detail::file::allocation_type::keep_size),  ///< Allocate space without changing file size.
                punch_hole = to_underlying(
                    io::detail::file::allocation_type::punch_hole),  ///< Deallocate space (create a "hole") in a range of the file.
                no_hide_stale = to_underlying(io::detail::file::allocation_type::no_hide_stale),  ///< Do not hide stale data when
                                                                                                  ///< punching holes (Linux only).
                collapse_range = to_underlying(
                    io::detail::file::allocation_type::collapse_range),  ///< Collapse a range of the file, removing the bytes and
                                                                         ///< shifting subsequent data (Linux only).
                zero_range =
                    to_underlying(io::detail::file::allocation_type::zero_range),  ///< Zero out a range of the file, deallocating
                                                                                   ///< space if possible (Linux only).
                insert_range = to_underlying(
                    io::detail::file::allocation_type::insert_range),  ///< Insert a range of the file, creating a hole and shifting
                                                                       ///< subsequent data (Linux only).
                unshare_range =
                    to_underlying(io::detail::file::allocation_type::unshare_range)  ///< Unshare a range of the file, breaking
                                                                                     ///< copy-on-write sharing (Linux only).
            };

        public:
            file() = default;

            file(const file &) = delete;
            file &operator=(const file &) = delete;
            file(file &&) = default;
            file &operator=(file &&) = default;

            file(const std::filesystem::path &filename, access access_mode = access::readonly,
                 disposition disposition_mode = static_cast<disposition>(0), hint hint_mode = hint::none) noexcept {
                open(filename, access_mode, disposition_mode, hint_mode);
            }

            file(const char *filename, access access_mode = access::readonly,
                 disposition disposition_mode = static_cast<disposition>(0), hint hint_mode = hint::none) noexcept {
                open(filename, access_mode, disposition_mode, hint_mode);
            }

            /** Open a file from a C FILE pointer.
             * @param f    The C FILE pointer.
             *
             * This constructor is intended for use with stdin, stdout, and stderr,
             * but can be used with any valid FILE pointer.
             *
             * @note The file will be duplicated, so the caller retains ownership of the
             *       FILE pointer and is responsible for closing it. The returned file
             *       object will have its own file descriptor that is independent of the
             *       original FILE pointer.
             * */
            file(FILE *f) noexcept {
                fd_ = io::detail::file::dup(io::detail::file::fileno(f));
            }

            ~file() {
                if (valid())
                    close();
            };

        protected:
            descriptor fd_{io::detail::file::invalid_descriptor};  ///< File descriptor.

        public:
            /** Check if the file is valid (ie. open).
             * */
            bool valid() const noexcept {
                return io::detail::file::valid(fd_);
            }

            /** Check if the file is valid (ie. open).
             * */
            operator bool() const noexcept {
                return valid();
            }

            /** Implicit conversion to host descriptor type.
             * */
            operator descriptor() const noexcept {
                return fd_;
            }

            /** Get last error.
             * @return Last error code.
             * */
            std::error_code error() const noexcept {
                return io::detail::file::last_error();
            }

            /** Get file size.
             * @return File size in bytes, @c -1 on error.
             * */
            offset_type filesize() const noexcept {
                return io::detail::file::filesize(fd_);
            }

            /** Get file block size.
             * @return Block size in bytes, or a default value if it cannot be retrieved from the OS.
             * */
            size_type blocksize() const noexcept {
                return io::detail::file::blocksize(fd_);
            }

            /** Open a file.
             * @param filename    The file name to open.
             * @param access      Access mode (default: readonly).
             * @param disposition File disposition (default: none).
             * @param hint        Additional hints (default: none).
             * @return @c true on success
             * @return @c false otherwise
             */
            bool open(const std::filesystem::path &filename, access access_mode = access::readonly,
                      disposition disposition_mode = static_cast<disposition>(0), hint hint_mode = hint::none) noexcept {
                fd_ = io::detail::file::open(filename.c_str(),                    //
                                             static_cast<int>(access_mode),       //
                                             static_cast<int>(disposition_mode),  //
                                             static_cast<int>(hint_mode));
                return valid();
            }

            /** Close the file.
             * */
            void close() noexcept {
                io::detail::file::close(fd_);
                fd_ = io::detail::file::invalid_descriptor;
            }

            /** Seek position in file.
             * @param off       Offset (bytes).
             * @param whence    From where (default: begin).
             * @return Offset from beginning, @c -1 on error.
             * */
            offset_type seek(offset_type off, whence from = whence::begin) noexcept {
                return io::detail::file::seek(fd_, off, static_cast<int>(from));
            }

            /** Read data.
             * @param buf   Buffer to read into.
             * @param count Number of bytes to read.
             * @return Number of bytes read, @c -1 on error.
             * */
            offset_type read(memory::area &buf) noexcept {
                return io::detail::file::read(fd_, buf.data(), buf.size());
            }

            /** Write data.
             * @param buf   Buffer to write from.
             * @param count Number of bytes to write.
             * @return Number of bytes written, @c -1 on error.
             * */
            offset_type write(const memory::view &buf) noexcept {
                return io::detail::file::write(fd_, buf.data(), buf.size());
            }

            /** Allocate space.
             * @param offset    Offset (bytes).
             * @param length    Length (bytes).
             * @param type      Allocation type.
             * @return @c true on success
             * @return @c false otherwise
             *
             * @note Not all allocation types are supported on all platforms,
             *       and some may be ignored by the underlying OS. If the
             *       underlying OS does not support any of these flags, they
             *       will be silently ignored.
             * */
            bool allocate(offset_type offset, offset_type length, allocation type = allocation::keep_size) noexcept {
                return io::detail::file::allocate(fd_, static_cast<int>(type), offset, length);
            }
        };

        ///@cond
        inline file::disposition operator|(file::disposition lhs, file::disposition rhs) {
            return static_cast<file::disposition>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline file::disposition &operator|=(file::disposition &lhs, file::disposition rhs) {
            lhs = static_cast<file::disposition>(to_underlying(lhs) | to_underlying(rhs));
            return lhs;
        }
        inline file::disposition operator&(file::disposition lhs, file::disposition rhs) {
            return static_cast<file::disposition>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline file::disposition &operator&=(file::disposition &lhs, file::disposition rhs) {
            lhs = static_cast<file::disposition>(to_underlying(lhs) & to_underlying(rhs));
            return lhs;
        }
        inline file::disposition operator~(file::disposition rhs) {
            return static_cast<file::disposition>(~to_underlying(rhs));
        }
        inline file::hint operator|(file::hint lhs, file::hint rhs) {
            return static_cast<file::hint>(to_underlying(lhs) | to_underlying(rhs));
        }
        inline file::hint &operator|=(file::hint &lhs, file::hint rhs) {
            lhs = static_cast<file::hint>(to_underlying(lhs) | to_underlying(rhs));
            return lhs;
        }
        inline file::hint operator&(file::hint lhs, file::hint rhs) {
            return static_cast<file::hint>(to_underlying(lhs) & to_underlying(rhs));
        }
        inline file::hint &operator&=(file::hint &lhs, file::hint rhs) {
            lhs = static_cast<file::hint>(to_underlying(lhs) & to_underlying(rhs));
            return lhs;
        }
        inline file::hint operator~(file::hint rhs) {
            return static_cast<file::hint>(~to_underlying(rhs));
        }
        ///@endcond

        /** A file sink.
         *
         * The file sink provides a buffered interface for writing to files. It
         * manages an internal buffer and flushes it to the underlying file when
         * necessary. The file is created in write-only mode, and can be
         * configured with various flags for direct I/O, truncation, and
         * buffering.
         *
         * */
        class VIDRE_CORE_VISIBILITY file::sink : public virtual io::sink {
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

            sink(const sink &) = delete;
            sink &operator=(const sink &) = delete;
            sink(sink &&other) noexcept = delete;
            sink &operator=(sink &&other) noexcept = delete;

            sink(const std::filesystem::path &filename, openmode_type mode = direct) {
                if (!open(filename, mode)) {
                    throw exception::runtime()                                          //
                        << error::message("failed to open file: " + filename.string())  //
                        << error::code(fd_.error());
                }
            }

            virtual ~sink() {
                flush();
                fd_.close();
            }

        protected:
            file fd_{};                 ///< Underlying file I/O.
            io::detail::buffer buf_{};  ///< Internal block buffer.

        protected:
            // Write buffer to disk and flush it.
            offset_type flush();

            // Low-level write method.
            offset_type write(const memory::view &buf) noexcept override;

        public:
            /** Check if valid (i.e., usable).
             * */
            bool valid() const noexcept override {
                return fd_.valid();
            }

            endpoint::cap input() const noexcept override {
                // FIXME
                return endpoint::cap::any;
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

            // FIXME void setup(const settings::json &config) override;

            /** Open file.
             * @param filename The file name to open.
             * @param mode     File flags (default: direct with block-wise buffering).
             * @return @c true on success
             * @return @c false otherwise
             */
            bool open(const std::filesystem::path &filename, openmode_type mode = direct) noexcept;

            /** Close file.
             * */
            void close() noexcept;
        };

        /** File source.
         *
         *
         *
         * */
        class VIDRE_CORE_VISIBILITY file::source : public virtual io::source {
        public:
            source() = default;

            source(const source &) = delete;
            source &operator=(const source &) = delete;
            source(source &&) = delete;
            source &operator=(source &&) = delete;

            source(const std::filesystem::path &filename) {
                if (!open(filename)) {
                    throw exception::runtime()                                          //
                        << error::message("failed to open file: " + filename.string())  //
                        << error::code(fd_.error());
                }
            }

            virtual ~source() {
                fd_.close();
            }

        protected:
            file fd_{};  ///< Underlying file I/O.

        protected:
            // Low-level read method.
            offset_type read(memory::area buf) noexcept override {
                return fd_.read(buf);
            }

        public:
            /** Check if valid (i.e., usable).
             * */
            bool valid() const noexcept override {
                return fd_.valid();
            }

            endpoint::cap output() const noexcept override {
                // FIXME
                return endpoint::cap::any | endpoint::cap::ondemand;
            }

            // FIXME void setup(const settings::json &config) override;

            /** Open file.
             * @param filename The file name to open.
             * @return @c true on success
             * @return @c false otherwise
             */
            bool open(const std::filesystem::path &filename) noexcept {
                return fd_.open(filename, file::access::readonly);
            }

            /** Close file.
             * */
            void close() noexcept {
                fd_.close();
            }

            void stream(sink::shared snk, thread::shared thr = {}) override;
        };

        namespace standard {

            /** Standard input. */
            extern file input;

            /** Standard output. */
            extern file output;

            /** Standard error. */
            extern file error;

        }  // namespace standard

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_FILE_HH
