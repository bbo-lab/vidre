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

            inline constexpr uint32_t fourcc_code(const char (&code)[5]) noexcept {
                return (static_cast<uint32_t>(code[0]) << 24) | (static_cast<uint32_t>(code[1]) << 16) |
                       (static_cast<uint32_t>(code[2]) << 8) | (static_cast<uint32_t>(code[3]));
            }

            inline constexpr uint32_t fourcc_be_code(const char (&code)[5]) noexcept {
                return fourcc_code(code) | (1u << 31);
            }

            template <typename T>
            inline constexpr bool fourcc_is_be(T code) noexcept {
                return (static_cast<uint32_t>(code) & (1u << 31)) != 0;
            }

            /** Enumeration of pixel formats.
             * @internal
             * This list is was auto-generated with the helper/generate.lua tool. Note also, that
             * name lookup is done in Lua (vidre.video.format.fourcc) and not in C++.
             * @endinternal
             * */
            enum class fourcc : uint32_t {
                invalid = static_cast<uint32_t>(-1),
                /* RGB formats (1 or 2 bytes per pixel) */
                rgb332 = fourcc_code("RGB1"),
                rgb444 = fourcc_code("R444"),
                argb444 = fourcc_code("AR12"),
                xrgb444 = fourcc_code("XR12"),
                rgba444 = fourcc_code("RA12"),
                rgbx444 = fourcc_code("RX12"),
                abgr444 = fourcc_code("AB12"),
                xbgr444 = fourcc_code("XB12"),
                bgra444 = fourcc_code("GA12"),
                bgrx444 = fourcc_code("BX12"),
                rgb555 = fourcc_code("RGBO"),
                argb555 = fourcc_code("AR15"),
                xrgb555 = fourcc_code("XR15"),
                rgba555 = fourcc_code("RA15"),
                rgbx555 = fourcc_code("RX15"),
                abgr555 = fourcc_code("AB15"),
                xbgr555 = fourcc_code("XB15"),
                bgra555 = fourcc_code("BA15"),
                bgrx555 = fourcc_code("BX15"),
                rgb565 = fourcc_code("RGBP"),
                rgb555x = fourcc_code("RGBQ"),
                argb555x = fourcc_be_code("AR15"),
                xrgb555x = fourcc_be_code("XR15"),
                rgb565x = fourcc_code("RGBR"),
                /* RGB formats (3 or 4 bytes per pixel) */
                bgr666 = fourcc_code("BGRH"),
                bgr24 = fourcc_code("BGR3"),
                rgb24 = fourcc_code("RGB3"),
                bgr32 = fourcc_code("BGR4"),
                abgr32 = fourcc_code("AR24"),
                xbgr32 = fourcc_code("XR24"),
                bgra32 = fourcc_code("RA24"),
                bgrx32 = fourcc_code("RX24"),
                rgb32 = fourcc_code("RGB4"),
                rgba32 = fourcc_code("AB24"),
                rgbx32 = fourcc_code("XB24"),
                argb32 = fourcc_code("BA24"),
                xrgb32 = fourcc_code("BX24"),
                rgbx1010102 = fourcc_code("RX30"),
                rgba1010102 = fourcc_code("RA30"),
                argb2101010 = fourcc_code("AR30"),
                /* RGB formats (6 or 8 bytes per pixel) */
                bgr48_12 = fourcc_code("B312"),
                abgr64_12 = fourcc_code("B412"),
                /* Grey formats */
                grey = fourcc_code("GREY"),
                y4 = fourcc_code("Y04 "),
                y6 = fourcc_code("Y06 "),
                y10 = fourcc_code("Y10 "),
                y12 = fourcc_code("Y12 "),
                y012 = fourcc_code("Y012"),
                y14 = fourcc_code("Y14 "),
                y16 = fourcc_code("Y16 "),
                y16_be = fourcc_be_code("Y16 "),
                /* Grey bit-packed formats */
                y10bpack = fourcc_code("Y10B"),
                y10p = fourcc_code("Y10P"),
                ipu3_y10 = fourcc_code("ip3y"),
                /* Palette formats */
                pal8 = fourcc_code("PAL8"),
                /* Chrominance formats */
                uv8 = fourcc_code("UV8 "),
                /* Luminance+Chrominance formats */
                yuyv = fourcc_code("YUYV"),
                yyuv = fourcc_code("YYUV"),
                yvyu = fourcc_code("YVYU"),
                uyvy = fourcc_code("UYVY"),
                vyuy = fourcc_code("VYUY"),
                y41p = fourcc_code("Y41P"),
                yuv444 = fourcc_code("Y444"),
                yuv555 = fourcc_code("YUVO"),
                yuv565 = fourcc_code("YUVP"),
                yuv24 = fourcc_code("YUV3"),
                yuv32 = fourcc_code("YUV4"),
                ayuv32 = fourcc_code("AYUV"),
                xyuv32 = fourcc_code("XYUV"),
                vuya32 = fourcc_code("VUYA"),
                vuyx32 = fourcc_code("VUYX"),
                yuva32 = fourcc_code("YUVA"),
                yuvx32 = fourcc_code("YUVX"),
                m420 = fourcc_code("M420"),
                yuv48_12 = fourcc_code("Y312"),
                /* YCbCr packed format. For each Y2xx format, xx bits of valid data occupy the MSBs of the 16 bit components, and
                 * 16-xx bits of zero padding occupy the LSBs. */
                y210 = fourcc_code("Y210"),
                y212 = fourcc_code("Y212"),
                y216 = fourcc_code("Y216"),
                /* two planes -- one Y, one Cr + Cb interleaved */
                nv12 = fourcc_code("NV12"),
                nv21 = fourcc_code("NV21"),
                nv16 = fourcc_code("NV16"),
                nv61 = fourcc_code("NV61"),
                nv24 = fourcc_code("NV24"),
                nv42 = fourcc_code("NV42"),
                p010 = fourcc_code("P010"),
                p012 = fourcc_code("P012"),
                /* two non contiguous planes - one Y, one Cr + Cb interleaved */
                nv12m = fourcc_code("NM12"),
                nv21m = fourcc_code("NM21"),
                nv16m = fourcc_code("NM16"),
                nv61m = fourcc_code("NM61"),
                p012m = fourcc_code("PM12"),
                /* three planes - Y Cb, Cr */
                yuv410 = fourcc_code("YUV9"),
                yvu410 = fourcc_code("YVU9"),
                yuv411p = fourcc_code("411P"),
                yuv420 = fourcc_code("YU12"),
                yvu420 = fourcc_code("YV12"),
                yuv422p = fourcc_code("422P"),
                /* three non contiguous planes - Y, Cb, Cr */
                yuv420m = fourcc_code("YM12"),
                yvu420m = fourcc_code("YM21"),
                yuv422m = fourcc_code("YM16"),
                yvu422m = fourcc_code("YM61"),
                yuv444m = fourcc_code("YM24"),
                yvu444m = fourcc_code("YM42"),
                /* Tiled YUV formats */
                nv12_4l4 = fourcc_code("VT12"),
                nv12_16l16 = fourcc_code("HM12"),
                nv12_32l32 = fourcc_code("ST12"),
                nv15_4l4 = fourcc_code("VT15"),
                p010_4l4 = fourcc_code("T010"),
                nv12_8l128 = fourcc_code("AT12"),
                nv12_10be_8l128 = fourcc_be_code("AX12"),
                /* Tiled YUV formats, non contiguous planes */
                nv12mt = fourcc_code("TM12"),
                nv12mt_16x16 = fourcc_code("VM12"),
                nv12m_8l128 = fourcc_code("NA12"),
                nv12m_10be_8l128 = fourcc_be_code("NT12"),
                /* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
                sbggr8 = fourcc_code("BA81"),
                sgbrg8 = fourcc_code("GBRG"),
                sgrbg8 = fourcc_code("GRBG"),
                srggb8 = fourcc_code("RGGB"),
                sbggr10 = fourcc_code("BG10"),
                sgbrg10 = fourcc_code("GB10"),
                sgrbg10 = fourcc_code("BA10"),
                srggb10 = fourcc_code("RG10"),
                sbggr10p = fourcc_code("pBAA"),
                sgbrg10p = fourcc_code("pGAA"),
                sgrbg10p = fourcc_code("pgAA"),
                srggb10p = fourcc_code("pRAA"),
                sbggr10alaw8 = fourcc_code("aBA8"),
                sgbrg10alaw8 = fourcc_code("aGA8"),
                sgrbg10alaw8 = fourcc_code("agA8"),
                srggb10alaw8 = fourcc_code("aRA8"),
                sbggr10dpcm8 = fourcc_code("bBA8"),
                sgbrg10dpcm8 = fourcc_code("bGA8"),
                sgrbg10dpcm8 = fourcc_code("BD10"),
                srggb10dpcm8 = fourcc_code("bRA8"),
                sbggr12 = fourcc_code("BG12"),
                sgbrg12 = fourcc_code("GB12"),
                sgrbg12 = fourcc_code("BA12"),
                srggb12 = fourcc_code("RG12"),
                sbggr12p = fourcc_code("pBCC"),
                sgbrg12p = fourcc_code("pGCC"),
                sgrbg12p = fourcc_code("pgCC"),
                srggb12p = fourcc_code("pRCC"),
                sbggr14 = fourcc_code("BG14"),
                sgbrg14 = fourcc_code("GB14"),
                sgrbg14 = fourcc_code("GR14"),
                srggb14 = fourcc_code("RG14"),
                sbggr14p = fourcc_code("pBEE"),
                sgbrg14p = fourcc_code("pGEE"),
                sgrbg14p = fourcc_code("pgEE"),
                srggb14p = fourcc_code("pREE"),
                sbggr16 = fourcc_code("BYR2"),
                sgbrg16 = fourcc_code("GB16"),
                sgrbg16 = fourcc_code("GR16"),
                srggb16 = fourcc_code("RG16"),
                /* HSV formats */
                hsv24 = fourcc_code("HSV3"),
                hsv32 = fourcc_code("HSV4"),
                /* compressed formats */
                mjpeg = fourcc_code("MJPG"),
                jpeg = fourcc_code("JPEG"),
                dv = fourcc_code("dvsd"),
                mpeg = fourcc_code("MPEG"),
                h264 = fourcc_code("H264"),
                h264_no_sc = fourcc_code("AVC1"),
                h264_mvc = fourcc_code("M264"),
                h263 = fourcc_code("H263"),
                mpeg1 = fourcc_code("MPG1"),
                mpeg2 = fourcc_code("MPG2"),
                mpeg2_slice = fourcc_code("MG2S"),
                mpeg4 = fourcc_code("MPG4"),
                xvid = fourcc_code("XVID"),
                vc1_annex_g = fourcc_code("VC1G"),
                vc1_annex_l = fourcc_code("VC1L"),
                vp8 = fourcc_code("VP80"),
                vp8_frame = fourcc_code("VP8F"),
                vp9 = fourcc_code("VP90"),
                vp9_frame = fourcc_code("VP9F"),
                hevc = fourcc_code("HEVC"),
                fwht = fourcc_code("FWHT"),
                fwht_stateless = fourcc_code("SFWH"),
                h264_slice = fourcc_code("S264"),
                hevc_slice = fourcc_code("S265"),
                av1_frame = fourcc_code("AV1F"),
                spk = fourcc_code("SPK0"),
                rv30 = fourcc_code("RV30"),
                rv40 = fourcc_code("RV40"),
                /* Vendor-specific formats */
                cpia1 = fourcc_code("CPIA"),
                wnva = fourcc_code("WNVA"),
                sn9c10x = fourcc_code("S910"),
                sn9c20x_i420 = fourcc_code("S920"),
                pwc1 = fourcc_code("PWC1"),
                pwc2 = fourcc_code("PWC2"),
                et61x251 = fourcc_code("E625"),
                spca501 = fourcc_code("S501"),
                spca505 = fourcc_code("S505"),
                spca508 = fourcc_code("S508"),
                spca561 = fourcc_code("S561"),
                pac207 = fourcc_code("P207"),
                mr97310a = fourcc_code("M310"),
                jl2005bcd = fourcc_code("JL20"),
                sn9c2028 = fourcc_code("SONX"),
                sq905c = fourcc_code("905C"),
                pjpg = fourcc_code("PJPG"),
                ov511 = fourcc_code("O511"),
                ov518 = fourcc_code("O518"),
                stv0680 = fourcc_code("S680"),
                tm6000 = fourcc_code("TM60"),
                cit_yyvyuy = fourcc_code("CITV"),
                konica420 = fourcc_code("KONI"),
                jpgl = fourcc_code("JPGL"),
                se401 = fourcc_code("S401"),
                s5c_uyvy_jpg = fourcc_code("S5CI"),
                y8i = fourcc_code("Y8I "),
                y12i = fourcc_code("Y12I"),
                z16 = fourcc_code("Z16 "),
                mt21c = fourcc_code("MT21"),
                mm21 = fourcc_code("MM21"),
                mt2110t = fourcc_code("MT2T"),
                mt2110r = fourcc_code("MT2R"),
                inzi = fourcc_code("INZI"),
                cnf4 = fourcc_code("CNF4"),
                hi240 = fourcc_code("HI24"),
                qc08c = fourcc_code("Q08C"),
                qc10c = fourcc_code("Q10C"),
                ajpg = fourcc_code("AJPG"),
                hextile = fourcc_code("HXTL"),
                /* 10bit raw packed, 32 bytes for every 25 pixels, last LSB 6 bits unused */
                ipu3_sbggr10 = fourcc_code("ip3b"),
                ipu3_sgbrg10 = fourcc_code("ip3g"),
                ipu3_sgrbg10 = fourcc_code("ip3G"),
                ipu3_srggb10 = fourcc_code("ip3r"),
                /* SDR formats - used only for Software Defined Radio devices */
                cu8 = fourcc_code("CU08"),
                cu16le = fourcc_code("CU16"),
                cs8 = fourcc_code("CS08"),
                cs14le = fourcc_code("CS14"),
                ru12le = fourcc_code("RU12"),
                pcu16be = fourcc_code("PC16"),
                pcu18be = fourcc_code("PC18"),
                pcu20be = fourcc_code("PC20"),
                /* Touch formats - used for Touch devices */
                delta_td16 = fourcc_code("TD16"),
                delta_td08 = fourcc_code("TD08"),
                tu16 = fourcc_code("TU16"),
                tu08 = fourcc_code("TU08"),
                /* Meta-data formats */
                vsp1_hgo = fourcc_code("VSPH"),
                vsp1_hgt = fourcc_code("VSPT"),
                uvc = fourcc_code("UVCH"),
                d4xx = fourcc_code("D4XX"),
                vivid = fourcc_code("VIVD"),
                /* Vendor specific - used for RK_ISP1 camera sub-system */
                rk_isp1_params = fourcc_code("RK1P"),
                rk_isp1_stat_3a = fourcc_code("RK1S"),
            };

            /** Pixel formats.
             *
             * There are several different types of pixel formats, and the list
             * is quite long. While VidRe internally uses the FourCC convention
             * (see e.g. [fourcc.org](https://fourcc.org/) to identify pixel formats,
             * the interface provides a mapping between several standards.
             * 
             * @note Even though most pixel-formats may be recognized, that does
             *       not necessarily mean that they can be decoded by VidRe!
             *
             * @see FFmpeg's [pixfmt.h](https://ffmpeg.org/doxygen/trunk/pixfmt_8h.html)
             * */
            class VIDRE_CORE_VISIBILITY pixel {
            public:
                /** Supported pixel format conventions.
                 * */
                enum class convention {
                    fourcc,      ///< FourCC code (default).
                    usb3vision,  ///< USB3 Vision pixel format code (see [USB3 Vision
                                 ///< standard](FIXME)).
                    gigevision,  ///< GigE Vision pixel format code (see [GigE Vision
                                 ///< standard](FIXME)).
                };

                template<convention Conv>
                pixel from(const std::string &name) noexcept;


                struct preferred {
                    static constexpr fourcc gray8 = fourcc::grey;
                    static constexpr fourcc gray10 = fourcc::y10;
                    static constexpr fourcc gray12 = fourcc::y12;
                    static constexpr fourcc gray14 = fourcc::y14;
                    static constexpr fourcc gray16 = fourcc::y16;

                    static constexpr fourcc rgb24 = fourcc::rgb24;
                    static constexpr fourcc rgb32 = fourcc::rgba32;
                };

            public:
                pixel() noexcept = default;
                explicit pixel(fourcc format) noexcept : format_(format) {}

            protected:
                fourcc format_{};

            public:
                /** Check if valid.
                 * */
                bool valid() const noexcept {
                    return format_ != fourcc::invalid;
                }

                operator fourcc() const noexcept {
                    return format_;
                }

                bool operator==(const pixel &other) const noexcept {
                    return format_ == other.format_;
                }

                std::string name() const noexcept;
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
