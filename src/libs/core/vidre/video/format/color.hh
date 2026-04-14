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

/**@addtogroup api-core-video-color
 * @ingroup api-core-video
 * @details
 * @fileinfo
 *
 * */

#include <vidre/core/config.hh>

namespace vidre {

    namespace detail {

        namespace video {

            /* see also http://vektor.theorem.ca/graphics/ycbcr/ */
            enum v4l2_colorspace {
                /*
                 * Default colorspace, i.e. let the driver figure it out.
                 * Can only be used with video capture.
                 */
                V4L2_COLORSPACE_DEFAULT = 0,

                /* SMPTE 170M: used for broadcast NTSC/PAL SDTV */
                V4L2_COLORSPACE_SMPTE170M = 1,

                /* Obsolete pre-1998 SMPTE 240M HDTV standard, superseded by Rec 709 */
                V4L2_COLORSPACE_SMPTE240M = 2,

                /* Rec.709: used for HDTV */
                V4L2_COLORSPACE_REC709 = 3,

                /*
                 * Deprecated, do not use. No driver will ever return this. This was
                 * based on a misunderstanding of the bt878 datasheet.
                 */
                V4L2_COLORSPACE_BT878 = 4,

                /*
                 * NTSC 1953 colorspace. This only makes sense when dealing with
                 * really, really old NTSC recordings. Superseded by SMPTE 170M.
                 */
                V4L2_COLORSPACE_470_SYSTEM_M = 5,

                /*
                 * EBU Tech 3213 PAL/SECAM colorspace.
                 */
                V4L2_COLORSPACE_470_SYSTEM_BG = 6,

                /*
                 * Effectively shorthand for V4L2_COLORSPACE_SRGB, V4L2_YCBCR_ENC_601
                 * and V4L2_QUANTIZATION_FULL_RANGE. To be used for (Motion-)JPEG.
                 */
                V4L2_COLORSPACE_JPEG = 7,

                /* For RGB colorspaces such as produces by most webcams. */
                V4L2_COLORSPACE_SRGB = 8,

                /* opRGB colorspace */
                V4L2_COLORSPACE_OPRGB = 9,

                /* BT.2020 colorspace, used for UHDTV. */
                V4L2_COLORSPACE_BT2020 = 10,

                /* Raw colorspace: for RAW unprocessed images */
                V4L2_COLORSPACE_RAW = 11,

                /* DCI-P3 colorspace, used by cinema projectors */
                V4L2_COLORSPACE_DCI_P3 = 12,
            };

/*
 * Determine how COLORSPACE_DEFAULT should map to a proper colorspace.
 * This depends on whether this is a SDTV image (use SMPTE 170M), an
 * HDTV image (use Rec. 709), or something else (use sRGB).
 */
#define V4L2_MAP_COLORSPACE_DEFAULT(is_sdtv, is_hdtv) \
    ((is_sdtv) ? V4L2_COLORSPACE_SMPTE170M : ((is_hdtv) ? V4L2_COLORSPACE_REC709 : V4L2_COLORSPACE_SRGB))

            enum v4l2_xfer_func {
                /*
                 * Mapping of V4L2_XFER_FUNC_DEFAULT to actual transfer functions
                 * for the various colorspaces:
                 *
                 * V4L2_COLORSPACE_SMPTE170M, V4L2_COLORSPACE_470_SYSTEM_M,
                 * V4L2_COLORSPACE_470_SYSTEM_BG, V4L2_COLORSPACE_REC709 and
                 * V4L2_COLORSPACE_BT2020: V4L2_XFER_FUNC_709
                 *
                 * V4L2_COLORSPACE_SRGB, V4L2_COLORSPACE_JPEG: V4L2_XFER_FUNC_SRGB
                 *
                 * V4L2_COLORSPACE_OPRGB: V4L2_XFER_FUNC_OPRGB
                 *
                 * V4L2_COLORSPACE_SMPTE240M: V4L2_XFER_FUNC_SMPTE240M
                 *
                 * V4L2_COLORSPACE_RAW: V4L2_XFER_FUNC_NONE
                 *
                 * V4L2_COLORSPACE_DCI_P3: V4L2_XFER_FUNC_DCI_P3
                 */
                V4L2_XFER_FUNC_DEFAULT = 0,
                V4L2_XFER_FUNC_709 = 1,
                V4L2_XFER_FUNC_SRGB = 2,
                V4L2_XFER_FUNC_OPRGB = 3,
                V4L2_XFER_FUNC_SMPTE240M = 4,
                V4L2_XFER_FUNC_NONE = 5,
                V4L2_XFER_FUNC_DCI_P3 = 6,
                V4L2_XFER_FUNC_SMPTE2084 = 7,
            };

/*
 * Determine how XFER_FUNC_DEFAULT should map to a proper transfer function.
 * This depends on the colorspace.
 */
#define V4L2_MAP_XFER_FUNC_DEFAULT(colsp)                                                                                 \
    ((colsp) == V4L2_COLORSPACE_OPRGB                                                                                     \
         ? V4L2_XFER_FUNC_OPRGB                                                                                           \
         : ((colsp) == V4L2_COLORSPACE_SMPTE240M                                                                          \
                ? V4L2_XFER_FUNC_SMPTE240M                                                                                \
                : ((colsp) == V4L2_COLORSPACE_DCI_P3                                                                      \
                       ? V4L2_XFER_FUNC_DCI_P3                                                                            \
                       : ((colsp) == V4L2_COLORSPACE_RAW                                                                  \
                              ? V4L2_XFER_FUNC_NONE                                                                       \
                              : ((colsp) == V4L2_COLORSPACE_SRGB || (colsp) == V4L2_COLORSPACE_JPEG ? V4L2_XFER_FUNC_SRGB \
                                                                                                    : V4L2_XFER_FUNC_709)))))

