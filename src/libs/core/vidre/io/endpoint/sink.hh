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
#ifndef VIDRE_CORE_IO_TYPES_SINK_HH
#define VIDRE_CORE_IO_TYPES_SINK_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/io/endpoint/base.hh>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-endpoint
         * */
        ///@{

        /** Data sink skeleton.
         *
         * The sink class provides a skeleton for data sinks, such as files or filter inputs.
         * It may work as a data stream or a memory region (see @c sink::mode).
         *
         * Derived classes need to implement at least:
         * @code
         * class VISIBILITY derived_sink : public io::sink {
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
         * @see @ref example/core/iograph.cc
         * */
        class VIDRE_CORE_VISIBILITY sink : public endpoint::sink {
        public:
            sink() = default;

            sink(const sink &) = delete;
            sink &operator=(const sink &) = delete;
            sink(sink &&) = delete;
            sink &operator=(sink &&) = delete;

            virtual ~sink();

        protected:
            shared<endpoint::source> sink_predecessor_{};  ///< Connected io::source.
            shared<task> sink_task_{};                     ///< Thread handler if applicable.

        protected:
            /** The low-level write method (if applicable).
             * @param buf Memory view containing the data to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual offset_type write(const memory::view &buf) {  // FIXME why is this const memory::view & ?
                return invalid<offset_type>;
            }

        public:
            /** Write data from a memory view.
             * @param v Memory view containing the data to write.
             * @returns Number of bytes actually written (@c invalid for write failure).
             * */
            virtual result put(const memory::view &v) {
                result retval = write(v);

                // update stats
                stats_.bytes.transmitted += retval.count;
                if (not retval)
                    stats_.errors++;

                return retval;
            }

            /** Transfer a data of type @c T.
             * @tparam T Type of the value to write.
             * @param v Value to write (in binary form).
             * @returns Status of the write operation.
             * */
            template <typename T>
            requires (not std::derived_from<T, memory::view> && std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>)
            result put(const T &v) {
                static_assert(!std::is_pointer_v<T>, "put() does not support pointer types");
                return put({sizeof(T), reinterpret_cast<const memory::const_pointer>(&v)});
            }

            /** Transfer an array of type @c T.
             * @tparam T Type of the array elements.
             * @tparam N Number of elements in the array.
             * @param arr The array to write (in binary form).
             * @returns Status of the write operation.
             * */
            template <typename T, size_t N>
            requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
            result put(const T (&arr)[N]) {
                static_assert(!std::is_pointer_v<T>, "put() does not support arrays of pointer types");
                return put({sizeof(T) * N, reinterpret_cast<const memory::const_pointer>(arr)});
            }

            /** Push a data packet.
             * @param p Shared pointer to the packet to write.
             * @returns Status of the write operation.
             * */
            status put(shared<packet> p) override {
                throw exception::internal() << "method not implemented or capability erroneously advertised";
            }

        public:
            status unlink() override;
            bool isready(endpoint::dir direction = endpoint::dir::any) const noexcept override {
                if (!sink_predecessor_)
                    return false;

                if (!!(direction & endpoint::dir::source))
                    return sink_predecessor_->isready(endpoint::dir::source);

                return true;
            }

        protected:
            /** Default thread handler.
             * @param pause Pause duration in milliseconds (used, for example, when waiting for the source to become active).
             * @see endpoint::isactive()
             * @see sink_step()
             * */
            void sink_handler(shared<task> self, std::chrono::milliseconds pause = std::chrono::milliseconds(1));

        protected:
            status attach(shared<endpoint::source> src, cap::flag_type cap) override;
            status detach(const endpoint::source *ptr) override;
        };

        ///@}

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_TYPES_SINK_HH
