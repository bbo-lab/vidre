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
 * @details
 * @fileinfo
 *
 * FIXME
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/memory/base.hh>
#include <vidre/exception.hh>
#include <vidre/util/uuid.hh>

#include <ankerl/unordered_dense.h>

#include <bit>
#include <utility>
#include <filesystem>

#include <cerrno>
#include <cstring>
#include <cassert>

namespace vidre {

    namespace detail {

        namespace io {

            /** Implementation details for I/O.
             * @ingroup api-core-io-details
             * */
            namespace detail {

                /** Write buffer.
                 * This simple buffer is mainly intended for use with sinks only,
                 * especially when using synchronous file writes. There, writing
                 * data not being a multiple of the underlying hardware's block size,
                 * can be orders of magnitude slower.
                 * */
                struct VIDRE_CORE_VISIBILITY buffer {
                    using size_type = memory::size_type;
                    using value_type = memory::value_type;
                    using pointer = memory::pointer;
                    using const_pointer = memory::const_pointer;

                    buffer() = default;
                    ~buffer() {
                        if (data)
                            std::free(data);
                    }

                    size_type size{};  ///< The overall buffer size.
                    size_type used{};  ///< Number of used bytes.
                    pointer data{};    ///< Memory pointer.

                    /** Setup buffer.
                     * @param len Number of bytes (rounded to a power of 2).
                     * @return Number of bytes allocated, or 0 if it was already allocated
                     *         previously or no memory could be allocated.
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

            }  // namespace detail

        }  // namespace io

    }  // namespace detail

    namespace io {

        /**@addtogroup api-core-io-types
         * */
        ///@{

        /** Unsigned default size type.
         * */
        using size_type = std::size_t;

        /** Signed default offset type.
         * */
        using offset_type = std::make_signed_t<size_type>;

        class sink;

        /** Data packet base type.
         * */
        struct VIDRE_CORE_VISIBILITY packet {
            /** Function type for putting a packet-based streams into a sink.
             * */
            using put_type = void (*)(const io::packet &source, io::sink &sink);

            vidre::uuid uuid{};  ///< Packet type (e.g., video, audio, metadata).
            uint32_t stream{};   ///< Stream identifier (e.g., for multiplexed streams).
            put_type putter{};   ///< Function pointer for putting this packet into a sink.

        public:
            using selector_type = put_type (*)(const io::packet &source, const io::packet &sink);
            using map_type = ankerl::unordered_dense::map<vidre::uuid, selector_type>;

        protected:
            static map_type registry;  ///< Static map for registering packet types and their put functions.

        public:
            /** Register a "selector" function.
             * */
            static void enregister(const vidre::uuid &uuid, selector_type func) {
                auto it = registry.find(uuid);
                if (it != registry.end())
                    throw vidre::exception() << error::message("packet type already registered");

                registry[uuid] = func;
            }

            /** Retrieve selector function.
             * */
            static selector_type retrieve(const vidre::uuid &uuid) {
                auto it = registry.find(uuid);
                if (it == registry.end())
                    throw vidre::exception() << error::message("packet type not registered");

                return it->second;
            }
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
        class VIDRE_CORE_VISIBILITY sink {
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
            sink(sink &&) = default;
            sink &operator=(sink &&) = default;

            virtual ~sink() {}

        protected:
            /** The low-level write method.
             * @param src Pointer to source data.
             * @param count Number of bytes to write.
             * @returns Number of bytes actually written.
             * */
            virtual offset_type write(memory::const_pointer src, size_type count) = 0;

        public:
            /** Check if the stream is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Transfer data.
             * @param count Number of bytes to write.
             * @param src Pointer to source data.
             * @returns Number of bytes actually written (-1 for write failure).
             * */
            virtual offset_type put(size_type count, memory::const_pointer src) {
                return write(src, count);
            }

            /** Transfer data from a memory view.
             * @param v Memory view containing the data to write.
             * @returns Number of bytes actually written (-1 for write failure).
             * */
            offset_type put(const memory::view &v) {
                return put(v.size(), v.data());
            }

            /** Write a packet.
             * */
            offset_type put(const packet *p) {
                // FIXME
                return -1;
            }
        };

        /** Data source.
         * 
         * FIXME
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

        protected:
            /** The low-level read method.
             * */
            virtual offset_type read(memory::pointer, size_type count) = 0;

        public:
            /** Check if the stream is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Transfer data.
             * @returns Number of bytes actually read (-1 for read failure).
             * */
            virtual offset_type get(size_type count, memory::pointer buf) {
                return read(buf, count);
            }

            offset_type get(memory::area &v) {
                return get(v.size(), v.data());
            }
        };

        /** FIXME
         * 
         * FIXME
         *
         * @see source::shared
         * */
        class VIDRE_CORE_VISIBILITY filter {
        public:
            /** Shared pointer type for filter.
             * This type is used for managing the lifetime of filter instances,
             * especially when they are shared across different components or threads.
             * @see filter::make
             * */
            using shared = std::shared_ptr<filter>;

            /** Create filter as shared pointer.
             * @tparam T Type of the filter to create.
             * @param args Arguments to forward to the filter's constructor.
             * @returns A shared pointer to the newly created filter.
             * @see filter::shared
             * */
            template <typename T, typename... Args>
            static shared make(Args &&...args) {
                return std::make_shared<T>(std::forward<Args>(args)...);
            }

        public:
            filter() = default;

        public:
            // FIXME
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_TYPES_HH
