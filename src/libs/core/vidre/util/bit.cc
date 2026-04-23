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
#include "bit.hh"
#include "../exception.hh"

template <>
vidre::memory::size_type vidre::bit::pack<16, 10, false>(const  memory::view &input, memory::area output) {
    using size_type = memory::size_type;

    auto data_in = input.data();
    size_type bytes_in = input.size();

    uint8_t *data_out = output;
    size_type bytes_out = pack_size<16, 10>(bytes_in);

    if (bytes_out > output.size())
        throw exception::runtime() << error::message("output buffer is too small");

    size_type nblocks = bytes_in / 8;
    size_type offset_in = 0;
    size_type offset_out = 0;
    for (size_type b = 0; b < nblocks; ++b) {
        /*
         0        1        2        3        4        5        6        7
        |01234567'89ABCDEF|01234567'89ABCDEF|01234567'89ABCDEF|01234567'89ABCDEF|
        |01234567 01       23456701 23       45670123 45       67012345 67|
         0        1              2               3               4
        */

        // clang-format off
        data_out[offset_out    ] = data_in[offset_in];
        data_out[offset_out + 1] = data_in[offset_in + 1] | (data_in[offset_in + 2] << 2);
        data_out[offset_out + 2] = (data_in[offset_in + 2] >> 6) | (data_in[offset_in + 3] << 2) | (data_in[offset_in + 4] << 4);
        data_out[offset_out + 3] = (data_in[offset_in + 4] >> 4) | (data_in[offset_in + 5] << 4) | (data_in[offset_in + 6] << 6);
        data_out[offset_out + 4] = (data_in[offset_in + 6] >> 2) | (data_in[offset_in + 7] << 6);
        // clang-format on

        offset_out += 5;
        offset_in += 8;
    }

    // rest
    switch (bytes_out - offset_out) {
        case 4:
            data_out[offset_out + 3] =
                (data_in[offset_in + 4] >> 4) | (data_in[offset_in + 5] << 4) | (data_in[offset_in + 6] << 6);
            [[fallthrough]];
        case 3:
            data_out[offset_out + 2] =
                (data_in[offset_in + 2] >> 6) | (data_in[offset_in + 3] << 2) | (data_in[offset_in + 4] << 4);
            [[fallthrough]];
        case 2:
            data_out[offset_out + 1] = data_in[offset_in + 1] | (data_in[offset_in + 2] << 2);
            [[fallthrough]];
        case 1:
            data_out[offset_out] = data_in[offset_in];
            [[fallthrough]];
        case 0:
            break;
        default:
            throw "implementation error";
            break;
    }

    return bytes_out;
}

template <>
vidre::memory::size_type vidre::bit::pack<16, 12, false>(const memory::view &input, memory::area output) {
    using size_type = memory::size_type;

    auto data_in = input.data();
    size_type bytes_in = input.size();

    uint8_t *data_out = output;
    size_type bytes_out = pack_size<16, 12>(bytes_in);

    if (bytes_out > output.size())
        throw exception::runtime() << error::message("output buffer is too small");

    size_type nblocks = bytes_in / 4;
    size_type offset_in = 0;
    size_type offset_out = 0;
    for (size_type b = 0; b < nblocks; ++b) {
        /*
         0        1        2        3
        |01234567'89ABCDEF|01234567'89ABCDEF|
        |01234567 0123     45670123 4567|
         0        1            2
        */

        // clang-format off
        data_out[offset_out    ] = data_in[offset_in];
        data_out[offset_out + 1] = data_in[offset_in + 1] | (data_in[offset_in + 2] << 4);
        data_out[offset_out + 2] = (data_in[offset_in + 2] >> 4) | (data_in[offset_in + 3] << 4);
        // clang-format on

        offset_out += 3;
        offset_in += 4;
    }

    // rest
    switch (bytes_out - offset_out) {
        case 2:
            data_out[offset_out + 1] = data_in[offset_in + 1] | (data_in[offset_in + 2] << 4);
            [[fallthrough]];
        case 1:
            data_out[offset_out] = data_in[offset_in];
            [[fallthrough]];
        case 0:
            break;
        default:
            throw "implementation error";
            break;
    }

    return bytes_out;
}

