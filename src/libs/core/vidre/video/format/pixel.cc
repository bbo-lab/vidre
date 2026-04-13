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
#include "pixel.hh"

#include <algorithm>
#include <cctype>

using namespace vidre;

// // mono8       = VIDRE_PIXEL_FORMAT(0x00, 0,  8),
// // mono10      = VIDRE_PIXEL_FORMAT(0x01, 0, 10),
// // mono12      = VIDRE_PIXEL_FORMAT(0x02, 0, 12),
// // mono16      = VIDRE_PIXEL_FORMAT(0x03, 0, 16),
// // rgb24       = VIDRE_PIXEL_FORMAT(0x04, 1, 24),
// // rgba32      = VIDRE_PIXEL_FORMAT(0x05, 1, 32),

// // // Specific video formats:
// // bgr24       = VIDRE_PIXEL_FORMAT(0x06, 1, 24),
// // bgra32      = VIDRE_PIXEL_FORMAT(0x08, 1, 32),
// // yuv420p     = VIDRE_PIXEL_FORMAT(0x09, 1, 12),
// // yuv422p     = VIDRE_PIXEL_FORMAT(0x0A, 1, 16),
// // yuv444p     = VIDRE_PIXEL_FORMAT(0x0B, 1, 24),

// static struct {
//     const char *name;
//     video::pixel_format::type fmt;
// } known_formats[] = {
//     // clang-format off
//     {"mono8", video::pixel_format::mono8},
//     {"mono10", video::pixel_format::mono10},
//     {"mono12", video::pixel_format::mono12},
//     {"mono16", video::pixel_format::mono16},
//     {"rgb24", video::pixel_format::rgb24},
//     {"rgba32", video::pixel_format::rgba32},
//     {"bgr24", video::pixel_format::bgr24},
//     {"bgra32", video::pixel_format::bgra32},
//     {"yuv420p", video::pixel_format::yuv420p},
//     {"yuv422p", video::pixel_format::yuv422p},
//     {"yuv444p", video::pixel_format::yuv444p},
//     // clang-format on
// };

// video::pixel_format::type video::pixel_format::parse(std::string_view name) noexcept {
//     std::string cmp = std::string(name);
//     std::transform(cmp.begin(), cmp.end(), cmp.begin(), [](unsigned char c) {
//         return std::tolower(c);
//     });

//     for (const auto &kf : known_formats) {
//         if (cmp == kf.name) {
//             return kf.fmt;
//         }
//     }

//     return type::undefined;
// }

// struct conversion {
//     video::pixel_format::type source;
//     video::pixel_format::type target;
//     video::pixel_format::transform_function func;
// } known_conversions[] = {
//     // FIXME add actual conversions
// };

// video::pixel_format::transform_function video::pixel_format::convert(const pixel_format &target) const {
//     for(const auto &conv : known_conversions) {
//         if (conv.source == pix_fmt_ && conv.target == target.pix_fmt_) {
//             return conv.func;
//         }
//     }
//     return nullptr;
// }