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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_IO_PACKET_UTIL_HH
#define VIDRE_CORE_IO_PACKET_UTIL_HH
#pragma once

/**@addtogroup api-core-io-packet-util
 * @brief Utilities for managing and manipulating packets.
 * @details
 * @fileinfo
 */

#include <vidre/core/config.hh>
#include <vidre/io/packet/base.hh>

#include <shared_mutex>
#include <queue>

namespace vidre {

    namespace io {

         /**@addtogroup api-core-io-packet-util
         * @{
         * */

        /** Queue for packets.
         *
         * This class provides a thread-safe queue for managing packets. It ensures
         * that multiple threads can safely add and remove packets without causing data
         * races or inconsistencies.
         *
         * In addition, it provides mechanisms to control the maximum depth of the queue,
         * allowing for both fixed and dynamic queue sizes.
         *
         * @see setdepth()
         * */
        class VIDRE_CORE_VISIBILITY packet::queue {
        public:
            /** Dynamic queue size indicator.
             * */
            constexpr static size_type dynamic = size_type(-1);

        public:
            queue(size_type d = dynamic) noexcept : depth_(d + (d == 0) * dynamic) {}

            queue(const queue &) = delete;
            queue &operator=(const queue &) = delete;
            queue(queue &&) = delete;
            queue &operator=(queue &&) = delete;

        protected:
            std::queue<shared<packet>> queue_{};  ///< The underlying queue storing the packets.
            mutable std::shared_mutex mutex_{};   ///< Mutex to protect access to the queue.
            size_type depth_{dynamic};            ///< Maximum size of the queue, dynamic if not fixed.

        public:
            /** Maximum depth of the queue.
             * @see setdepth()
             * */
            size_type depth() const noexcept {
                return depth_;
            }

            /** Set maximum depth of the queue.
             *
             * Devices, such as cameras, do not typically reallocate space for data packets
             * once allocated during device setup. This means, that packets may be overwritten
             * when the device produces new packets. This queue must, therefore, never hold
             * "dirty" (ie. overwritten) packets.
             *
             * @note The depth of the queue must be set before usage.
             *
             * @param d The new maximum depth (0 is understood as equivalent to the `dynamic` indicator).
             * @see push()
             * */
            void setdepth(size_type d) noexcept {
                depth_ = (d == 0) ? dynamic : d;
            }

            /** Number of queued packets.
             * */
            size_type size() const noexcept {
                std::shared_lock lock(mutex_);
                return queue_.size();
            }

            /** Check if the queue is empty.
             * */
            bool empty() const noexcept {
                std::shared_lock lock(mutex_);
                return queue_.empty();
            }

            /** Access the front packet in the queue.
             * @returns The packet at the front of the queue, or nullptr if the queue is empty.
             * */
            shared<packet> front() const noexcept {
                std::shared_lock lock(mutex_);
                if (!queue_.empty())
                    return queue_.front();
                return nullptr;
            }

            /** Push a packet into the queue.
             * @param p The packet to be added to the queue.
             * @returns `true` if the packet was successfully added to the queue,
             *          `false` if a packet had to be dropped (see `setdepth()`).
             * */
            bool push(shared<packet> p) {
                std::unique_lock lock(mutex_);
                if (queue_.size() < depth_) [[likely]]
                    queue_.push(std::move(p));
                else {
                    if (depth_ == 1) {
                        queue_.front() = std::move(p);
                    }
                    else {
                        queue_.pop();
                        queue_.push(std::move(p));
                    }
                    return false;
                }

                return true;
            }

            /** Pop the front packet from the queue.
             * */
            shared<packet> pop() {
                std::unique_lock lock(mutex_);

                if (queue_.empty())
                    return nullptr;

                auto p = std::move(queue_.front());
                queue_.pop();
                return p;
            }
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_PACKET_UTIL_HH
