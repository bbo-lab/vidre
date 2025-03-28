#pragma once

#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>

#include <boost/interprocess/managed_shared_memory.hpp>

#include "config.h"
#include "vidre/header.hpp"
#include "vidre/shared_frame_header.hpp"

#include <variant>



namespace vidre {
    struct CCVFrameTail;

    struct Config {
        Pylon::EPixelType pixelType;
        uint32_t width;
        uint32_t height;
        uint32_t maxNumBuffer;
        size_t paddingX;
        double frameRate;
        std::string serialNumber;
    };

    class Storage {
    public:
        Storage() : output_file(nullptr, fclose) {}
        Storage(const Config& config, const std::string& header_format = "ccv", const std::string& output_file_name = "some.ccv");
        ~Storage() {};

        void setConfig(const Config& config);
        Config getConfig();
        void save(const void* buffer, uint32_t frameID);
        std::vector<void*>* getSharedMemory();
        boost::interprocess::managed_shared_memory* getM()
        {
            return _sharedMemory.get();
        }
        void updateHeader(int frames);
        void appendFrame(const void* buffer);
        void appendTail(uint32_t frame_idx, double time_stamp);
        SharedFrameHeader* retrieveFrame(uint32_t frame_idx);
    private:
        std::unique_ptr<boost::interprocess::managed_shared_memory> _sharedMemory;
        std::unique_ptr<std::vector<void *>> ptrs;
        Config _config;
        std::string _outputFileName = "some.ccv";
        std::string _headerFormat = "ccv";
        
        std::unique_ptr<IHeader> _header;
        std::unique_ptr<FILE, decltype(&fclose)> output_file;
    };
}  // namespace vidre