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
#ifndef VIDRE_CORE_VIDEO_PIXEL_HH
#define VIDRE_CORE_VIDEO_PIXEL_HH
#pragma once

/**@addtogroup api-core-video-format-pixel
 * @ingroup api-core-video-format
 * @details
 * @fileinfo
 *
 * Encoding and decoding of pixel formats, and conversion between them.
 *
 * @see FFmpeg's [pixfmt.h](https://ffmpeg.org/doxygen/trunk/pixfmt_8h.html)
 * */

#include <vidre/core/config.hh>
#include <vidre/video/format/geometry.hh>
#include <vidre/storage.hh>
#include <vidre/exception.hh>
#include <vidre/util/status.hh>

#include <string>
#include <string_view>
#include <bit>

#include <cstdint>
#include <cstddef>

#include <linux/videodev2.h>

namespace vidre {

    namespace video {

        namespace format {

            /**@addtogroup api-core-video-format-pixel
             * */
            ///@{

            inline constexpr uint32_t fourcc(const char (&code)[5]) noexcept {
                return (static_cast<uint32_t>(code[0]) << 24) | (static_cast<uint32_t>(code[1]) << 16) |
                       (static_cast<uint32_t>(code[2]) << 8) | (static_cast<uint32_t>(code[3]));
            }

            inline constexpr uint32_t fourcc_be(const char (&code)[5]) noexcept {
                return fourcc(code) | (1u << 31);
            }

