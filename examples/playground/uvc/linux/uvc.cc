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
#include "uvc.hh"

#include <vidre/util/macros.hh>

#include <linux/videodev2.h>
#include <linux/media.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

using namespace vidre;

// V4L2 =======================================================================

#if VIDRE_DEVELOP
#define UVC_PIX_FMT(pixfmt) #pixfmt, pixfmt
#else
#define UVC_PIX_FMT(pixfmt) pixfmt
#endif

struct {
#if VIDRE_DEVELOP
    const char *v4l2name;
#endif
    uint32_t uvc;
    video::pixel_format::type we;
} pixfmt_mapping[] = {
    // clang-format off
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGB332), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGB444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ARGB444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XRGB444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGBA444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGBX444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ABGR444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XBGR444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGRA444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGRX444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGB555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ARGB555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XRGB555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGBA555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGBX555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ABGR555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XBGR555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGRA555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGRX555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGB565), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGB555X), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ARGB555X), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XRGB555X), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGB565X), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGR666), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGR24), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGB24), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGR32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ABGR32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XBGR32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGRA32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGRX32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGB32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGBA32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGBX32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ARGB32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XRGB32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGBX1010102), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RGBA1010102), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ARGB2101010), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_BGR48_12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ABGR64_12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_GREY), video::pixel_format::mono8},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y4), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y6), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y012), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y14), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y16_BE), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y10BPACK), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y10P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_IPU3_Y10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_PAL8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_UV8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUYV), video::pixel_format::yuyv422},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YYUV), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YVYU), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_UYVY), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VYUY), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y41P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV444), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV555), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV565), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV24), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_AYUV32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XYUV32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VUYA32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VUYX32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUVA32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUVX32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_M420), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV48_12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y210), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y212), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y216), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV21), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV61), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV24), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV42), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_P010), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_P012), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV21M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV16M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV61M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_P012M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV410), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YVU410), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV411P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV420), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YVU420), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV422P), video::pixel_format::yuv422p},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV420M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YVU420M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV422M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YVU422M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YUV444M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_YVU444M), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12_4L4), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12_16L16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12_32L32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV15_4L4), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_P010_4L4), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12_8L128), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12_10BE_8L128), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12MT), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12MT_16X16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12M_8L128), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_NV12M_10BE_8L128), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR10P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG10P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG10P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB10P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR10ALAW8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG10ALAW8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG10ALAW8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB10ALAW8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR10DPCM8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG10DPCM8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG10DPCM8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB10DPCM8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR12P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG12P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG12P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB12P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR14), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG14), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG14), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB14), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR14P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG14P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG14P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB14P), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SBGGR16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGBRG16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SGRBG16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SRGGB16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_HSV24), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_HSV32), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MJPEG), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_JPEG), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_DV), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MPEG), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_H264), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_H264_NO_SC), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_H264_MVC), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_H263), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MPEG1), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MPEG2), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MPEG2_SLICE), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MPEG4), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_XVID), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VC1_ANNEX_G), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VC1_ANNEX_L), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VP8), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VP8_FRAME), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VP9), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_VP9_FRAME), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_HEVC), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_FWHT), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_FWHT_STATELESS), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_H264_SLICE), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_HEVC_SLICE), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_AV1_FRAME), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SPK), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RV30), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_RV40), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_CPIA1), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_WNVA), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SN9C10X), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SN9C20X_I420), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_PWC1), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_PWC2), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_ET61X251), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SPCA501), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SPCA505), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SPCA508), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SPCA561), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_PAC207), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MR97310A), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_JL2005BCD), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SN9C2028), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SQ905C), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_PJPG), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_OV511), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_OV518), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_STV0680), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_TM6000), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_CIT_YYVYUY), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_KONICA420), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_JPGL), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SE401), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_S5C_UYVY_JPG), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y8I), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Y12I), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_Z16), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MT21C), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MM21), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MT2110T), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_MT2110R), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_INZI), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_CNF4), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_HI240), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_QC08C), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_QC10C), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_AJPG), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_HEXTILE), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_IPU3_SBGGR10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_IPU3_SGBRG10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_IPU3_SGRBG10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_IPU3_SRGGB10), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_PRIV_MAGIC), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_FLAG_PREMUL_ALPHA), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_FLAG_SET_CSC), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_HM12), video::pixel_format::undefined},
    { UVC_PIX_FMT(V4L2_PIX_FMT_SUNXI_TILED_NV12), video::pixel_format::undefined}
    // clang-format on
};

