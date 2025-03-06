#pragma once
#include "config.h"
#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "vidre/ccv_format.hpp"

#include <fstream>
#include <iostream>

namespace vidre {
    struct Config {
        Pylon::EPixelType sourcePixelType;
        uint32_t sourceWidth;
        uint32_t sourceHeight;
        uint32_t sourceMaxNumBuffer;
        size_t sourcePaddingX;
        double sourceFrameRate;
    };

    class Storage {
    public:
        Storage() {};
        Storage(const Config& config, std::string outputFileName = "some.ccv");
        ~Storage() {};

        void setConfig(const Config& config);
        Config getConfig();
        void save(const void *buffer, size_t dataSize, int frame);
        boost::interprocess::managed_shared_memory* getSharedMemory();
        void updateCCVHeader(uint32_t frameID);
        void appendCCVFrame(const void* buffer, ccv_frame_tail tail);
    private:
        std::unique_ptr<boost::interprocess::managed_shared_memory> _sharedMemory;
        Config _config;
        std::string _outputFileName = "some.ccv";
        ccv_cam_header _CCVHeader;
    };
}  // namespace vidre