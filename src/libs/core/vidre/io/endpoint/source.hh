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
#ifndef VIDRE_CORE_IO_ENDPOINT_SOURCE_HH
#define VIDRE_CORE_IO_ENDPOINT_SOURCE_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/sync/events.hh>
#include <vidre/io/endpoint/base.hh>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-endpoint
         * */
        ///@{

        /** Data source.
         *
         * Data sources provide data to be read, such as files or filter outputs. They can
         * be either chunk- or packet-based (see @c source::mode).
         *
         * Packet based sources implement a task loop that either pushes packets directly
         * to a sink, or use an event to notify that sink of data availability.
         *
         * Example:
         * @snippet examples/core/packetio.cc simple-packet-source
         * */
        class VIDRE_CORE_VISIBILITY source : public endpoint::source {
        public:
            source() = default;

            source(const source &) = delete;
            source &operator=(const source &) = delete;
            source(source &&) = delete;
            source &operator=(source &&) = delete;

            virtual ~source();

        protected:
            shared<endpoint::sink> source_follower_{};  ///< Connected io::sink.
            shared<task> source_task_{};                ///< Task handler for event-pull or push method.
            event source_event_{};                      ///< Event for notifying data availability.

        protected:
            /** The low-level read method (if applicable).
             * @param buf Memory area to store the retrieved data.
             * @returns Number of bytes actually read (@c invalid for read failure).
             * */
            virtual offset_type read(memory::area buf) {
                return invalid<offset_type>;
            }

        public:
            /** Retrieve a chunk of data from the source.
             * @param v Memory area to store the retrieved data.
             * @throw exception::logic if source is packet based.
             * @throw exception::runtime for other errors
             * @returns Number of bytes actually read (@c invalid for read failure).
             * */
            virtual result get(memory::area v) {
                result retval = read(v);

                if (retval)
                    stats_.bytes.transmitted += retval.count;
                else
                    stats_.errors++;

                return retval;
            }

            /** Pull a packet from this source.
             * @throw exception::logic if source is not packet based.
             * @returns Shared pointer to the retrieved packet.
             * */
            shared<packet> get() override {
                throw exception::internal() << "method not implemented or capability erroneously advertised";
            }

        public:
            status unlink() override;

            bool isready(endpoint::dir direction = endpoint::dir::any) const noexcept override {
                if (!source_follower_)
                    return false;

                if (!!(direction & endpoint::dir::sink))
                    return source_follower_->isready(endpoint::dir::sink);

                return true;
            }

        public:
            /** Get the queue depth limit for this source.
             *
             * @return The maximum number of packets that can be buffered
             *         before data loss occurs:
             *         - `0` (default): does not apply (not packet-based)
             *         - `io::dynamic` (-1): unlimited (only limited by system memory)
             *         - `> 0`: maximum number of packets that are "safe"
             *
             * @rationale{
             *      Sources—typically tied to devices that generate data—are
             *      expected to set up their memory needs before acquisition starts,
             *      for example in the form of a ring buffer. A camera driver, for
             *      instance, would allocate all memory resources before starting
             *      acquisition to buffer frames for at least one second.
             *
             *      Sinks, on the other hand, are only expected to react to a
             *      data packet as fast as they can and simply drop packets if they
             *      cannot keep up. This holds for storing data on disk as much as
             *      for cases where, for example, a neural network reacts to camera
             *      observations but at a slower rate than the camera's acquisition
             *      rate.
             * }
             */
            virtual size_type queue_depth() const noexcept {
                return 0;
            }

            /** Wait indefinitely until a packet is available for pulling. */
            [[nodiscard]] event::result_type wait() {
                return source_event_.wait();
            }

            /** Wait until a packet is available for pulling.
             * @param timeout The maximum time to wait for a packet.
             * */
            [[nodiscard]] event::result_type wait(std::chrono::milliseconds timeout) {
                return source_event_.wait(timeout);
            }

            /** Cancel the source event.
             * This method cancels any ongoing wait operations on the source event.
             * */
            void cancel() noexcept {
                // FIXME currently public, consider making it protected
                source_event_.cancel();
            }

        protected:
            /** Notify the sink, that a packet is available for pulling.
             * @param count The number of packets available for pulling.
             * */
            auto notify(uint64_t count = 1) {
                return source_event_.notify(count);
            }

            /** Event step.
             * This method is called in a loop by the default thread handler to
             * process source events. A typical implementation would check for
             * available data and handle them accordingly, for example:
             * @code
             *     if (wait_for_data_available()) {
             *         // read data and assemble packet
             *         // ...
             *
             *         // store packet in output queue
             *         output_queue_.push(packet);
             *
             *         // notify the sink that a new packet is available
             *         notify();
             *     }
             *     else {
             *         // handle signals (such as termination) and errors
             *         // ...
             *     }
             * @endcode
             * @note The implementation has to take into account the source capabilities!
             * @see endpoint::getcap()
             * */
            virtual bool source_step() {
                std::this_thread::yield();
                return true;
            }

            /** Default thread handler.
             * @param pause Pause duration in milliseconds (used, for example, when waiting for the source to become active).
             * @see endpoint::isactive()
             * @see source_step()
             * */
            virtual void source_handler(shared<task> self, std::chrono::milliseconds pause = std::chrono::milliseconds(1));

        protected:
            status attach(shared<endpoint::sink> snk, cap::flag_type cap) override;
            status detach(const endpoint::sink *ptr) override;
        };

        ///@}

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_ENDPOINT_SOURCE_HH
