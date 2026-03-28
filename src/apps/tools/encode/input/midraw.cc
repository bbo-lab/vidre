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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "midraw.hh"

#include <QRegularExpression>

#include <iostream>
#include <format>

using namespace vidre;

void decode_16bit(QFile &in, AVFrame *dst, unsigned size) {
    in.read((char *)dst->data[0], size);
}

void decode_10bit_packed(QFile &in, AVFrame *dst, unsigned size) {
    //      Pixel Packing (4 pixels per 5 bytes):
    //             bit|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
    //        Byte 0  | ----------------Pixel 0[7:0]----------------- |
    //        Byte 1  | ----------Pixel 1[5:0]----------- |Pix 0[9:8] |
    //        Byte 2  | ----Pixel 2[3:0]----- | -----Pixel 1[9:6]---- |
    //        Byte 3  |Pix 3[1:0] | --------Pixel 2[9:4]------------- |
    //        Byte 4  | ----------------Pixel 3[9:2]----------------- |

    unsigned i = 0;
    unsigned o = 0;

    uint8_t buf[5];
    while (i < size) {
        in.read((char *)buf, 5);

        uint16_t *data = reinterpret_cast<uint16_t *>(dst->data[0]) + o;

        data[0] = uint16_t(buf[0]) | uint16_t(buf[1] & 0x3) << 8;        // B0[0:7] B1[0:1]
        data[1] = uint16_t(buf[1]) >> 2 | uint16_t(buf[2] & 0xF) << 6;   // B1[2:7] B2[0:3]
        data[2] = uint16_t(buf[2]) >> 4 | uint16_t(buf[3] & 0x3F) << 4;  // B2[4:7] B3[0:5]
        data[3] = uint16_t(buf[3]) >> 6 | uint16_t(buf[4]) << 2;         // B3[6:7] B4[0:7]

        i += 5;
        o += 4;
    }
}

bool input::midraw::open() {
    if (not file_.open(QIODevice::ReadOnly))
        return false;

    switch (bpp_) {
        case 10:
            frame_.allocate(ffmpeg::pixel_format::gray10le, width_, height_);
            if (packed_)
                decode_ = decode_10bit_packed;
            else
                decode_ = decode_16bit;
            break;

        default:
            throw vidre::exception() << vidre::throw_message("internal error");
    }

    return true;
}

bool input::midraw::check(bool full) {
    size_t rowsize = 0;
    unsigned frames = 0;

    // check *_info.txt file
    auto filename = file_.filesystemFileName();
    filename.replace_extension("");
    filename.replace_filename(filename.filename().string() + "_info.txt");

    QFile info_txt{filename};
    if (info_txt.exists()) {
        qInfo("Found %s...", filename.filename().c_str());

        if (not info_txt.open(QIODevice::ReadOnly)) {
            qCritical("Failed to open. Ignoring...");
        }
        else {
            QRegularExpression cassign("^//[ \t]+((\\w+[ ])*\\w+)[ \t]+=[ \t]+([^\n\r]+)[\n\r]*$");  // comment assignment
            QRegularExpression number("([0-9]+).*");

            while (not info_txt.atEnd()) {
                auto line = info_txt.readLine();

                auto match = cassign.match(line);
                if (match.hasMatch()) {
                    auto key = match.captured(1).toStdString();
                    auto value = match.captured(3);
                    // qDebug("%s = %s", key.c_str(), value.toStdString().c_str());
                    if (key == "Width") {
                        width_ = value.toUInt();
                        rowsize = width_;
                        qDebug("    width = %u", width_);
                    }
                    else if (key == "Height") {
                        height_ = value.toUInt();
                        qDebug("    height = %u", height_);
                    }
                    else if (key == "Image Format") {
                        if (value == "Bayer 10") {
                            qDebug("    sensor = %s", value.toStdString().c_str());
                        }
                        else {
                            qWarning("Image format not recognized: %s", value.toStdString().c_str());
                        }
                    }
                    else if (key == "Sensor output") {
                        match = number.match(value);
                        if (match.hasMatch()) {
                            bpp_ = match.captured(1).toUInt();
                            qDebug("    bpp = %u", bpp_);
                        }
                        else
                            qDebug("'Sensor output = %s' not recognized", value.toStdString().c_str());
                    }
                    else if (key == "Stored as") {
                        if (value == "packed 10 bits/pixel") {
                            qDebug("    format = %s", value.toStdString().c_str());
                            packed_ = true;
                            rowsize = rowsize * 5 / 4;
                        }
                        else if (value == "16 bits/pixel") {
                            qDebug("    format = %s", value.toStdString().c_str());
                            packed_ = false;
                            rowsize = rowsize * 2;
                        }
                        else
                            qDebug("'Stored as = %s' not recognized", value.toStdString().c_str());
                    }
                    else if (key == "Frames") {
                        frames = value.toUInt();
                        qDebug("    frames = %u", frames);
                    }
                }
            }
        }
    }

    if (mono_ != true)
        return false;

    // auto-detect rowsize (and packed)
    if (not rowsize) {
        switch (bpp_) {
            case 8:
                rowsize = width_;
                break;
            case 10:
                rowsize = width_ / 4 * 5;  // 10-bit packed
                packed_ = true;
                break;
            default:
                qCritical("Invalid bits-per-pixel value (only 8 or 10 allowed).");
                return false;
        }
    }

    framesize_ = rowsize * height_;
    size_t count = size() / framesize_;

    if (size() != count * framesize_) {
        if (bpp_ == 10)
            rowsize = 2 * width_;  // 16-bit
        framesize_ = rowsize * height_;
        count = size() / framesize_;
        if (size() != count * framesize_) 
            throw vidre::exception() << throw_message("internal error");
        packed_ = false;
    }

    if (full) {
        // FIXME
    }

    return true;
}