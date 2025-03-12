#pragma once


#include <cstdint>
#include <cstddef>
#include <array>

#include "config.h"

#include "vidre/config.h"


namespace vidre {

    struct GreenbergString {
        uint32_t length; 
        std::vector<uint8_t> data;
        uint8_t terminator = 0;
    
        GreenbergString(uint32_t len, const std::vector<uint8_t>& input_data)
            : length(len), data(input_data) {}
    };

    struct SensorInfo {
        uint32_t offset[2];
        uint32_t size[2];
        int64_t clock;
        double exposure;
        double gain;
    };

    struct CamCommandoHeader {
        uint32_t headerSize;
        GreenbergString cameraType;
        double headerVersion;
        GreenbergString imageType;

        uint32_t bytesPerPixel;
        uint32_t bitsPerPixel;
        uint32_t frameBytesOnDisk;

        uint32_t width;
        uint32_t height;
        double frameRate;

#if CCV_HEADER_VERSION >= 0.12
        uint8_t packed;
#endif

        uint32_t frameCount;

#if CCV_HEADER_VERSION >= 0.12
        SensorInfo sensor;
#endif
    };

    struct CCVFrameTail {
        uint32_t frameIndex;
        double timeStamp;
        uint8_t padding[9];  
        double timeStamp;
        uint8_t padding[9];  
    };
}  // namespace vidre