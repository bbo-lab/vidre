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
#ifndef VIDRE_CORE_MEMORY_CHUNK_HH
#define VIDRE_CORE_MEMORY_CHUNK_HH
#pragma once

/**@addtogroup api-core-memory-chunk
 * @details
 * @fileinfo
 *
 * This header provides utilities to manage "chunks" of data (e.g. a video image).
 * ...FIXME..
 *
 * @{
 * @namespace vider::memory::chunk Chunk handling.
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/memory/base.hh>
#include <vidre/util/events.hh>

#include <bit>
#include <atomic>

namespace vidre {

    namespace memory {

        namespace chunk {

            /**@addtogroup api-core-memory-chunk
             * */
            ///@{

            /** A FIFO chunk manager.
             * @tparam N    Ring size.
             *              If zero (default), the ring is dynamically allocated, otherwise
             *              (requiring N being a power of 2) it is statically allocated.
             *
             * This class is designed to manage a ring of pointers to data chunks (e.g. video frames)
             * in a FIFO manner, where a "producer" queues pointers to new data chunks,
             * while the output is consumed by a "consumer" in the same order.
             *
             * FIXME
             * - event based...
             * - producer-consumer synchronization (e.g. via eventfd)...
             * - buffer management (e.g. via a "buffer factory",see "data accessors")...
             * - how to signal if the producer dies?
             * etc.
             *
             * @todo need to work out, if atomic operations are required....
             * */
            template <size_type N = 0>
            class fifo;

            template <>
            class VIDRE_CORE_VISIBILITY fifo<0> {
            public:
                fifo() = default;

                fifo(const fifo &) = delete;
                fifo &operator=(const fifo &) = delete;

                fifo(fifo &&) noexcept = delete; // FIXME should be movable...
                fifo &operator=(fifo &&) noexcept = delete;

                fifo(size_type n) : size_(n), store_(new pointer[n]) {}
                ~fifo() {
                    if (store_)
                        delete[] store_;
                }

            protected:
                size_type size_{};              ///< Ring size.
                pointer *store_{};              ///< Pointer store.
                size_type in_{};                ///< Ring head.
                std::atomic<size_type> out_{};  ///< Ring tail.

                event signal_{};  ///< Event to signal the consumer about new data.

            protected:
                void skip(size_type n) noexcept {
                    // FIXME adjust out_ to make space for the new element, i.e. drop the oldest one
                    abort_with_message("internal error: fifo full");
                }

                void next() noexcept {
                    ++in_;
                    in_ *= (in_ < size_);
                }

            public:
                /** Allocate space.
                 * @note This method may only be called once, iff the ring is not yet
                 *       allocated (i.e. size() == 0).
                 * */
                void allocate(size_type n) noexcept {
                    if (size_)
                        abort_with_message("internal error: fifo already allocated");

                    size_ = n;
                    store_ = new pointer[size_];
                }

                /** Get buffer count.
                 * */
                size_type size() const noexcept {
                    return size_;
                }

                /** Data accessors.
                 * */
                ///@{
                pointer operator[](size_type i) noexcept {
                    return store_[i];
                }
                const_pointer operator[](size_type i) const noexcept {
                    return store_[i];
                }
                ///@}

                /** Get free size.
                 * */
                size_type free() const noexcept {
                    bool trail = (in_ >= out_);
                    size_type a = trail * ((size_ - in_) + out_);  // out_ <= in_

                    bool lead = not trail;
                    size_type b = lead * (out_ - in_);  // in_ < out_;

                    return a + b;
                }

                /** Queue an element.
                 * */
                void queue() noexcept {
                    next();
                    signal_.notify();
                }

                void queue(pointer ptr) noexcept {
                    next();
                    store_[in_] = ptr;
                    signal_.notify();
                }

                /** Consume an element.
                 * FIXME
                 * */
                pointer consume() noexcept {
                    // FIXME
                    return nullptr;
                }
            };

            template <size_type N>
            class VIDRE_CORE_VISIBILITY fifo {
            public:
                static_assert(std::has_single_bit(N), "Ring size must be a power of 2.");

            public:
                fifo() = delete;
                fifo(size_type n);

            protected:
                size_type size_{};  ///< Ring size.
                pointer *data_{};   ///< Pointer store.

            public:
                static constexpr size_type size() noexcept {
                    return N;
                }

                // FIXME
            };

            ///@}

        }  // namespace chunk

    }  // namespace memory

}  // namespace vidre

#endif  // VIDRE_CORE_MEMORY_CHUNK_HH