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
#ifndef VIDRE_CORE_IO_TYPES_MONITOR_HH
#define VIDRE_CORE_IO_TYPES_MONITOR_HH
#include <chrono>
#pragma once

/**@addtogroup api-core-io-monitor
 * @brief I/O monitoring (such as current and commulative endpoint throughput).
 * @details
 * @fileinfo
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/io/types/basic.hh>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-monitor
         * */
        ///@{

        class VIDRE_CORE_VISIBILITY monitor {
        public:
            /** Stats info (measurements per second).
             * */
            struct VIDRE_CORE_VISIBILITY stats {
                struct {
                    double transmitted{};  ///< Total number of packets that were available for transmission.
                    double dropped{};      ///< Total number of packets dropped by this endpoint.
                } packets;

                struct {
                    double transmitted{};  ///< Total number of bytes that were available for transmission.
                } bytes;

                double errors{};  ///< Total number of transmission errors encountered by this endpoint.
            };

            /** Calculate stats (units (1/s)).
             * */
            static monitor::stats calculate(endpoint *ep, chrono::timer::time_point tp = chrono::monotonic::now()) noexcept {
                auto s = ep->getstats();
                double t = std::chrono::duration_cast<chrono::timer::seconds>(tp - s.stamp).count();
                    // FIXME ^--- this could be a helper in "our" chrono namespace...

                return {
                    //
                    .packets =
                        {
                            //
                            .transmitted = double(s.packets.transmitted) / t,  //
                            .dropped = double(s.packets.dropped) / t           //
                        },                                                     //
                    .bytes =
                        {
                            //
                            .transmitted = double(s.bytes.transmitted) / t,  //
                        }  //
                };
            }

        public:
            monitor() = default;
            ~monitor();
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_TYPES_MONITOR_HH
