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
#include <vidre/io/types/source.hh>
#include <vidre/io/types/sink.hh>

#include <queue>
#include <mutex>

namespace vidre {

    namespace io {

        /** Fan-in and -out.
         * @ingroup api-core-io-types */
        namespace fan {

            /**@addtogroup api-core-io-types
             * */
            ///@{

            /** Fan-out endpoint.
             * This class represents an output endpoint that can fan out data from one source to many branches.
             * */
            class VIDRE_CORE_VISIBILITY out
                : public io::sink
                , public endpoint::source {
            public:
                class branch;

            public:
                out() = default;

                out(const out &) = delete;
                out &operator=(const out &) = delete;
                out(out &&) = delete;
                out &operator=(out &&) = delete;

                virtual ~out();

            protected:
                std::vector<shared<branch>> branches_{};  ///< The list of branches connected to this fan-out endpoint.

            public:
                cap getcap() const noexcept override {
                    return {
                        .source = {},  //
                        .sink = {}     //
                        // FIXME
                    };
                }

                status unlink() override;

                bool isready(endpoint::dir direction = endpoint::dir::any) const noexcept override;

            protected:
                status attach(shared<endpoint::source> src, cap::flag_type cap) override;
                status detach(const endpoint::source *ptr) override;
                
                status attach(shared<endpoint::sink> src, cap::flag_type cap) override;
                status detach(const endpoint::sink *ptr) override;
            };

            /** Branch of a fan-out endpoint.
             * This class represents a single branch that receives data from the fan-out endpoint.
             * */
            class VIDRE_CORE_VISIBILITY out::branch : public io::source {
            public:
            public:
                branch(fan::out &parent) : parent_(parent) {}

                branch(const branch &) = delete;
                branch &operator=(const branch &) = delete;
                branch(branch &&) = delete;
                branch &operator=(branch &&) = delete;

                virtual ~branch() {}

            private:
                fan::out &parent_;

            protected:
                std::queue<shared<packet>> queue_{};
                std::mutex queue_mutex_{};

            protected:
                status queue(shared<packet> pkt);

            public:
                cap getcap() const noexcept override {
                    return {
                        .source = {}
                        // FIXME
                    };
                }

                shared<packet> pull() override;

                status unlink() override;

                size_type queue_depth() const noexcept override {
                    if (parent_.sink_left)
                        return std::static_pointer_cast<io::source>(parent_.sink_left)->queue_depth();
                    return 0;
                }

            protected:
                void source_step() override;
                status attach(shared<endpoint::sink> snk, cap::flag_type cap) override;
                status detach(const endpoint::sink *ptr) override;
            };

            ///@}

        }  // namespace fan

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_TYPES_FAN_HH
