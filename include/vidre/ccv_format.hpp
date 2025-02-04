#pragma once
#include <cstdint>
#include <cstddef>

#ifndef CCV_CAM_HEADER_VERSION
#define CCV_CAM_HEADER_VERSION 0.12
#endif

namespace vidre {
    typedef double timestamp_t;
    typedef char byte_t;

    struct ccv_cam_header {
        uint32_t headerSize;
        const double headerVersion = CCV_CAM_HEADER_VERSION;

        uint32_t bytesPerPixel;
        uint32_t bitsPerPixel;
        uint32_t frameBytesOnDisk;

        uint32_t width;
        uint32_t height;
        double frameRate;

#if double CCV_CAM_HEADER_VERSION >= 0.12
        byte_t packed;
#endif

        uint32_t frameCount;

#if double CCV_CAM_HEADER_VERSION >= 0.12
        struct ccv_cam_sensor_header {
            uint32_t offset[2];
            uint32_t size[2];
            uint64_t clock;
            double exposure;
            double gain;
        } sensor;
#endif
    };

    struct ccv_frame_tail {
        uint32_t frameIndex;
        timestamp_t timeStamp;
        byte_t padding[9];  // Probably for additional custom data
    };
}  // namespace vidre
