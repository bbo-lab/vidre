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
#ifndef VIDRE_CORE_IO_TYPES_SOURCE_HH
#define VIDRE_CORE_IO_TYPES_SOURCE_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/io/types/sink.hh>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-types
         * */
        ///@{

        /** Data source.
         *
         * Data sources provide data to be read, such as files or filter outputs. They can
         * be either stream- or packet-based (see @c source::mode).
         *
         * */
        class VIDRE_CORE_VISIBILITY source : public virtual endpoint {
        public:
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

            source(const source &) = delete;
            source &operator=(const source &) = delete;
            source(source &&) = delete;
            source &operator=(source &&) = delete;

            virtual ~source() {}

        protected:
            event avail_{};  ///< Event for signaling data availability.

        protected:
            /** Notify, that @a n packets are available.
             * */
            event::state notify(size_type n = 1) {
                return avail_.notify(n);
            }

            /** The low-level read method.
             * @param buf Memory area to store the retrieved data.
             * @returns Number of bytes actually read (@c invalid for read failure).
             * */
            virtual offset_type read(memory::area buf) {
                return invalid<offset_type>;
            }

        protected:
            // FIXME

        public:
            /** Configure source.
             * @param config JSON configuration object.
             * @throw exception::logic if the configuration is invalid for this source.
             * @note The configuration format is source-specific and may be ignored by some sources.
             * */
            virtual void setup(const settings::json &config) {}

            /** Retrieve data from the source.
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

            /** Poll for data availability (waits indefinitely).
             * @returns True if data is available, false if the wait was interrupted.
             * @see vidre::event
             * */
            [[nodiscard]] event::result poll() {
                return avail_.wait();
            }

            /** Poll for data availability with timeout.
             * @param timeout The maximum time to wait for data (milliseconds).
             * @returns True if data is available, false if the wait timed out or was interrupted.
             * @see vidre::event
             * */
            [[nodiscard]] event::result poll(std::chrono::milliseconds timeout) {
                return avail_.wait(timeout);
            }

            /** Retrieve a packet from the source.
             * @returns Pointer to the retrieved packet, or nullptr if no packet is available.
             * @throw exception::logic if source is packet based
             * @throw exception::runtime for other errors
             * */
            virtual const packet::basic *get() {
                throw exception::logic();
            }

        private:
            friend class route;
            friend class dashboard;
        };

        ///@}

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_TYPES_SOURCE_HH
