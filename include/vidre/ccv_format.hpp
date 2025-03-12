#pragma once


#include <cstdint>
#include <cstddef>
#include <array>

#include "config.h"


namespace vidre {

    struct GreenbergString {
        uint32_t length; 
        std::string data;
        GreenbergString() {
            data = "basler";
            length = data.length();
        }
    };

    struct SensorInfo {
        uint32_t offset[2];
        uint32_t size[2];
        int64_t clock;
        double exposure;
        double gain;
    };

    struct CamCommandoHeader {
#if CCV_HEADER_VERSION >= 120
        uint32_t headerSize = 93;
#else
        uint32_t headerSize = 52;
#endif
        GreenbergString cameraType;
        double headerVersion = CCV_HEADER_VERSION_MAJOR + CCV_HEADER_VERSION_MINOR / 100.0;
        GreenbergString imageType;

        uint32_t bytesPerPixel;
        uint32_t bitsPerPixel;
        uint32_t frameBytesOnDisk;

        uint32_t width;
        uint32_t height;
        double frameRate;

#if CCV_HEADER_VERSION >= 120
        uint8_t packed;
#endif

        uint32_t frameCount;

#if CCV_HEADER_VERSION >= 120
        SensorInfo sensor;
#endif
    };

    struct CCVFrameTail {
        uint32_t frameIndex;
        double timeStamp;
        uint8_t padding[9];  
    };
}  // namespace vidre