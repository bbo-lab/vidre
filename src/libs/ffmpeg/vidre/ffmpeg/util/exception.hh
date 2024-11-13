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
#ifndef VIDRE_FFMPEG_EXCEPTION_HH
#define VIDRE_FFMPEG_EXCEPTION_HH
#pragma once

#include <vidre/ffmpeg/config.hh>
#include <vidre/exception.hh>

extern "C" {
#include <libavutil/error.h>
}

namespace vidre {

    namespace ffmpeg {
        /**@addtogroup api-support-ffmpeg-util-error
         * */
        ///@{

        /** FFmpeg exception.
         * */
        class VIDRE_FFMPEG_VISIBILITY exception : public virtual vidre::exception {
        public:
            class message;

        public:
            exception(const std::source_location location = std::source_location::current()) : vidre::exception(location) {}

        public:
            const char *what() const noexcept override;
        };

        /** FFmpeg error codes.
         * */
        enum class errc {
            bsf_not_found = AVERROR_BSF_NOT_FOUND,
            bug = AVERROR_BUG,
            buffer_too_small = AVERROR_BUFFER_TOO_SMALL
            //... TODO ...
        };

        /** FFmpeg error category.
         * */
        struct VIDRE_FFMPEG_VISIBILITY error_category : std::error_category {
            const char *name() const noexcept override;
            std::string message(int ev) const override;
        };

        /** Make an FFmpeg error code.
         * */
        std::error_code make_error_code(ffmpeg::errc e);

        inline std::error_code make_error_code(int e) {
            return make_error_code(static_cast<ffmpeg::errc>(e));
        }

        /** Error-code exception info.
         * */
        using errc_info = vidre::error_info<struct errc_info_, errc>;

        ///@}
    }  // namespace ffmpeg

}  // namespace vidre

namespace std {
    template <>
    struct is_error_code_enum<vidre::ffmpeg::errc> : true_type {};
}  // namespace std

namespace boost {
    inline std::string to_string(vidre::ffmpeg::errc_info const &e) {
        auto ecode = vidre::ffmpeg::make_error_code(e.value());
        return ecode.message();
    }
}  // namespace boost

#endif  // VIDRE_FFMPEG_EXCEPTION_HH
