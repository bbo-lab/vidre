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
#ifndef VIDRE_CORE_VIDEO_FORMAT_COLOR_HH
#define VIDRE_CORE_VIDEO_FORMAT_COLOR_HH
#pragma once

/**@addtogroup api-video-format-color
 * @ingroup api-video-format
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>
#include <vidre/settings/json.hh>

namespace vidre {

    namespace video {

        namespace format {

            /**@addtogroup api-video-format-color
             * */
            ///@{

            /** Color information.
             * @note This information is targeted for use with FFmpeg and should be used for viewing purposes only.
             *       Otherwise, as VidRe is intended for scientific use, raw data should be used as far as possible.
             * */
            class __vidre_packed__(4) VIDRE_VIDEO_VISIBILITY color {
            public:
                /** Color primaries.
                 * */
                class __vidre_packed__(1) VIDRE_VIDEO_VISIBILITY primary {
                public:
                    enum : uint8_t {
                        none = 0,   ///< No or unknown primary.
                        bt709,      ///< BT.709.
                        bt470m,     ///< BT.470M.
                        bt470bg,    ///< BT.470BG or BT.601-6 625.
                        smpte170m,  ///< SMPTE-170M or BT.601-6 525.
                        smpte240m,  ///< SMPTE-240M.
                        film,       ///< Film.
                        smpte431,   ///< SMPTE-431.
                        smpte432,   ///< SMPTE-432.
                        bt2020,     ///< BT.2020.
                        jedec_p22   ///< JEDEC P22 phosphors.
                    };

                public:
                    primary() noexcept = default;

                protected:
                    uint8_t id_{};

                public:
                    /** Brief color primary description.
                     * */
                    const char *name() const noexcept;
                };

                /** Colorspace.
                 * */
                class __vidre_packed__(1) VIDRE_VIDEO_VISIBILITY space {
                public:
                    enum : uint8_t {
                        none = 0,   ///< No or unknown color space.
                        bt709,      ///< BT.709 color space, used for HDTV video.
                        fcc,        ///< FCC Code of Federal Regulations (CFR) Title 47 (2003) 73.682(a).
                        bt470bg,    ///< BT.470BG or BT.601-6 625.
                        smpte170m,  ///< SMPTE-170M or BT.601-6 525.
                        smpte240m,  ///< SMPTE-240M.
                        ycgco,      ///< YCgCo color space, used for lossless RGB encoding in JPEG XR and HEVC.
                        bt2020ncl,  ///< BT.2020 non-constant luminance.
                    };

                public:
                    space() noexcept = default;

                protected:
                    uint8_t id_{};

                public:
                    /** Brief color space description.
                     * */
                    const char *name() const noexcept;
                };

                /** Transfer function.
                 * */
                class __vidre_packed__(1) VIDRE_VIDEO_VISIBILITY transfer {
                public:
                    enum : uint8_t {
                        none = 0,      ///< No or unknown transform function.
                        bt709,         ///< BT.709
                        bt470m,        ///< BT.470M
                        bt470bg,       ///< BT.470BG
                        gamma22,       ///< Constant gamma of 2.2
                        gamma28,       ///< Constant gamma of 2.8
                        smpte170m,     ///< SMPTE-170M, BT.601-6 625 or BT.601-6 525
                        smpte240m,     ///< SMPTE-240M
                        srgb,          ///< SRGB
                        iec61966_2_1,  ///< IEC 61966-2-1
                        iec61966_2_4,  ///< IEC 61966-2-4
                        xvycc,         ///< xvycc
                        bt2020_10,     ///< BT.2020 for 10-bits content
                        bt2020_12,     ///< BT.2020 for 12-bits content
                    };

                public:
                    transfer() noexcept = default;

                protected:
                    uint8_t id_{};

                public:
                    /** Brief transfer function description.
                     * */
                    const char *name() const noexcept;
                };

                /** Quantization.
                 * */
                class __vidre_packed__(1) VIDRE_VIDEO_VISIBILITY quantization {
                public:
                    enum : uint8_t {
                        none,  ///< No or unknown quantization range (assume full range).
                        tv,    ///< TV (restricted) range
                        mpeg,  ///< MPEG (restricted) range
                        pc,    ///< PC (full) range
                        jpeg   ///< JPEG (full) range
                    };

                public:
                    quantization() noexcept = default;

                protected:
                    uint8_t id_{};

                public:
                    /** Brief quantization description.
                     * */
                    const char *name() const noexcept;
                };

            public:
                color() noexcept = default;

            protected:
                primary primary_{};
                space space_{};
                transfer transfer_{};
                quantization quantization_{};

            public:
                /** Get color information as JSON.
                 * */
                settings::json json() const noexcept;
            };

            ///@}

        }  // namespace format

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_FORMAT_COLOR_HH