            /** Enumeration of pixel formats (FourCC).
             * */
            enum class fourcc : uint32_t {
                /* RGB formats (1 or 2 bytes per pixel) */
                rgb332 = fourcc("RGB1"),       ///< RGB-3-3-2
                rgb444 = fourcc("R444"),       ///< xxxxrrrr ggggbbbb
                argb444 = fourcc("AR12"),      ///< aaaarrrr ggggbbbb
                xrgb444 = fourcc("XR12"),      ///< xxxxrrrr ggggbbbb
                rgba444 = fourcc("RA12"),      ///< rrrrgggg bbbbaaaa
                rgbx444 = fourcc("RX12"),      ///< rrrrgggg bbbbxxxx
                abgr444 = fourcc("AB12"),      ///< aaaabbbb ggggrrrr
                xbgr444 = fourcc("XB12"),      ///< xxxxbbbb ggggrrrr
                bgra444 = fourcc("GA12"),      ///< bbbbgggg rrrraaaa
                bgrx444 = fourcc("BX12"),      ///< bbbbgggg rrrrxxxx
                rgb555 = fourcc("RGBO"),       ///< RGB-5-5-5
                argb555 = fourcc("AR15"),      ///< ARGB-1-5-5-5
                xrgb555 = fourcc("XR15"),      ///< XRGB-1-5-5-5
                rgba555 = fourcc("RA15"),      ///< RGBA-5-5-5-1
                rgbx555 = fourcc("RX15"),      ///< RGBX-5-5-5-1
                abgr555 = fourcc("AB15"),      ///< ABGR-1-5-5-5
                xbgr555 = fourcc("XB15"),      ///< XBGR-1-5-5-5
                bgra555 = fourcc("BA15"),      ///< BGRA-5-5-5-1
                bgrx555 = fourcc("BX15"),      ///< BGRX-5-5-5-1
                rgb565 = fourcc("RGBP"),       ///< RGB-5-6-5
                rgb555x = fourcc("RGBQ"),      ///< RGB-5-5-5 BE
                argb555x = fourcc_be("AR15"),  ///< ARGB-5-5-5 BE
                xrgb555x = fourcc_be("XR15"),  ///< XRGB-5-5-5 BE
                rgb565x = fourcc("RGBR"),      ///< RGB-5-6-5 BE
                /* RGB formats (3 or 4 bytes per pixel) */
                bgr666 = fourcc("BGRH"),       ///< BGR-6-6-6
                bgr24 = fourcc("BGR3"),        ///< BGR-8-8-8
                rgb24 = fourcc("RGB3"),        ///< RGB-8-8-8
                bgr32 = fourcc("BGR4"),        ///< BGR-8-8-8-8
                abgr32 = fourcc("AR24"),       ///< BGRA-8-8-8-8
                xbgr32 = fourcc("XR24"),       ///< BGRX-8-8-8-8
                bgra32 = fourcc("RA24"),       ///< ABGR-8-8-8-8
                bgrx32 = fourcc("RX24"),       ///< XBGR-8-8-8-8
                rgb32 = fourcc("RGB4"),        ///< RGB-8-8-8-8
                rgba32 = fourcc("AB24"),       ///< RGBA-8-8-8-8
                rgbx32 = fourcc("XB24"),       ///< RGBX-8-8-8-8
                argb32 = fourcc("BA24"),       ///< ARGB-8-8-8-8
                xrgb32 = fourcc("BX24"),       ///< XRGB-8-8-8-8
                rgbx1010102 = fourcc("RX30"),  ///< RGBX-10-10-10-2
                rgba1010102 = fourcc("RA30"),  ///< RGBA-10-10-10-2
                argb2101010 = fourcc("AR30"),  ///< ARGB-2-10-10-10
                /* RGB formats (6 or 8 bytes per pixel) */
                bgr48_12 = fourcc("B312"),   ///< BGR 12-bit per component
                abgr64_12 = fourcc("B412"),  ///< BGRA 12-bit per component
                /* Grey formats */
                grey = fourcc("GREY"),       ///< 8  Greyscale
                y4 = fourcc("Y04 "),         ///< 4  Greyscale
                y6 = fourcc("Y06 "),         ///< 6  Greyscale
                y10 = fourcc("Y10 "),        ///< Greyscale
                y12 = fourcc("Y12 "),        ///< Greyscale
                y012 = fourcc("Y012"),       ///< Greyscale
                y14 = fourcc("Y14 "),        ///< Greyscale
                y16 = fourcc("Y16 "),        ///< Greyscale
                y16_be = fourcc_be("Y16 "),  ///< Greyscale BE
                /* Grey bit-packed formats */
                y10bpack = fourcc("Y10B"),  ///< Greyscale bit-packed
                y10p = fourcc("Y10P"),      ///< Greyscale, MIPI RAW10 packed
                ipu3_y10 = fourcc("ip3y"),  ///< IPU3 packed 10-bit greyscale
                /* Palette formats */
                pal8 = fourcc("PAL8"),  ///< 8  8-bit palette
                /* Chrominance formats */
                uv8 = fourcc("UV8 "),  ///< 8  UV 4:4
                /* Luminance+Chrominance formats */
                yuyv = fourcc("YUYV"),      ///< YUV 4:2:2
                yyuv = fourcc("YYUV"),      ///< YUV 4:2:2
                yvyu = fourcc("YVYU"),      ///< YVU 4:2:2
                uyvy = fourcc("UYVY"),      ///< YUV 4:2:2
                vyuy = fourcc("VYUY"),      ///< YUV 4:2:2
                y41p = fourcc("Y41P"),      ///< YUV 4:1:1
                yuv444 = fourcc("Y444"),    ///< xxxxyyyy uuuuvvvv
                yuv555 = fourcc("YUVO"),    ///< YUV-5-5-5
                yuv565 = fourcc("YUVP"),    ///< YUV-5-6-5
                yuv24 = fourcc("YUV3"),     ///< YUV-8-8-8
                yuv32 = fourcc("YUV4"),     ///< YUV-8-8-8-8
                ayuv32 = fourcc("AYUV"),    ///< AYUV-8-8-8-8
                xyuv32 = fourcc("XYUV"),    ///< XYUV-8-8-8-8
                vuya32 = fourcc("VUYA"),    ///< VUYA-8-8-8-8
                vuyx32 = fourcc("VUYX"),    ///< VUYX-8-8-8-8
                yuva32 = fourcc("YUVA"),    ///< YUVA-8-8-8-8
                yuvx32 = fourcc("YUVX"),    ///< YUVX-8-8-8-8
                m420 = fourcc("M420"),      ///< YUV 4:2:0 2 lines y, 1 line uv interleaved
                yuv48_12 = fourcc("Y312"),  ///< YUV 4:4:4 12-bit per component
                /* YCbCr packed format. For each Y2xx format, xx bits of valid data occupy the MSBs of the 16 bit components, and
                 * 16-xx bits of zero padding occupy the LSBs. */
                y210 = fourcc("Y210"),  ///< YUYV 4:2:2
                y212 = fourcc("Y212"),  ///< YUYV 4:2:2
                y216 = fourcc("Y216"),  ///< YUYV 4:2:2
                /* two planes -- one Y, one Cr + Cb interleaved */
                nv12 = fourcc("NV12"),  ///< Y/CbCr 4:2:0
                nv21 = fourcc("NV21"),  ///< Y/CrCb 4:2:0
                nv16 = fourcc("NV16"),  ///< Y/CbCr 4:2:2
                nv61 = fourcc("NV61"),  ///< Y/CrCb 4:2:2
                nv24 = fourcc("NV24"),  ///< Y/CbCr 4:4:4
                nv42 = fourcc("NV42"),  ///< Y/CrCb 4:4:4
                p010 = fourcc("P010"),  ///< Y/CbCr 4:2:0 10-bit per component
                p012 = fourcc("P012"),  ///< Y/CbCr 4:2:0 12-bit per component
                /* two non contiguous planes - one Y, one Cr + Cb interleaved */
                nv12m = fourcc("NM12"),  ///< Y/CbCr 4:2:0
                nv21m = fourcc("NM21"),  ///< Y/CrCb 4:2:0
                nv16m = fourcc("NM16"),  ///< Y/CbCr 4:2:2
                nv61m = fourcc("NM61"),  ///< Y/CrCb 4:2:2
                p012m = fourcc("PM12"),  ///< Y/CbCr 4:2:0 12-bit per component
                /* three planes - Y Cb, Cr */
                yuv410 = fourcc("YUV9"),   ///< 9  YUV 4:1:0
                yvu410 = fourcc("YVU9"),   ///< 9  YVU 4:1:0
                yuv411p = fourcc("411P"),  ///< YVU411 planar
                yuv420 = fourcc("YU12"),   ///< YUV 4:2:0
                yvu420 = fourcc("YV12"),   ///< YVU 4:2:0
                yuv422p = fourcc("422P"),  ///< YVU422 planar
                /* three non contiguous planes - Y, Cb, Cr */
                yuv420m = fourcc("YM12"),  ///< YUV420 planar
                yvu420m = fourcc("YM21"),  ///< YVU420 planar
                yuv422m = fourcc("YM16"),  ///< YUV422 planar
                yvu422m = fourcc("YM61"),  ///< YVU422 planar
                yuv444m = fourcc("YM24"),  ///< YUV444 planar
                yvu444m = fourcc("YM42"),  ///< YVU444 planar
                /* Tiled YUV formats */
                nv12_4l4 = fourcc("VT12"),            ///< Y/CbCr 4:2:0  4x4 tiles
                nv12_16l16 = fourcc("HM12"),          ///< Y/CbCr 4:2:0 16x16 tiles
                nv12_32l32 = fourcc("ST12"),          ///< Y/CbCr 4:2:0 32x32 tiles
                nv15_4l4 = fourcc("VT15"),            ///< Y/CbCr 4:2:0 10-bit 4x4 tiles
                p010_4l4 = fourcc("T010"),            ///< Y/CbCr 4:2:0 10-bit 4x4 macroblocks
                nv12_8l128 = fourcc("AT12"),          ///< Y/CbCr 4:2:0 8x128 tiles
                nv12_10be_8l128 = fourcc_be("AX12"),  ///< Y/CbCr 4:2:0 10-bit 8x128 tiles
                /* Tiled YUV formats, non contiguous planes */
                nv12mt = fourcc("TM12"),               ///< Y/CbCr 4:2:0 64x32 tiles
                nv12mt_16x16 = fourcc("VM12"),         ///< Y/CbCr 4:2:0 16x16 tiles
                nv12m_8l128 = fourcc("NA12"),          ///< Y/CbCr 4:2:0 8x128 tiles
                nv12m_10be_8l128 = fourcc_be("NT12"),  ///< Y/CbCr 4:2:0 10-bit 8x128 tiles
                /* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
                sbggr8 = fourcc("BA81"),        ///< 8  BGBG.. GRGR..
                sgbrg8 = fourcc("GBRG"),        ///< 8  GBGB.. RGRG..
                sgrbg8 = fourcc("GRBG"),        ///< 8  GRGR.. BGBG..
                srggb8 = fourcc("RGGB"),        ///< 8  RGRG.. GBGB..
                sbggr10 = fourcc("BG10"),       ///< BGBG.. GRGR..
                sgbrg10 = fourcc("GB10"),       ///< GBGB.. RGRG..
                sgrbg10 = fourcc("BA10"),       ///< GRGR.. BGBG..
                srggb10 = fourcc("RG10"),       ///< RGRG.. GBGB..
                sbggr10p = fourcc("pBAA"),      ///<
                sgbrg10p = fourcc("pGAA"),      ///<
                sgrbg10p = fourcc("pgAA"),      ///<
                srggb10p = fourcc("pRAA"),      ///<
                sbggr10alaw8 = fourcc("aBA8"),  ///<
                sgbrg10alaw8 = fourcc("aGA8"),  ///<
                sgrbg10alaw8 = fourcc("agA8"),  ///<
                srggb10alaw8 = fourcc("aRA8"),  ///<
                sbggr10dpcm8 = fourcc("bBA8"),  ///<
                sgbrg10dpcm8 = fourcc("bGA8"),  ///<
                sgrbg10dpcm8 = fourcc("BD10"),  ///<
                srggb10dpcm8 = fourcc("bRA8"),  ///<
                sbggr12 = fourcc("BG12"),       ///< BGBG.. GRGR..
                sgbrg12 = fourcc("GB12"),       ///< GBGB.. RGRG..
                sgrbg12 = fourcc("BA12"),       ///< GRGR.. BGBG..
                srggb12 = fourcc("RG12"),       ///< RGRG.. GBGB..
                sbggr12p = fourcc("pBCC"),      ///<
                sgbrg12p = fourcc("pGCC"),      ///<
                sgrbg12p = fourcc("pgCC"),      ///<
                srggb12p = fourcc("pRCC"),      ///<
                sbggr14 = fourcc("BG14"),       ///< BGBG.. GRGR..
                sgbrg14 = fourcc("GB14"),       ///< GBGB.. RGRG..
                sgrbg14 = fourcc("GR14"),       ///< GRGR.. BGBG..
                srggb14 = fourcc("RG14"),       ///< RGRG.. GBGB..
                sbggr14p = fourcc("pBEE"),      ///<
                sgbrg14p = fourcc("pGEE"),      ///<
                sgrbg14p = fourcc("pgEE"),      ///<
                srggb14p = fourcc("pREE"),      ///<
                sbggr16 = fourcc("BYR2"),       ///< BGBG.. GRGR..
                sgbrg16 = fourcc("GB16"),       ///< GBGB.. RGRG..
                sgrbg16 = fourcc("GR16"),       ///< GRGR.. BGBG..
                srggb16 = fourcc("RG16"),       ///< RGRG.. GBGB..
                /* HSV formats */
                hsv24 = fourcc("HSV3"),  ///<
                hsv32 = fourcc("HSV4"),  ///<
                /* compressed formats */
                mjpeg = fourcc("MJPG"),           ///< Motion-JPEG
                jpeg = fourcc("JPEG"),            ///< JFIF JPEG
                dv = fourcc("dvsd"),              ///< 1394
                mpeg = fourcc("MPEG"),            ///< MPEG-1/2/4 Multiplexed
                h264 = fourcc("H264"),            ///< H264 with start codes
                h264_no_sc = fourcc("AVC1"),      ///< H264 without start codes
                h264_mvc = fourcc("M264"),        ///< H264 MVC
                h263 = fourcc("H263"),            ///< H263
                mpeg1 = fourcc("MPG1"),           ///< MPEG-1 ES
                mpeg2 = fourcc("MPG2"),           ///< MPEG-2 ES
                mpeg2_slice = fourcc("MG2S"),     ///< MPEG-2 parsed slice data
                mpeg4 = fourcc("MPG4"),           ///< MPEG-4 part 2 ES
                xvid = fourcc("XVID"),            ///< Xvid
                vc1_annex_g = fourcc("VC1G"),     ///< SMPTE 421M Annex G compliant stream
                vc1_annex_l = fourcc("VC1L"),     ///< SMPTE 421M Annex L compliant stream
                vp8 = fourcc("VP80"),             ///< VP8
                vp8_frame = fourcc("VP8F"),       ///< VP8 parsed frame
                vp9 = fourcc("VP90"),             ///< VP9
                vp9_frame = fourcc("VP9F"),       ///< VP9 parsed frame
                hevc = fourcc("HEVC"),            ///< HEVC aka H.265
                fwht = fourcc("FWHT"),            ///< Fast Walsh Hadamard Transform (vicodec)
                fwht_stateless = fourcc("SFWH"),  ///< Stateless FWHT (vicodec)
                h264_slice = fourcc("S264"),      ///< H264 parsed slices
                hevc_slice = fourcc("S265"),      ///< HEVC parsed slices
                av1_frame = fourcc("AV1F"),       ///< AV1 parsed frame
                spk = fourcc("SPK0"),             ///< Sorenson Spark
                rv30 = fourcc("RV30"),            ///< RealVideo 8
                rv40 = fourcc("RV40"),            ///< RealVideo 9 & 10
                /* Vendor-specific formats */
                cpia1 = fourcc("CPIA"),         ///< cpia1 YUV
                wnva = fourcc("WNVA"),          ///< Winnov hw compress
                sn9c10x = fourcc("S910"),       ///< SN9C10x compression
                sn9c20x_i420 = fourcc("S920"),  ///< SN9C20x YUV 4:2:0
                pwc1 = fourcc("PWC1"),          ///< pwc older webcam
                pwc2 = fourcc("PWC2"),          ///< pwc newer webcam
                et61x251 = fourcc("E625"),      ///< ET61X251 compression
                spca501 = fourcc("S501"),       ///< YUYV per line
                spca505 = fourcc("S505"),       ///< YYUV per line
                spca508 = fourcc("S508"),       ///< YUVY per line
                spca561 = fourcc("S561"),       ///< compressed GBRG bayer
                pac207 = fourcc("P207"),        ///< compressed BGGR bayer
                mr97310a = fourcc("M310"),      ///< compressed BGGR bayer
                jl2005bcd = fourcc("JL20"),     ///< compressed RGGB bayer
                sn9c2028 = fourcc("SONX"),      ///< compressed GBRG bayer
                sq905c = fourcc("905C"),        ///< compressed RGGB bayer
                pjpg = fourcc("PJPG"),          ///< Pixart 73xx JPEG
                ov511 = fourcc("O511"),         ///< ov511 JPEG
                ov518 = fourcc("O518"),         ///< ov518 JPEG
                stv0680 = fourcc("S680"),       ///< stv0680 bayer
                tm6000 = fourcc("TM60"),        ///< tm5600/tm60x0
                cit_yyvyuy = fourcc("CITV"),    ///< one line of Y then 1 line of VYUY
                konica420 = fourcc("KONI"),     ///< YUV420 planar in blocks of 256 pixels
                jpgl = fourcc("JPGL"),          ///< JPEG-Lite
                se401 = fourcc("S401"),         ///< se401 janggu compressed rgb
                s5c_uyvy_jpg = fourcc("S5CI"),  ///< S5C73M3 interleaved UYVY/JPEG
                y8i = fourcc("Y8I "),           ///< Greyscale 8-bit L/R interleaved
                y12i = fourcc("Y12I"),          ///< Greyscale 12-bit L/R interleaved
                z16 = fourcc("Z16 "),           ///< Depth data 16-bit
                mt21c = fourcc("MT21"),         ///< Mediatek compressed block mode
                mm21 = fourcc("MM21"),          ///< Mediatek 8-bit block mode, two non-contiguous planes
                mt2110t = fourcc("MT2T"),       ///< Mediatek 10-bit block tile mode
                mt2110r = fourcc("MT2R"),       ///< Mediatek 10-bit block raster mode
                inzi = fourcc("INZI"),          ///< Intel Planar Greyscale 10-bit and Depth 16-bit
                cnf4 = fourcc("CNF4"),          ///< Intel 4-bit packed depth confidence information
                hi240 = fourcc("HI24"),         ///< BTTV 8-bit dithered RGB
                qc08c = fourcc("Q08C"),         ///< Qualcomm 8-bit compressed
                qc10c = fourcc("Q10C"),         ///< Qualcomm 10-bit compressed
                ajpg = fourcc("AJPG"),          ///< Aspeed JPEG
                hextile = fourcc("HXTL"),       ///< Hextile compressed
                /* 10bit raw packed, 32 bytes for every 25 pixels, last LSB 6 bits unused */
                ipu3_sbggr10 = fourcc("ip3b"),  ///< IPU3 packed 10-bit BGGR bayer
                ipu3_sgbrg10 = fourcc("ip3g"),  ///< IPU3 packed 10-bit GBRG bayer
                ipu3_sgrbg10 = fourcc("ip3G"),  ///< IPU3 packed 10-bit GRBG bayer
                ipu3_srggb10 = fourcc("ip3r"),  ///< IPU3 packed 10-bit RGGB bayer
                /* SDR formats - used only for Software Defined Radio devices */
                cu8 = fourcc("CU08"),      ///< IQ u8
                cu16le = fourcc("CU16"),   ///< IQ u16le
                cs8 = fourcc("CS08"),      ///< complex s8
                cs14le = fourcc("CS14"),   ///< complex s14le
                ru12le = fourcc("RU12"),   ///< real u12le
                pcu16be = fourcc("PC16"),  ///< planar complex u16be
                pcu18be = fourcc("PC18"),  ///< planar complex u18be
                pcu20be = fourcc("PC20"),  ///< planar complex u20be
                /* Touch formats - used for Touch devices */
                delta_td16 = fourcc("TD16"),  ///< 16-bit signed deltas
                delta_td08 = fourcc("TD08"),  ///< 8-bit signed deltas
                tu16 = fourcc("TU16"),        ///< 16-bit unsigned touch data
                tu08 = fourcc("TU08"),        ///< 8-bit unsigned touch data
                /* Meta-data formats */
                vsp1_hgo = fourcc("VSPH"),  ///< R-Car VSP1 1-D Histogram
                vsp1_hgt = fourcc("VSPT"),  ///< R-Car VSP1 2-D Histogram
                uvc = fourcc("UVCH"),       ///< UVC Payload Header metadata
                d4xx = fourcc("D4XX"),      ///< D4XX Payload Header metadata
                vivid = fourcc("VIVD"),     ///< Vivid Metadata
                /* Vendor specific - used for RK_ISP1 camera sub-system */
                rk_isp1_params = fourcc("RK1P"),   ///< Rockchip ISP1 3A Parameters
                rk_isp1_stat_3a = fourcc("RK1S"),  ///< Rockchip ISP1 3A Statistics
            };

            /** FIXME pixel-format converter
             * */
            template <auto Input, auto Output>
            struct convert {
                storage::size_type size(format::size) const noexcept;
                status operator()(format::size, const uint8_t *, uint8_t *) noexcept;
            };

            ///@}

        }  // namespace format

    }  // namespace video

}  // namespace vidre

#endif  // VIDRE_CORE_VIDEO_PIXEL_HH
