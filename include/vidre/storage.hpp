#pragma once

#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>

#include <boost/interprocess/managed_shared_memory.hpp>

#include "config.h"
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
        std::string sourceSerialNumber;
    };

    class Storage {
    public:
        Storage() {};
        Storage(const Config& config, std::string outputFileName = "some.ccv");
        ~Storage() {};

        void setConfig(const Config& config);
        Config getConfig();
        void save(const void *buffer, uint32_t frameID);
        boost::interprocess::managed_shared_memory* getSharedMemory();
        void updateCCVHeader();
        void appendCCVFrame(const void* buffer, CCVFrameTail tail);
    private:
        std::unique_ptr<boost::interprocess::managed_shared_memory> _sharedMemory;
        Config _config;
        std::string _outputFileName = "some.ccv";
        CamCommandoHeader _CCVHeader;
    };
}  // namespace vidre