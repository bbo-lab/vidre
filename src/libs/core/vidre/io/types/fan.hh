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
#ifndef VIDRE_CORE_IO_TYPES_FAN_HH
#define VIDRE_CORE_IO_TYPES_FAN_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/io/types/endpoint.hh>
#include <vidre/io/types/sink.hh>
#include <vidre/io/types/source.hh>

namespace vidre {

    namespace io {

        /** Fan-in and -out.
         * @ingroup api-core-io-types */
        namespace fan {

            /**@addtogroup api-core-io-types
             * */
            ///@{

            class VIDRE_CORE_VISIBILITY out
                : public io::sink
                , public io::source {
            public:
                class branch;

            public:
                out() = default;

                out(const out &) = delete;
                out &operator=(const out &) = delete;
                out(out &&) = delete;
                out &operator=(out &&) = delete;

                virtual ~out() {}

            protected:
            public:
                status unlink() override {
                    return status::bad;
                }
                bool isready(endpoint::dir direction = endpoint::dir::any) const noexcept override {
                    // FIXME
                    return false;
                }
            };

            class VIDRE_CORE_VISIBILITY out::branch : public io::source {
            public:
            public:
                branch() = default;

                branch(const branch &) = delete;
                branch &operator=(const branch &) = delete;
                branch(branch &&) = delete;
                branch &operator=(branch &&) = delete;

                virtual ~branch() {}

            protected:
            public:
                status unlink() override {
                    return status::bad;
                }
            };

            ///@}

        }  // namespace fan

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_TYPES_FAN_HH
