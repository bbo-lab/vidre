#pragma once
#include <cstdint>
#include <cstddef>

#ifndef CCV_CAM_HEADER_VERSION
#define CCV_CAM_HEADER_VERSION 0.12
#endif

namespace vidre {
    typedef double ccv_timestamp_t;
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

// #if double CCV_CAM_HEADER_VERSION >= 0.12
        byte_t packed;
// #endif

        uint32_t frameCount;
    };

    struct ccv_frame_tail {
        uint32_t frameIndex;
        ccv_timestamp_t timeStamp;
        byte_t padding[9];  // Probably for additional custom data
    };
}  // namespace vidre
