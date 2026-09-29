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
#ifndef VIDRE_CORE_IO_TYPES_CHANNEL_HH
#define VIDRE_CORE_IO_TYPES_CHANNEL_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/io/endpoint/sink.hh>
#include <vidre/io/endpoint/source.hh>

namespace vidre {

    namespace io {

        /**@addtogroup api-core-io-endpoint
         * */
        ///@{

        /** A channel for transforming and relaying packets between a sink and a source.
         *
         * The channel acts as an intermediary that can transform packets before
         * relaying them from the sink to the source. The default implementation
         * only relays data directly from the source to the sink (without any
         * transformation).
         *
         * By default, channels always use the push method on the sink side.
         *
         * @todo example filter implementation.
         * @see @ref example/core/iograph.cc
         * */
        class VIDRE_CORE_VISIBILITY channel
            : public sink
            , public source {
        public:
            channel() = default;

            channel(const channel &) = delete;
            channel &operator=(const channel &) = delete;
            channel(channel &&) = delete;
            channel &operator=(channel &&) = delete;

            virtual ~channel();

        public:
            /** Default channel capabilities.
             * */
            cap getcap() const noexcept override {
                return {                                                 //
                        .source = cap::packet | cap::push | cap::event,  //
                        .sink = cap::packet | cap::push};
            }

            /** Default implementation for the push (i.e. transform) method.
             * */
            result push(shared<packet> p) override;

        public:
            size_type queue_depth() const noexcept override {
                if (sink_left_)
                    return std::static_pointer_cast<io::source>(sink_left_)->queue_depth();
                return 0;
            }

        public:
            status unlink() override;
            bool isready(endpoint::dir direction = endpoint::dir::any) const noexcept override {
                if (!source_right_)
                    return false;

                if (!sink_left_)
                    return false;

                auto retval = true;
                if (!!(direction & endpoint::dir::sink))
                    retval &= source_right_->isready(endpoint::dir::sink);

                if (retval and !!(direction & endpoint::dir::source))
                    retval &= sink_left_->isready(endpoint::dir::source);

                return retval;
            }

        protected:
            // status attach(shared<io::source> src, cap::flag_type cap) override;
            // status attach(shared<io::sink> src, cap::flag_type cap) override;
            // status detach(const io::source *ptr) override;
            // status detach(const io::sink *ptr) override;
        };

        ///@}

    }  // namespace io

}  // namespace vidre
#endif  // VIDRE_CORE_IO_TYPES_CHANNEL_HH