            enum v4l2_ycbcr_encoding {
                /*
                 * Mapping of V4L2_YCBCR_ENC_DEFAULT to actual encodings for the
                 * various colorspaces:
                 *
                 * V4L2_COLORSPACE_SMPTE170M, V4L2_COLORSPACE_470_SYSTEM_M,
                 * V4L2_COLORSPACE_470_SYSTEM_BG, V4L2_COLORSPACE_SRGB,
                 * V4L2_COLORSPACE_OPRGB and V4L2_COLORSPACE_JPEG: V4L2_YCBCR_ENC_601
                 *
                 * V4L2_COLORSPACE_REC709 and V4L2_COLORSPACE_DCI_P3: V4L2_YCBCR_ENC_709
                 *
                 * V4L2_COLORSPACE_BT2020: V4L2_YCBCR_ENC_BT2020
                 *
                 * V4L2_COLORSPACE_SMPTE240M: V4L2_YCBCR_ENC_SMPTE240M
                 */
                V4L2_YCBCR_ENC_DEFAULT = 0,

                /* ITU-R 601 -- SDTV */
                V4L2_YCBCR_ENC_601 = 1,

                /* Rec. 709 -- HDTV */
                V4L2_YCBCR_ENC_709 = 2,

                /* ITU-R 601/EN 61966-2-4 Extended Gamut -- SDTV */
                V4L2_YCBCR_ENC_XV601 = 3,

                /* Rec. 709/EN 61966-2-4 Extended Gamut -- HDTV */
                V4L2_YCBCR_ENC_XV709 = 4,

                /*
                 * sYCC (Y'CbCr encoding of sRGB), identical to ENC_601. It was added
                 * originally due to a misunderstanding of the sYCC standard. It should
                 * not be used, instead use V4L2_YCBCR_ENC_601.
                 */
                V4L2_YCBCR_ENC_SYCC = 5,

                /* BT.2020 Non-constant Luminance Y'CbCr */
                V4L2_YCBCR_ENC_BT2020 = 6,

                /* BT.2020 Constant Luminance Y'CbcCrc */
                V4L2_YCBCR_ENC_BT2020_CONST_LUM = 7,

                /* SMPTE 240M -- Obsolete HDTV */
                V4L2_YCBCR_ENC_SMPTE240M = 8,
            };

            /*
             * enum v4l2_hsv_encoding values should not collide with the ones from
             * enum v4l2_ycbcr_encoding.
             */
            enum v4l2_hsv_encoding {
                /* Hue mapped to 0 - 179 */
                V4L2_HSV_ENC_180 = 128,

                /* Hue mapped to 0-255 */
                V4L2_HSV_ENC_256 = 129,
            };

/*
 * Determine how YCBCR_ENC_DEFAULT should map to a proper Y'CbCr encoding.
 * This depends on the colorspace.
 */
#define V4L2_MAP_YCBCR_ENC_DEFAULT(colsp)                                     \
    (((colsp) == V4L2_COLORSPACE_REC709 || (colsp) == V4L2_COLORSPACE_DCI_P3) \
         ? V4L2_YCBCR_ENC_709                                                 \
         : ((colsp) == V4L2_COLORSPACE_BT2020                                 \
                ? V4L2_YCBCR_ENC_BT2020                                       \
                : ((colsp) == V4L2_COLORSPACE_SMPTE240M ? V4L2_YCBCR_ENC_SMPTE240M : V4L2_YCBCR_ENC_601)))

            enum v4l2_quantization {
                /*
                 * The default for R'G'B' quantization is always full range.
                 * For Y'CbCr the quantization is always limited range, except
                 * for COLORSPACE_JPEG: this is full range.
                 */
                V4L2_QUANTIZATION_DEFAULT = 0,
                V4L2_QUANTIZATION_FULL_RANGE = 1,
                V4L2_QUANTIZATION_LIM_RANGE = 2,
            };

/*
 * Determine how QUANTIZATION_DEFAULT should map to a proper quantization.
 * This depends on whether the image is RGB or not, the colorspace.
 * The Y'CbCr encoding is not used anymore, but is still there for backwards
 * compatibility.
 */
#define V4L2_MAP_QUANTIZATION_DEFAULT(is_rgb_or_hsv, colsp, ycbcr_enc) \
    (((is_rgb_or_hsv) || (colsp) == V4L2_COLORSPACE_JPEG) ? V4L2_QUANTIZATION_FULL_RANGE : V4L2_QUANTIZATION_LIM_RANGE)

/*
 * Deprecated names for opRGB colorspace (IEC 61966-2-5)
 *
 * WARNING: Please don't use these deprecated defines in your code, as
 * there is a chance we have to remove them in the future.
 */
#define V4L2_COLORSPACE_ADOBERGB V4L2_COLORSPACE_OPRGB
#define V4L2_XFER_FUNC_ADOBERGB  V4L2_XFER_FUNC_OPRGB

        }  // namespace video

    }  // namespace detail

    namespace video {

        /**@addtogroup api-core-video-color
         * */
        ///@{

        ///@}

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_FORMAT_COLOR_HH
