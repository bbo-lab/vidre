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
/// @file
#ifndef VIDRE_CORE_STORAGE_CCV_HH
#define VIDRE_CORE_STORAGE_CCV_HH
#pragma once

/**@defgroup api-storage-ccv CCV
 * @ingroup api-storage
 * @details
 * @fileinfo
 *
 * The CCV file format was developed by David S. Greenberg¹ during his
 * time at BBO (MPI f. Neurobiol. of Behavior — caesar, Bonn, Germany).
 *
 * **General structure**:
 *
 *      file-header
 *      (
 *          image-data         uint8_t[+]                                   0.13
 *          image-padding      uint8_t[*]                                   0.20
 *          tail-information                                                0.13
 *      )*
 *
 * The <tt>file-header</tt>:
 *
 *      header-size             uint32_t                                    0.12
 *      camera-type             lv-string                                   0.12 (e.g. "Basler\0", or "Aptina\0")
 *                                                                          0.20 (this should be "global" or "rolling")
 *      version                 double                                      0.12
 *      image-type              lv-string                                   0.12 (e.g. "Mono8")
 *      bytes-per-pixel         uint32_t                                    0.12
 *      bits-per-pixel          uint32_t                                    0.12
 *      frame-bytes-on-disk     uint32_t                                    0.12
 *      w,h                     uint32_t[2]                                 0.12
 *      frame-rate              double                                      0.12
 *      bits-are-packed         uint8_t                                     0.13 (boolean)
 *      endianness              uint8_t                                     0.20
 *      reserved                uint8_t[3]                                  0.20
 *      frame-count             uint32_t                                    0.12
 *      offset                  uint32_t[2]                                 0.13
 *      sensor-size             uint32_t[2]                                 0.13
 *      clock-speed             uint64_t                                    0.13
 *      exposure-time-ms        double                                      0.13
 *      gain                    double                                      0.13
 *      system-identification   lv-string                                   0.20
 *
 * with <tt>lv-string</tt>
 *
 *      (
 *          string-length           uint8_t                                 0.13
 *          zero-terminated-string  uint8_t[]                               0.13
 *      )
 *
 * replaced in version 0.20 with:
 *
 *      (
 *          field-length            uint8_t                                 0.20
 *          zero-terminated-string  uint8_t[]                               0.20
 *      )
 *
 * The <tt>tail-information</tt> depends on camera type:
 *
 * @c Aptina:
 *
 *      frame-index             uint32_t                                    0.13
 *      computer-time           double                                      0.13
 *
 * @c Basler:
 *
 *      frame-index             uint32_t                                    0.13
 *      computer-time           double                                      0.13
 *      camera-time             uint64_t                                    0.13
 *      flag-data               uint8_t                                     0.13
 *
 * @c general:
 *
 *      frame-index             uint32_t                                    0.13
 *      computer-time           double                                      0.13
 *      device-timestamp        uint64_t                                    0.20
 *      flag-data               uint32_t                                    0.20
 *
 * The <tt>frame-index</tt> can start at 0 or 1.
 * The <tt>computer-time</tt> in seconds is based on a performance counter (on Windows®:
 * QueryPerformanceCounter()/GetComputerClockRate()). The <tt>camera-time</tt> is a time-stamp, provided by the camera (available
 * for "Basler" camera only, 0 for "Aptina" at the moment of writing). The <tt>flag-data</tt> is a boolean field indicating if the
 * image grab failed.
 *
 * The <tt>image-data</tt> depends on <i>image-type</i> (usually Mono8 or Mono10). When <i>bits-are-packed</i>.
 * Example: Mono10 data may be received from the camera as 16-bit values, for storage this can be packed to 10-bit values.
 *
 * __New in version 0.20__:
 *
 * For the use with memory alignment sensitive architectures, fields are padded to 4 bytes (int32).
 *
 * <tt>field-length </tt>: the field-length must be 4- or 8-byte aligned (see @c vidre::storage::alignment).
 *
 * <tt>endianness</tt>: @c b for "big endian", "l" for little endian.
 *
 * <tt>system-identification</tt>:
 *
 * @TBD{JS suggests, this is a JSON string containing at least}
 *
 *      {
 *          {"plugin" : "full/plugin/name"},
 *          {"camera" :
 *              {
 *                  {"manufacturer", "Manufacturer"},
 *                  {"user-name" : "User-name if applicable"},
 *                  {"serial-number" : "91238740981723"}
 *              }
 *          }
 *      }
 *
 * @TBD{... but may well also contain all settings?}
 *
 * <tt>image-padding</tt>: choosen, so the the tail-information starts at memory alignment boundary.
 * @note Assuming a cropped Mono10 video is recorded with dimension 133×133 pixels. For packing (<i>bits-are-packed</i>),
 *       this means that 28302.4 bytes are used. Therefore, 1.6 bytes must be padded (<i>image-paddeing</i>).
 *
 * <tt>flag-data</tt>:
 *
 * @TBD{to be decided}
 *
 * ¹ Helmholz Centre Hereon, Geesthacht, Germany
 * */

