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
        std::string sourceSerialNumber;
    };

    class Storage {
    public:
        Storage() {};
        Storage(const Config& config, std::string output_file_name_ = "some.ccv");
        ~Storage() {};

        void SetConfig(const Config& config);
        Config GetConfig();
        void Save(const void *data_ptr, size_t data_size, int frame);
        boost::interprocess::managed_shared_memory* GetSharedMemory();
        void UpdateCCVHeader();
        void AppendCCVFrame(const void* buffer, ccv_frame_tail tail);
    private:
        std::unique_ptr<boost::interprocess::managed_shared_memory> shared_memory_;
        Config config_;
        std::string output_file_name = "some.ccv";
        ccv_cam_header ccv_header;
    };
}  // namespace vidre