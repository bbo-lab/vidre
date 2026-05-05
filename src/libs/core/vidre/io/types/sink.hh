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
#ifndef VIDRE_CORE_IO_TYPES_SINK_HH
#define VIDRE_CORE_IO_TYPES_SINK_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/io/types/basic.hh>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-types
         * */
        ///@{

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
        class VIDRE_CORE_VISIBILITY sink : public virtual endpoint {
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
            sink(endpoint::cap c = endpoint::cap::none) : endpoint({c, endpoint::cap::none}) {}

            sink(const sink &) = delete;
            sink &operator=(const sink &) = delete;
            sink(sink &&) = delete;
            sink &operator=(sink &&) = delete;

            virtual ~sink() {}

        protected:
            event ready_{};  ///< Event for signaling sink ready state.

        protected:
            /** The low-level write method (if applicable).
             * @param buf Memory view containing the data to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual offset_type write(const memory::view &buf) noexcept {
                return invalid<offset_type>;
            }

            /** Signal that the sink is ready.
             * This method can be called by derived classes to indicate that the sink is ready
             * to accept data.
             * */
            void ready() {
                ready_.notify();
            }

        public:  // should be protected

            /** Step method for processing data from the source.
             * @param src The source to process data from.
             * @returns @c false if handler should stop processing, @c true to continue.
             * 
             * FIXME
             * 
             * */
            virtual bool step(source &src) {
                return false;
            }

        public:
            /** Check if the stream is valid (ie. usable).
             * */
            virtual bool valid() const noexcept {
                return false;
            }

            /** Configure sink.
             * @param config JSON configuration object.
             * @throw exception::logic_error if the configuration is invalid for this sink.
             * @note The configuration format is sink-specific and may be ignored by some sinks.
             * */
            virtual void setup(const settings::json &config) {}

            /** Transfer data from a memory view.
             * @param v Memory view containing the data to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual result put(const memory::view &v) {
                return write(v);
            }

            /** Put a value of type @c T.
             * @tparam T Type of the value to write.
             * @param v Value to write (in binary form).
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            template <typename T>
            requires (not std::derived_from<T, memory::view>)
            result put(const T &v) {
                return put({sizeof(T), reinterpret_cast<const memory::const_pointer>(&v)});
            }

            /** Write a packet.
             * @param p Pointer to the packet to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual result put(const packet::basic *p);

            /** Wait for sink ready signal.
             * @returns True if the sink is ready, false if the wait was interrupted.
             * */
            [[nodiscard]] event::result wait() {
                return ready_.wait();
            }

            /** Wait for sink ready signal, with a timeout.
             * @param timeout The maximum time to wait for the sink to become ready (milliseconds).
             * @returns True if the sink is ready, false if the wait timed out or was interrupted.
             * */
            [[nodiscard]] event::result wait(std::chrono::milliseconds timeout) {
                return ready_.wait(timeout);
            }

        private:
            friend class route;
            friend class dashboard;
        };

        ///@}

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_TYPES_SINK_HH