///@cond
#if not defined(VIDRE_CCV_VISIBILITY)
#define VIDRE_CCV_VISIBILITY VIDRE_API_IMPORT
#endif
///@endcond

#include <vidre/storage.hh>

#include <string_view>
#include <string>
#include <array>
#include <bit>
#include <cstring>
#include <cassert>
#include <cstdint>

namespace vidre {

    namespace storage {

        /**@addtogroup api-storage-ccv
         * */
        ///@{

        /** CCV file.
         * */
        class VIDRE_CCV_VISIBILITY ccv {
        public:
            /** Pair of uint32_t (e.g. offset).
             * */
            using pair32 = std::array<uint32_t, 2>;

            static constexpr double version = 0.2;       ///< CCV implementation version.
            static constexpr std::size_t alignment = 4;  ///< Header alignment (for given version).

            /** Helper calculating size required for field padding.
             * */
            static inline size_t pad(size_t n) {
                return (n + alignment - 1) / alignment;
            }

            /** Per-frame tail information.
             * */
            struct __attribute__((packed)) VIDRE_CCV_VISIBILITY tail {
                uint32_t frame_index;
                double computer_time;
                uint64_t camera_timestamp;
                uint32_t flags;
            };

            /** LV-type string.
             * The CCV header encodes a (zero-terminated!) character sequence
             * as
             *
             *      <length>
             *      <data>
             *
             * Since version 0.20, these strings are padded to a 4-byte
             * boundary.
             * */
            struct VIDRE_CCV_VISIBILITY lvstring {
                uint8_t length{};  ///< Field length.
                char data[4]{};    ///< Character data.

                operator std::string_view() const noexcept {
                    // @todo should actually throw an error if zero-termination was not found!
                    return {&data[0], strnlen(&data[0], length)};
                }
            };

            /** Abstraction of the file header.
             *
             * FIXME
             *
             * */
            class VIDRE_CCV_VISIBILITY header {
            public:
                /** Compute required header size.
                 * */
                static size_t size(const std::string_view &camera_type = {}, const std::string_view &image_type = {},
                                   const std::string_view &system_identification = {}) {
                    return sizeof(uint32_t)                         // header-size
                           + pad(camera_type.size() + 2)            // camera-type
                           + sizeof(double)                         // version
                           + pad(camera_type.size() + 2)            // image-type
                           + sizeof(uint32_t)                       // bytes-per-pixel
                           + sizeof(uint32_t)                       // frame-bytes-on-disk
                           + sizeof(uint32_t) * 2                   // w,h
                           + sizeof(double)                         // frame-rate
                           + sizeof(uint8_t)                        // bits-are-packed
                           + sizeof(uint8_t)                        // endianess
                           + sizeof(uint8_t) * 2                    // reserved
                           + sizeof(uint32_t)                       // frame-count
                           + sizeof(uint32_t) * 2                   // offset
                           + sizeof(uint32_t) * 2                   // sensor-size
                           + sizeof(uint64_t)                       // clock-speed
                           + sizeof(double)                         // exposure-time-ms
                           + sizeof(double)                         // gain
                           + pad(system_identification.size() + 2)  // system-identification
                        ;
                }

            public:
                header() = default;

            protected:
                uint32_t *header_size_{};
                lvstring *camera_type_{};
                double *version_{};
                lvstring *image_type_{};
                uint32_t *bytes_per_pixel_{};
                uint32_t *frame_bytes_on_disk_{};
                uint32_t *w_h_{};
                double *framerate_{};
                uint8_t *bits_are_packed_{};
                uint8_t *endianness_{};
                uint8_t *reserved_{};
                uint32_t *frame_count_{};
                uint32_t *offset_{};
                uint32_t *sensor_size_{};
                uint64_t *clock_speed_{};
                double *exposure_time_ms_{};
                double *gain_{};
                lvstring *system_identification_{};

