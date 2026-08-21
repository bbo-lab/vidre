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
#ifndef VIDRE_CORE_IO_FLOW_HH
#define VIDRE_CORE_IO_FLOW_HH
#include <vector>
#pragma once

/**@addtogroup api-core-io-flow
 * @brief Flow control.
 * @details
 * @fileinfo
 *
 * ## Packet-based data flow
 *
 * As mentioned above, packet-based data flow is graph-based, meaning that data can flow through complex routes. 
 * In contrast to chunk-based flow, packet-based flow is generally asynchronous, and, therefore, has multiple "drivers",
 * which are usually implemented as tasks. The following example illustrates this with a simple camera-to-storage (expected to 
 * have no frame loss) and camera-to-display flow (which only updates the display with the latest camera frame when ready), 
 * while also including messages logger (which can include user messages), also streaming its packets to a remote server:
 *
 * @image html design/io-packet-complex.png
 *
 * 1. The *camera* (source) captures frames, running in a task, only _notifies_ the receiver when a new frame is available.
 *    The implementation does not guarantee that all frames are transmitted, only that the cameras internal ring buffer
 *    stores a certain amount of frames (at least 1 sec).
 *
 *    The associated *arbiter* polls the camera for video packets, which is always only the latest camera frame. It's
 *    storage behavior matches that of the source, here, the camera, meaning that it only stores the latest frame and
 *    drops all previous frames.
 *
 * 2. The *logger* (also a source) on the other hand _pushes_ log messages to the next endpoint.
 *
 *    The associated *arbiter* then _queues_ the log messages.
 *
 * 3. The *arbiters* are connected to where the output behavior depends on the sink type:
 *    a) The *transcoder*, converting video formats, requires messages to be _pushed_ through, while the
 *       the arbiter _waits_ for the transcoder (note, which includes the transfer to the *display*)
 *       is finished.
 *    b) a *merger* for file storage and messages from the *logger*
 *    c) a log message *sender*, which, for example, sends the log messages to a remote server.
 *
 *
 *
 *
 *
 *
 * ## Overview
 *
 * VidRe provides to major types of flow control: chunk-based and packet-based. The former is a simple stream
 * of data, while the latter is a sequence of discrete packets, each with its own header and payload. The flow
 * control API provides mechanisms for both types of data flow, allowing for flexible and efficient data
 * transmission between sources, channels (which includes filters, mergers and arbiters) and sinks.
 *
 * For graphical description, we will use the following symbols:
 *
 * @image html design/io-flow-symbols.png
 *
 * The **source** can be, for example, a simple file (only providing chunk-based flow, i.e., a stream of bytes;
 * in contrast to `storage::raw`, which provides packet-based data flow only), or a camera (providing packet-based
 * flow, i.e., discrete frames with headers). The **sink** can be, for example, a file (again the distinction
 * between a simple, chunk-based file or packet-based raw storage) or a video display (requiring packet-based flow).
 *
 * **Channel**'s, on the other hand, can be, for example, a filter (which processes the data in some way), a merger
 * (which combines multiple sources into one) or an arbiter (which channels the data to multiple sinks).
 *
 * ## Chunk-based data flow
 *
 * Chunk-based data flow is generally only linear (compared to graph-based packet flow).
 *
 * The most simple path is either a source pushing data to a sink,
 * @image html design/io-chunk-push.png
 * or a sink pulling data from a source. In both cases, the data is transmitted as a stream of bytes,
 * @image html design/io-chunk-pull.png
 * without any discrete packets or headers. This is suitable for simple file I/O or streaming
 * applications where the data can be processed sequentially.
 *
 * Chunked data can still be filtered (such as character encoding conversion),
 * @image html design/io-chunk-filter.png
 * for example to convert between different character encodings (e.g., UTF-8 to UTF-16) or to apply compression.
 * However, chunk-based data flow is purely linear, meaning that it does not support branching or merging of data streams.
 * As well, tasked background operation is not supported. The data transmission is synchronous with a single "driver" (either
 * the source pushes or the next endpoint pulls).
 *
 * @note Chunk-based data flow has a "single" driver, i.e., either the source pushes or the sink pulls. In case of channels,
 *       it is always the channel that pulls, and, at the same time, pushes data to the next endpoint. Depending on the capabilites,
 *       *pull* is preferred.
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/io/types.hh>

#include <ankerl/unordered_dense.h>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-flow
         * */
        ///@{

        /** Source to sink route.
         *
         * This class handles the connection between a source and a sink, including
         * intermediate channels.
         *
         * FIXME
         *
         * */
        class VIDRE_CORE_VISIBILITY route {
        public:
            template <typename Key, typename Value>
            using map_type = ankerl::unordered_dense::map<Key, Value>;

        public:
            route(std::string tag = {}) : tag_(std::move(tag)) {}
            route(const route &) = delete;
            route &operator=(const route &) = delete;
            route(route &&) = default;
            route &operator=(route &&) = default;
            ~route() {
                // FIXME
            }

            /** Simple source to sink route.
             * */
            route(endpoint::shared src, endpoint::shared snk, std::string tag = {});

        protected:
            std::string tag_{};                   ///< Route tag (used by dashboard).
            std::vector<endpoint::shared> ep_{};  ///< Endpoint list on this "route".

#if 0
        protected:
            /** Match capabilities between two endpoint types.
             * @param from The source endpoint type.
             * @param to The sink endpoint type.
             * @returns @c true if the capabilities are compatible, @c false otherwise.
             * */
            static bool match(cap::flag_type &from, cap::flag_type &to) noexcept;

        public:
            /** Match capabilities between two endpoint types.
             * @tparam From The source endpoint type.
             * @tparam To The sink endpoint type.
             * @param from The source endpoint instance.
             * @param to The sink endpoint instance.
             * @returns @c true if the capabilities are compatible, @c false otherwise.
             * */
            template <class From, class To>
            requires (isa_v<From> && isa_v<To>)
            static bool match(From *from, To *to) {
                if (not from || not to)
                    throw exception::logic() << "invalid endpoint(s)";

                // reset cap_ before matching
                from->cap_[endpoint::source] = from->getcap()[endpoint::source];
                to->cap_[endpoint::sink] = to->getcap()[endpoint::sink];

                return match(from->cap_[endpoint::source], to->cap_[endpoint::sink]);
            }

            /** Match capabilities between two endpoint types.
             * @tparam From The source endpoint type.
             * @tparam To The sink endpoint type.
             * @param from The source endpoint instance.
             * @param to The sink endpoint instance.
             * @returns @c true if the capabilities are compatible, @c false otherwise.
             * */
            template <class From, class To>
            requires (isa_v<From> && isa_v<To>)
            static bool match(std::shared_ptr<From> from, std::shared_ptr<To> to) {
                return match(from.get(), to.get());
            }
#endif
        public:
            /** Get endpoint tag.
             * @returns The tag of this endpoint.
             * */
            const std::string &tag() const noexcept {
                return tag_;
            }

            /** Set enpoint tag.
             * @param tag The tag to set for this endpoint.
             * */
            void tag(const std::string &tg) noexcept {
                tag_ = tg;
            }

        public:
            /** Execute the route.
             * */
            void execute();
        };

        /** FIXME
         * */
        class VIDRE_CORE_VISIBILITY dashboard {
        public:
            dashboard() = default;
            dashboard(const dashboard &) = delete;
            dashboard &operator=(const dashboard &) = delete;
            dashboard(dashboard &&) = default;
            dashboard &operator=(dashboard &&) = default;
            ~dashboard() = default;
        };

        ///@}

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_FLOW_HH
