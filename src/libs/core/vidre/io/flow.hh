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
                route(std::string name = {}) : name_(std::move(name)) {}
                route(const route &) = delete;
                route &operator=(const route &) = delete;
                route(route &&) = default;
                route &operator=(route &&) = default;
                ~route() {
                    // FIXME
                }

                route(source::shared src, sink::shared snk, std::string name = {}) : name_(std::move(name)) {
                    // FIXME
                }

            protected:
                std::string name_{};  ///< Route name (used by dashboard).

            public:
                /** Get endpoint name.
                 * @returns The name of this endpoint.
                 * */
                const std::string &name() const noexcept {
                    return name_;
                }

                /** Set enpoint name.
                 * @param name The name to set for this endpoint.
                 * */
                void name(const std::string &nm) noexcept {
                    name_ = nm;
                }

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
