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
 *
 * @{
 * @namespace vider::memory::chunk Chunk handling.
 * @}
 * */

#include <vidre/core/config.hh>

#include <bit>

namespace vidre {

    namespace memory {

        namespace chunk {

            /**@addtogroup api-core-memory-chunk
             * */
            ///@{

            /** FIXME
             * */
            class VIDRE_CORE_VISIBILITY ring {
            public:
                using size_type = std::size_t;
                using value_type = uint8_t;
                using pointer = uint8_t *;
                using const_pointer = const uint8_t *;

            public:
                ring() = delete;

                ring(size_type n);

            protected:
                size_type size_{};  ///< Ring size.
                pointer *data_{};  ///< Data store.

            public:
                /** Get buffer count.
                 * */
                size_type size() const noexcept {
                    return size_;
                }

                // /** Get free size.
                //  * */
                // size_type free() const noexcept {
                //     bool trail = (in_ >= out_);
                //     size_type a = trail * ((size_ - in_) + out_);  // out_ <= in_

                //     bool lead = not trail;
                //     size_type b = lead * (out_ - in_);  // in_ < out_;

                //     return a + b;
                // }

                // /** Add element.
                //  * */
                // bool push_back(pointer ptr) noexcept {
                //     if (not free())
                //         return false;

                //     ring_[in_] = ptr;
                //     ++in_;
                //     in_ *= (in_<size_);
                //     return true;
                // }

                // /** Remove element.
                //  * */
                // pointer pop_front() noexcept {
                //     pointer retval = ring_[out_];
                //     FIXME()
                // }
            };

            ///@}

        }  // namespace chunk

    }  // namespace memory

}  // namespace vidre

#endif  // VIDRE_CORE_MEMORY_CHUNK_HH