template <>
vidre::memory::size_type vidre::bit::pack<10, 16, false>(const memory::view &input, memory::area output) {
    using size_type = memory::size_type;

    auto data_in = input.data();
    size_type bytes_in = input.size();

    uint8_t *data_out = output;
    size_type bytes_out = pack_size<10, 16>(bytes_in);

    if (bytes_out > output.size())
        throw exception::runtime() << error::message("output buffer is too small");

    size_type nblocks = bytes_in / 5;
    size_type offset_in = 0;
    size_type offset_out = 0;
    for (size_type b = 0; b < nblocks; ++b) {
        /*
         0        1              2               3               4
        |01234567 01       23456701 23       45670123 45       67012345 67|
        |01234567'89ABCDEF|01234567'89ABCDEF|01234567'89ABCDEF|01234567'89ABCDEF|
         0        1        2        3        4        5        6        7
        */

        // clang-format off
        data_out[offset_out] = (data_in[offset_in]);
        data_out[offset_out+1] = (data_in[offset_in+1] & 0b00000011);
        data_out[offset_out+2] = (data_in[offset_in+1] >> 2) | (data_in[offset_in+2] << 6);
        data_out[offset_out+3] = (data_in[offset_in+2] & 0b00001100) >> 2;
        data_out[offset_out+4] = (data_in[offset_in+2] >> 4) | (data_in[offset_in+3] << 4);
        data_out[offset_out+5] = (data_in[offset_in+3] & 0b00110000) >> 4;
        data_out[offset_out+6] = (data_in[offset_in+3] >> 6) | (data_in[offset_in+4] << 2);
        data_out[offset_out+7] = (data_in[offset_in+4] & 0b11000000) >> 6;
        // clang-format on

        offset_out += 8;
        offset_in += 5;
    }

    // rest
    switch (bytes_out - offset_out) {
        case 7:
            data_out[offset_out + 6] = (data_in[offset_in + 3] >> 6) | (data_in[offset_in + 4] << 2);
            [[fallthrough]];
        case 6:
            data_out[offset_out + 5] = (data_in[offset_in + 3] & 0b00110000) >> 4;
            [[fallthrough]];
        case 5:
            data_out[offset_out + 4] = (data_in[offset_in + 2] >> 4) | (data_in[offset_in + 3] << 4);
            [[fallthrough]];
        case 4:
            data_out[offset_out + 3] = (data_in[offset_in + 2] & 0b00001100) >> 2;
            [[fallthrough]];
        case 3:
            data_out[offset_out + 2] = (data_in[offset_in + 1] >> 2) | (data_in[offset_in + 2] << 6);
            [[fallthrough]];
        case 2:
            data_out[offset_out + 1] = (data_in[offset_in + 1] & 0b00000011);
            [[fallthrough]];
        case 1:
            data_out[offset_out] = (data_in[offset_in]);
            [[fallthrough]];
        case 0:
            break;
        default:
            throw "implementation error";
            break;
    }

    return bytes_out;
}

template <>
vidre::memory::size_type vidre::bit::pack<12, 16, false>(const memory::view &input, memory::area output) {
    using size_type = memory::size_type;

    auto data_in = input.data();
    size_type bytes_in = input.size();

    uint8_t *data_out = output;
    size_type bytes_out = pack_size<12, 16>(bytes_in);

    if (bytes_out > output.size())
        throw exception::runtime() << error::message("output buffer is too small");

    size_type nblocks = bytes_in / 3;
    size_type offset_in = 0;
    size_type offset_out = 0;
    for (size_type b = 0; b < nblocks; ++b) {
        /*
         0        1            2
        |01234567 0123     45670123 4567|
        |01234567'89ABCDEF|01234567'89ABCDEF|
         0        1        2        3
        */

        // clang-format off
        data_out[offset_out] = (data_in[offset_in]);
        data_out[offset_out+1] = (data_in[offset_in+1] & 0b00001111);
        data_out[offset_out+2] = (data_in[offset_in+1] >> 4) | (data_in[offset_in+2] << 4);
        data_out[offset_out+3] = (data_in[offset_in+2] & 0b11110000) >> 4;
        // clang-format on

        offset_out += 4;
        offset_in += 3;
    }

    // rest
    switch (bytes_out - offset_out) {
        case 3:
            data_out[offset_out + 2] = (data_in[offset_in + 1] >> 2) | (data_in[offset_in + 2] << 6);
            [[fallthrough]];
        case 2:
            data_out[offset_out + 1] = (data_in[offset_in + 1] & 0b00001111);
            [[fallthrough]];
        case 1:
            data_out[offset_out] = (data_in[offset_in]);
            [[fallthrough]];
        case 0:
            break;
        default:
            throw "implementation error";
            break;
    }

    return bytes_out;
}
