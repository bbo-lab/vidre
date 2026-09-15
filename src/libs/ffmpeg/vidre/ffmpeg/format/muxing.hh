/* VidRe
 * Copyright (C) 2024-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_FFMPEG_FORMAT_MUXING_HH
#define VIDRE_FFMPEG_FORMAT_MUXING_HH
#pragma once

#include <vidre/ffmpeg/config.hh>
#include <vidre/ffmpeg/util/exception.hh>
#include <vidre/ffmpeg/util/dictionary.hh>
#include <vidre/ffmpeg/codec/packet.hh>

#include <filesystem>

extern "C" {
#include <libavformat/avformat.h>
}

namespace vidre {

    namespace ffmpeg {

        /** Format namespace (muxing/demuxing etc.).
         * @ingroup api-support-ffmpeg-format
         */
        namespace format {
            /**@addtogroup api-support-ffmpeg-format-muxing
             * */
            ///@{

            /** Output format.
             * */
            class VIDRE_FFMPEG_VISIBILITY output {
            public:
                /** Guess output format.
                 * */
                // FIXME output guess();

            protected:
                const AVOutputFormat *m_ = {};

            public:
                output() noexcept = default;
                output(const AVOutputFormat *ptr) : m_(ptr) {}

            public:
                /** Dereference AVFormatContext.
                 * */
                const AVOutputFormat *operator->() const noexcept {
                    return m_;
                }
                const AVOutputFormat *operator&() const noexcept {
                    return m_;
                }
            };

            /** Format I/O context for muxing.
             * @see [AVFormatContext](https://ffmpeg.org/doxygen/trunk/structAVFormatContext.html)
             */
            class VIDRE_FFMPEG_VISIBILITY context {
            public:
            protected:
                AVFormatContext *m_ = {};

            public:
                context() noexcept = default;
                ~context() {
                    // FIXME
                }

            public:
                /** Dereference AVFormatContext.
                 * */
                AVFormatContext *operator->() noexcept {
                    return m_;
                }
                const AVFormatContext *operator->() const noexcept {
                    return m_;
                }
                AVFormatContext *operator&() noexcept {
                    return m_;
                }
                const AVFormatContext *operator&() const noexcept {
                    return m_;
                }

                // TODO list formats

                /** Check if valid.
                 * */
                bool valid() const noexcept {
                    return m_;
                }

                /** Allocate context.
                 * */
                errc allocate(const std::filesystem::path &filename) noexcept {
                    return static_cast<errc>(avformat_alloc_output_context2(&m_, nullptr, nullptr, filename.c_str()));
                }

                /** Allocate context.
                 * */
                errc allocate(const std::filesystem::path &filename, const std::string format) noexcept {
                    return static_cast<errc>(avformat_alloc_output_context2(&m_, nullptr, format.c_str(), filename.c_str()));
                }

                /** Free context.
                 * */
                void free() noexcept {
                    if (m_) {
                        avformat_free_context(m_);
                        m_ = {};
                    }
                }

                /** Allocate the stream private data and write the stream header to an output media file.
                 * */
                errc write_header(dictionary dict = {}) {
                    return static_cast<errc>(avformat_write_header(m_, &(&dict)));
                }

                /** Write trailer.
                 * */
                errc write_trailer() {
                    return static_cast<errc>(av_write_trailer(m_));
                }

                /** Get output format.
                 * */
                output output_format() noexcept {
                    return m_->oformat;
                }

                /** Write a packet to an output media file.
                 * */
                errc write_packet(packet &p) noexcept {
                    return static_cast<errc>(av_write_frame(m_, p));
                }
            };

            ///@}
        }  // namespace format

    }  // namespace ffmpeg

}  // namespace vidre

#endif  // VIDRE_FFMPEG_FORMAT_MUXING_HH
