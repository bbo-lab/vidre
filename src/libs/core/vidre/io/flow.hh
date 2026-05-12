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
 * ## Overview
 * 
 * VidRe provides to major types of flow control: chunk-based and packet-based. The former is a simple stream 
 * of data, while the latter is a sequence of discrete packets, each with its own header and payload. The flow 
 * control API provides mechanisms for both types of data flow, allowing for flexible and efficient data 
 * transmission between sources, channels (which includes filters, mergers and arbiters) and sinks.
 * 
 * For graphical description, we will use the following symbols:
 *
 * @image html design/io-flow-symbols.svg
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
 * ## Packet-based data flow
 *
 *
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