bool uvc::query_stream(int fd) {
    log::trace tr;

    if (VIDRE_DEVELOP)
        tr << "querying camera " << dev_stream_.c_str() << "...\n";

    log::debug() << "Querying camera " << dev_stream_.c_str() << "...\n";

    v4l2_fmtdesc pixfmt{.index = static_cast<uint32_t>(-1),  //
                        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE};
    while (true) {
        pixfmt.index++;
        int retval = ioctl(fd, VIDIOC_ENUM_FMT, &pixfmt);
        if (retval < 0) {
            switch (errno) {
                case EINVAL:
                    break;
                default:
                    log::error() << "Querying pixel formats failed: " << std::strerror(errno) << log::endl;
                    return false;
            }

            break;
        }

        // supported by VidRe?
        for (auto &p : pixfmt_mapping) {
            if (p.uvc == pixfmt.pixelformat) {
                tr << "-> pixel format found: " << (VIDRE_DEVELOP ? p.v4l2name : (const char *)pixfmt.description) << log::endl;

                // create configuration
                settings::json pixfmt_config = {
                    {"supported", p.we},
                    {"pixel_format", std::string((VIDRE_DEVELOP ? p.v4l2name : (const char *)pixfmt.description))},
                    {"compressed", (pixfmt.flags & V4L2_FMT_FLAG_COMPRESSED) != 0},
                    {"emulated", (pixfmt.flags & V4L2_FMT_FLAG_EMULATED) != 0},
                    {"continuous_bytestream", (pixfmt.flags & V4L2_FMT_FLAG_CONTINUOUS_BYTESTREAM) != 0},
                    {"dynamic_resolution", (pixfmt.flags & V4L2_FMT_FLAG_DYN_RESOLUTION) != 0},
                    {"enc_cap_frame_interval", (pixfmt.flags & V4L2_FMT_FLAG_ENC_CAP_FRAME_INTERVAL) != 0},
                    {"csc_colorspace", (pixfmt.flags & V4L2_FMT_FLAG_CSC_COLORSPACE) != 0},
                    {"csc_xfer_func", (pixfmt.flags & V4L2_FMT_FLAG_CSC_XFER_FUNC) != 0},
                    {"csc_hsv_enc", (pixfmt.flags & V4L2_FMT_FLAG_CSC_HSV_ENC) != 0},
                    {"csc_quantization", (pixfmt.flags & V4L2_FMT_FLAG_CSC_QUANTIZATION) != 0},
                };

                // query frame sizes for this pixel format
                settings::json frm_config;

                v4l2_frmsizeenum frm{.index = static_cast<uint32_t>(-1),  //
                                     .pixel_format = pixfmt.pixelformat};
                while (true) {
                    frm.index++;
                    int retval = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frm);
                    if (retval < 0) {
                        if (errno == EINVAL) {
                        }
                        else {
                            log::error() << "Querying frame sizes failed: " << std::strerror(errno) << log::endl;
                            state_ &= ~device::state::available;
                        }

                        break;
                    }

                    switch (frm.type) {
                        case V4L2_FRMSIZE_TYPE_DISCRETE: {
                            tr << "  -> frame size found: " << frm.discrete.width << "x" << frm.discrete.height << log::endl;
                            frm_config.push_back({
                                {"type", "discrete"},
                                {"width", frm.discrete.width},
                                {"height", frm.discrete.height},
                            });
                            break;
                        }
                        case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                        case V4L2_FRMSIZE_TYPE_STEPWISE: {
                            tr << "  -> frame size range found: " << frm.stepwise.min_width << "x" << frm.stepwise.min_height
                               << " - " << frm.stepwise.max_width << "x" << frm.stepwise.max_height
                               << " (step: " << frm.stepwise.step_width << "x" << frm.stepwise.step_height << ")" << log::endl;
                            frm_config.push_back({
                                {"type", frm.type == V4L2_FRMSIZE_TYPE_CONTINUOUS ? "continuous" : "stepwise"},
                                {"min_width", frm.stepwise.min_width},
                                {"min_height", frm.stepwise.min_height},
                                {"max_width", frm.stepwise.max_width},
                                {"max_height", frm.stepwise.max_height},
                                {"step_width", frm.stepwise.step_width},
                                {"step_height", frm.stepwise.step_height},
                            });
                            break;
                        }
                        default:
                            log::error() << "Unknown frame size type: " << frm.type << log::endl;
                            break;
                    }
                }

                pixfmt_config["framesizes"] = frm_config;

                auto &cfg = info_["capabilities"];
                cfg.push_back(pixfmt_config);
            }
        }
    }

    return false;
}

void uvc::query() {
    log::trace tr;

    // open video stream device and check capabilities
    storage::file::io stream(dev_stream_,
                             storage::file::io::readwrite | storage::file::io::non_block | storage::file::io::close_on_exec);
    if (!stream.valid()) {
        state_ &= ~device::state::available;
        return;
    }

    v4l2_capability cap;
    if ((ioctl(stream, VIDIOC_QUERYCAP, &cap) < 0) or not(cap.device_caps & V4L2_CAP_STREAMING)) {
        state_ &= ~device::state::available;
        return;
    }

    // query streaming camera
    info_["name"] = std::string((const char *)cap.card);
    // FIXME ...
    info_["capabilities"] = settings::json::array();

    if (not query_stream(stream)) {
        state_ &= ~device::state::available;
        return;
    }

    //     // open metadata stream device and query capabilities
    //     storage::file::io meta(dev_meta_, storage::file::io::readonly | storage::file::io::non_block |
    //     storage::file::io::close_on_exec); if (meta.valid()) {
    //         if ((ioctl(meta, VIDIOC_QUERYCAP, &cap_[1]) < 0) or not(cap_[1].device_caps & V4L2_CAP_META_CAPTURE)) {
    //             meta.close();
    //         }

    //         // FIXME
    //     }
}

