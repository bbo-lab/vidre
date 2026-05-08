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
#pragma once

/**@addtogroup api-core-io-flow
 * @brief Flow control.
 * @details
 * @fileinfo
 * 
 * Data transmision can be packet (see @ref api-core-io-packet) or chunk based (e.g., a video
 * stream is only packet based, while an audio device would be able to handle both modes).
 * For packet based I/O the data is either passed using event mechanisms (i.e. either the
 * source or the sink wait for the other to signal the availability of data or readiness to
 * receive data) or by direct calls (i.e. the source directly calls the sink's put method,
 * or the sink directly calls the source's get method). For chunk based I/O only the latter
 * is available.
 *
 * ## Packet-based data flow
 *
 * 
 *
 * ## Chunk-based data flow
 *
 *
 *
 * @{
 * @namespace vidre::io::flow  Flow control.
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/io/types.hh>

#include <ankerl/unordered_dense.h>

namespace vidre {

    namespace io {

        namespace flow {

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
                /** Item.
                 * */
                struct VIDRE_CORE_VISIBILITY item {
                    endpoint::shared ep;  ///< Endpoint (source, sink, or channel).
                    task::shared task;    ///< Task (thread) for this item.
                };

                using map_type = ankerl::unordered_dense::map<const char *, item>;

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
                std::string tag_{};  ///< Route tag (used by dashboard).
                map_type items_;     ///< Map of items in this route, indexed by endpoint tag.

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

            //@todo implement "router"
            //@todo implement "dashboard"

        }  // namespace flow

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_FLOW_HH
