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
#ifndef VIDRE_APPENCODE_INPUT_MIDRAW_HH
#define VIDRE_APPENCODE_INPUT_MIDRAW_HH
#pragma once

#include <vidre/config.hh>
#include <vidre/ffmpeg/util/frame.hh>

#include <QFile>

namespace vidre {

    namespace input {

        /** Micron Imaging Device raw video reader.
         *
         * FIXME this is a very crude and quick implementation to read the MID/Aptina/OnSemi DevWareX raw videos!
         *
         */
        class midraw {
        public:
            midraw() noexcept = default;
            midraw(const std::filesystem::path &filename) noexcept : file_(filename) {}
            ~midraw() {}

        protected:
            QFile file_;

            unsigned width_ = 752;
            unsigned height_ = 480;
            unsigned bpp_ = 10;
            bool packed_ = true;
            bool mono_ = true;

            size_t framesize_ = 0;

            void (*decode_)(QFile &, AVFrame *, unsigned) = {};
            ffmpeg::frame frame_ = {};

        public:
            bool open();

            void close() {
                file_.close();
            }

            size_t size() {
                return file_.size();
            }

            void setformat(unsigned width, unsigned height, unsigned bpp, bool monochrome = true) {
                width_ = width;
                height_ = height;
                bpp_ = bpp;
                mono_ = monochrome;
            }

            bool check(bool full = false);

            bool eof() const noexcept {
                return file_.atEnd();
            }

            ffmpeg::frame &get_frame() {
                frame_.make_writable();
                decode_(file_, &frame_, framesize_);
                return frame_;
            }
        };

    }  // namespace input

}  // namespace vidre

#endif  // VIDRE_APPENCODE_INPUT_MIDRAW_HH