// ============================================================================
// ENUMERATOR =================================================================
// ============================================================================

device::list<device::camera> uvc::enumerator::find() noexcept {
    log::trace tr;
    device::list<device::camera> cameras;  // what is returned

    // try to find UVC cameras on the system and return a list of camera instances
    tr << "Searching for UVC cameras in /dev/video* ...\n";

    bool have_uvch = true;  // flag, if we parse for UVC metadata devices
    int lastcam = -1;       // last camera number found, used to find UVC metadata devices

    for (int dev = 0; dev <V_VI 64; ++dev) {
        std::filesystem::path devpath = std::format("/dev/video{}", dev);

        // check if device exists (and get rdev info)
        struct stat st;
        if (stat(devpath.c_str(), &st) < 0)
            continue;

        std::string rdev = std::format("{}:{}", major(st.st_rdev), minor(st.st_rdev));
        tr << std::format("  {} ({})\n", devpath.string(), rdev);

        // get capabilities and check if this is a UVC camera
        storage::file::io fd = storage::file::io(devpath, storage::file::io::readwrite);
        if (!fd) {
            tr << std::format("  !!! failed to open device: {}\n", std::strerror(errno));
            continue;
        }

        v4l2_capability cap;
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
            tr << std::format("  !!! failed to query device capabilities: {}\n", std::strerror(errno));
            continue;
        }

        if (not(cap.device_caps & V4L2_CAP_STREAMING)) {
            tr << "  !!! not a streaming device, ignoring\n";
            continue;
        }

        if (cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) {
            tr << "    -> video capture device\n";
            cameras.emplace_back(std::make_shared<uvc>(device::state::available));
            lastcam = cameras.size() - 1;

            dynamic_cast<uvc *>(cameras[lastcam].get())->dev_stream_ = devpath;
            dynamic_cast<uvc *>(cameras[lastcam].get())->name_ =  //
                std::string((const char *)cap.card)               //
                + " (" + std::string((const char *)cap.bus_info) + ")";
        }
        else if (cap.device_caps & V4L2_CAP_META_CAPTURE) {
            tr << "    -> metadata capture device\n";

            if (lastcam < 0) {
                // meta-capture should come after video capture devices,
                // if we haven't found any video capture device yet,
                // we won't parse for metadata devices any further
                have_uvch = false;
                tr << "    !!! device listed out of order\n";
            }

            if (!have_uvch) {
                tr << "    !!! ignoring metadata device\n";
                continue;
            }

            dynamic_cast<uvc *>(cameras[lastcam].get())->dev_meta_ = devpath;
        }
        else {
            tr << "  !!! not a video capture or metadata device, ignoring.\n";
            continue;
        }

        // get media information if available
        std::filesystem::path sysdir = std::format("/sys/dev/char/{}/device", rdev);
        for (const auto &entry : std::filesystem::directory_iterator(sysdir)) {
            auto filename = entry.path().filename().string();

            if (filename.find("media") == 0) {
                tr << std::format("         {}:\n", filename);

                storage::file::io fd = storage::file::io(std::format("/dev/{}", filename), storage::file::io::readonly);
                if (fd.valid()) {
                    struct media_device_info mdi;
                    if (ioctl(fd, MEDIA_IOC_DEVICE_INFO, &mdi) == 0) {
                        tr << std::format("             driver:   {}\n", (const char *)mdi.driver);
                        tr << std::format("             model:    {}\n", (const char *)mdi.model);
                        tr << std::format("             serial:   {}\n", (const char *)mdi.serial);
                        tr << std::format("             bus_info: {}\n", (const char *)mdi.bus_info);
                        tr << std::format("             version:  {}.{}.{}\n", (mdi.media_version >> 16) & 0xFF,
                                          (mdi.media_version >> 8) & 0xFF, mdi.media_version & 0xFF);

                        // for now, we ignore the media information, but we could use it to filter for specific devices in the
                        // future note also, that the "serial" is not necessarily a unique identifier for the camera, so we ignore
                        // it
                    }
                }

                break;
            }
        }
    }

    // query cameras
    for (auto &camera : cameras) {
        dynamic_cast<uvc *>(camera.get())->query();
    }

    return cameras;
}

// ============================================================================
// UVC ========================================================================
// ============================================================================

settings::json uvc::info() const noexcept {
    return info_;
}

void uvc::setup(settings::json) {
    // FIXME setup camera
}

bool uvc::connect() {
    return false;
}

void uvc::disconnect() {
    // FIXME
}
