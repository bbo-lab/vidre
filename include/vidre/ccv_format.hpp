#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

#include "config.h"


namespace vidre {

    struct GreenbergString {
        uint32_t length; 
        std::string data;
        GreenbergString()
        {
            data= "basler";
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

    struct ccv_cam_header {
        uint32_t headerSize = 93;
        GreenbergString cameraType;
        double headerVersion = 0.12;
        GreenbergString imageType;

        uint32_t bytesPerPixel;
        uint32_t bitsPerPixel;
        uint32_t frameBytesOnDisk;

        uint32_t width;
        uint32_t height;
        double frameRate;

        uint8_t packed;

        uint32_t frameCount;

        SensorInfo sensor;
    };

    struct ccv_frame_tail {
        uint32_t frameIndex;
        double timeStamp;
        uint8_t padding[9];  
    };
    
}  // namespace vidre