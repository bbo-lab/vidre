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
#ifndef VIDRE_FFMPEG_CODEC_PACKET_HH
#define VIDRE_FFMPEG_CODEC_PACKET_HH
#pragma once

#include <vidre/ffmpeg/config.hh>
#include <vidre/ffmpeg/util/exception.hh>
#include <vidre/ffmpeg/util/dictionary.hh>

extern "C" {
#include <libavcodec/packet.h>
}

namespace vidre {

    namespace ffmpeg {

        /**@addtogroup api-support-ffmpeg-codec
         * */
        ///@{

        /** This structure stores compressed data.
         * @see [AVPacket etal.](https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html)
         */
        class VIDRE_FFMPEG_VISIBILITY packet {
        public:
            enum type {
                key = AV_PKT_FLAG_KEY,          ///< The packet contains a keyframe.
                corrupt = AV_PKT_FLAG_CORRUPT,  ///< The packet content is corrupted.
                discard = AV_PKT_FLAG_DISCARD,  ///< Flag is used to discard packets which are required to maintain valid decoder
                                                ///< state but are not required for output and should be dropped after decoding.
                trusted = AV_PKT_FLAG_TRUSTED,  ///< The packet comes from a trusted source.
                disposable = AV_PKT_FLAG_DISPOSABLE,  ///< Flag is used to indicate packets that contain frames that can be
                                                      ///< discarded by the decoder.
            };

        protected:
            AVPacket *m_ = {};

        public:
            packet() noexcept = default;
            ~packet() {
                // FIXME
            }

        public:
            /** Dereference AVPacket. */
            AVPacket *operator->() noexcept {
                return m_;
            }
            const AVPacket *operator->() const noexcept {
                return m_;
            }
            operator AVPacket *() noexcept {
                return m_;
            }
            operator const AVPacket *() const noexcept {
                return m_;
            }
            operator AVPacket **() noexcept {
                return &m_;
            }
        };

        ///@}

    }  // namespace ffmpeg

}  // namespace vidre

#endif  // VIDRE_FFMPEG_CODEC_PACKET_HH