            public:
                // getters
                size_t header_size() {
                    assert(header_size_ != nullptr);
                    return static_cast<size_t>(*header_size_);
                }
                std::string_view camera_type() {
                    assert(camera_type_ != nullptr);
                    return *camera_type_;
                }
                double version() {
                    assert(version_ != nullptr);
                    return *version_;
                }
                std::string_view image_type() {
                    assert(image_type_ != nullptr);
                    return *image_type_;
                }
                uint32_t bytes_per_pixel() {
                    assert(bytes_per_pixel_ != nullptr);
                    return *bytes_per_pixel_;
                }
                uint32_t frame_bytes_on_disk() {
                    assert(frame_bytes_on_disk_ != nullptr);
                    return *frame_bytes_on_disk_;
                }
                pair32 w_h() {
                    assert(w_h_ != nullptr);
                    return {w_h_[0], w_h_[1]};
                }
                double framerate() {
                    assert(framerate_ != nullptr);
                    return *framerate_;
                }
                uint8_t bits_are_packed() {
                    assert(bits_are_packed_ != nullptr);
                    return *bits_are_packed_;
                }
                std::endian endianness() {
                    assert(endianness_ != nullptr);
                    switch (*endianness_) {
                        case 'b':
                            return std::endian::big;
                        case 'l':
                            return std::endian::little;
                        default:
                            // FIXME show error message
                            return std::endian::little;
                    }
                }
                uint32_t frame_count() {
                    assert(frame_count_ != nullptr);
                    return *frame_count_;
                }
                pair32 offset() {
                    assert(offset_ != nullptr);
                    return {offset_[0], offset_[1]};
                }
                pair32 sensor_size() {
                    assert(sensor_size_ != nullptr);
                    return {sensor_size_[0], sensor_size_[1]};
                }
                uint64_t clock_speed() {
                    assert(clock_speed_ != nullptr);
                    return *clock_speed_;
                }
                double exposure_time_ms() {
                    assert(exposure_time_ms_ != nullptr);
                    return *exposure_time_ms_;
                }
                double gain() {
                    assert(gain_ != nullptr);
                    return *gain_;
                }
                std::string_view system_identification() {
                    assert(system_identification_ != nullptr);
                    return *system_identification_;
                }

                // setters
                void header_size(size_t value) {
                    assert(header_size_ != nullptr);
                    *header_size_ = static_cast<uint32_t>(value);
                }
                void camera_type(const std::string_view &value) {
                    assert(camera_type_ != nullptr);
                    throw "FIXME";
                }
                void version(double value) {
                    assert(version_ != nullptr);
                    *version_ = value;
                }
                void image_type(const std::string_view &value) {
                    assert(image_type_ != nullptr);
                    throw "FIXME";
                }
                void bytes_per_pixel(uint32_t value) {
                    assert(bytes_per_pixel_ != nullptr);
                    *bytes_per_pixel_ = value;
                }
                void frame_bytes_on_disk(uint32_t value) {
                    assert(frame_bytes_on_disk_ != nullptr);
                    *frame_bytes_on_disk_ = value;
                }
                void w_h(pair32 value) {
                    assert(w_h_ != nullptr);
                    w_h_[0] = value[0];
                    w_h_[1] = value[1];
                }
                void framerate(double value) {
                    assert(framerate_ != nullptr);
                    *framerate_ = value;
                }
                void bits_are_packed(uint8_t value) {
                    assert(bits_are_packed_ != nullptr);
                    *bits_are_packed_ = value;
                }
                void endianness(std::endian value) {
                    assert(endianness_ != nullptr);
                    switch (value) {
                        case std::endian::big:
                            endianness_[0] = 'b';
                            break;
                        case std::endian::little:
                            endianness_[0] = 'l';
                            break;
                    }
                }
                void frame_count(uint32_t value) {
                    assert(frame_count_ != nullptr);
                    *frame_count_ = value;
                }
                void offset(pair32 value) {
                    assert(offset_ != nullptr);
                    offset_[0] = value[0];
                    offset_[1] = value[1];
                }
                void sensor_size(pair32 value) {
                    assert(sensor_size_ != nullptr);
                    sensor_size_[0] = value[0];
                    sensor_size_[1] = value[1];
                }
                void clock_speed(uint64_t value) {
                    assert(clock_speed_ != nullptr);
                    *clock_speed_ = value;
                }
                void exposure_time_ms(double value) {
                    assert(exposure_time_ms_ != nullptr);
                    *exposure_time_ms_ = value;
                }
                void gain(double value) {
                    assert(gain_ != nullptr);
                    *gain_ = value;
                }
                void system_identification(const std::string_view &value) {
                    assert(system_identification_ != nullptr);
                    throw "FIXME";
                }
            };

        public:
        };

        ///@}

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_CCV_HH
