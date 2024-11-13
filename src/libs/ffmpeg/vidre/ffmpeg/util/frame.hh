/* VidRe
 * Copyright (C) 2024 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_FFMPEG_UTIL_FRAME_HH
#define VIDRE_FFMPEG_UTIL_FRAME_HH
#pragma once

#include <vidre/ffmpeg/config.hh>
#include <vidre/ffmpeg/util/exception.hh>
#include <vidre/ffmpeg/util/pixelformat.hh>

extern "C" {
#include <libavutil/frame.h>
}

namespace vidre {

    namespace ffmpeg {
        /**@addtogroup api-support-ffmpeg-util-data
         * */
        ///@{

        /** Raw audio or video data.
         * @see [AVFrame etal.](https://ffmpeg.org/doxygen/trunk/group__lavu__frame.html)
         */
        class VIDRE_FFMPEG_VISIBILITY frame {
        public:
            // TODO AVFrameSideData
            // TODO enum AVFrameSideDataType etal.
            // TODO AVSideDataDescriptor
            // TODO AVRegionOfInterest

        protected:
            AVFrame *m_ = {};

        public:
            frame() noexcept = default;
            ~frame() {
                if (m_)
                    free();
            }

            /** Construct video output frame (with buffers).
             * @see [ffmpeg exmample mux.c](https://git.ffmpeg.org/gitweb/ffmpeg.git/blob/HEAD:/doc/examples/mux.c)
             * */
            frame(PixelFormat pix_fmt, int width, int height, int align = 0) {
                if (not allocate())
                    throw ffmpeg::exception() << boost::errinfo_errno(ENOMEM);  // FIXME

                m_->format = static_cast<int>(pix_fmt);
                m_->width = width;
                m_->height = height;

                auto ret = get_buffer(align);
                if (ret != errc{})
                    throw ffmpeg::exception() << ffmpeg::errc_info(ret);
            }

        public:
            /** Dereference the internal AVFrame.
             * */
            AVFrame *operator->() noexcept {
                return m_;
            }
            const AVFrame *operator->() const noexcept {
                return m_;
            }
            AVFrame *operator&() noexcept {
                return m_;
            }
            const AVFrame *operator&() const noexcept {
                return m_;
            }

            /** Allocate frame.
             * @returns ''true'' on success, ''false'' on allocation failure.
             * */
            bool allocate() noexcept {
                m_ = av_frame_alloc();
                return !!m_;
            }

            /** Free frame.
             * This method is automatically called from the destructor.
             * */
            void free() noexcept {
                av_frame_free(&m_);
            }

            /** Allocate new buffer(s) for audio or video data.
             * */
            errc get_buffer(int align = 0) noexcept {
                assert(m_ && (m_->buf == nullptr));
                return static_cast<errc>(av_frame_get_buffer(m_, align));
            }

            /** Check if the frame data is writable.
             * */
            bool is_writable() const noexcept {
                return av_frame_is_writable(m_);
            }

            /** Ensure that the frame data is writable, avoiding data copy if possible.
             * */
            errc make_writable() noexcept {
                return static_cast<errc>(av_frame_make_writable(m_));
            }
        };

        ///@}
    }  // namespace ffmpeg

}  // namespace vidre

#endif  // VIDRE_FFMPEG_UTIL_FRAME_HH
