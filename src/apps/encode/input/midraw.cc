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
#include "midraw.hh"

#include <iostream>
#include <format>

using namespace vidre;

// void show(uint8_t *ptr, unsigned from, unsigned to) {
//     for (unsigned i = from; i <= to; ++i) {
//         unsigned byte = i / 8;
//         unsigned bit = i - 8 * byte;

//         if (ptr[byte] & (1 << bit))
//             std::cout << '1';
//         else
//             std::cout << '0';
//     }
//     std::cout << ' ';
// }

// void show(uint16_t v, unsigned from, unsigned to) {
//     for (unsigned i = from; i <= to; ++i) {
//         if (v & (1 << i))
//             std::cout << '1';
//         else
//             std::cout << '0';
//     }
//     std::cout << ' ';
// }

bool input::midraw::check(bool full) {
    size_t rowsize;
    switch (bpp_) {
        case 8:
            rowsize = width_;
            break;
        case 10:
            rowsize = width_ / 4 * 5;  // 10-bit packed
            break;
        default:
            qCritical("Invalid bits-per-pixel value (only 8 or 10 allowed).");
            return false;
    }

    framesize_ = rowsize * height_;
    size_t count = size() / framesize_;

    if (size() != count * framesize_) {
        qCritical("Expected packed format.");
        return false;
    }

#if 0
    open();
    if (full and (bpp_ == 10)) {
        // check how many bits are used
        uint16_t bitfield = 0;

        uint8_t buf[5];
        uint16_t data[4];

        while (not file_.atEnd()) {
            file_.read((char *)buf, 5);

            //      Pixel Packing (4 pixels per 5 bytes):
            //             bit|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
            //        Byte 0  | ----------------Pixel 0[7:0]----------------- |
            //        Byte 1  | ----------Pixel 1[5:0]----------- |Pix 0[9:8] |
            //        Byte 2  | ----Pixel 2[3:0]----- | -----Pixel 1[9:6]---- |
            //        Byte 3  |Pix 3[1:0] | --------Pixel 2[9:4]------------- |
            //        Byte 4  | ----------------Pixel 3[9:2]----------------- |

            data[0] = uint16_t(buf[0]) | uint16_t(buf[1] & 0x3) << 8;        // B0[0:7] B1[0:1]
            data[1] = uint16_t(buf[1]) >> 2 | uint16_t(buf[2] & 0xF) << 6;   // B1[2:7] B2[0:3]
            data[2] = uint16_t(buf[2]) >> 4 | uint16_t(buf[3] & 0x3F) << 4;  // B2[4:7] B3[0:5]
            data[3] = uint16_t(buf[3]) >> 6 | uint16_t(buf[4]) << 2;         // B3[6:7] B4[0:7]

            // show(buf, 0, 9);
            // show(buf, 10, 19);
            // show(buf, 20, 29);
            // show(buf, 30, 39);
            // std::cout << file_.pos();
            // std::cout << std::endl;

            // show(data[0], 0, 9);
            // show(data[1], 0, 9);
            // show(data[2], 0, 9);
            // show(data[3], 0, 9);
            // std::cout << std::endl;
            // std::cout << std::endl;

            for (int i = 0; i < 4; ++i) {
                bitfield |= data[i];
            }
        }

        std::cout << bitfield << std::endl;
    }

    file_.seek(0);
#endif
    return true;
}