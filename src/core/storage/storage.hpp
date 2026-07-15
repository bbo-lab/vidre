#pragma once

// #include <pylon/PylonIncludes.h>
// #include <pylon/ImagePersistence.h>

#include <boost/interprocess/managed_shared_memory.hpp>

#include <config.h>
#include <core/header/header.hpp>
#include <core/runtime/shared_frame_header.hpp>
#include <core/storage/disk_writer.hpp>

#include <variant>



namespace vidre {
    struct CCVFrameTail;

    struct Config {
        // todo: rm
        //Pylon::EPixelType pixelType;
        
        // frame
        uint32_t width;
        uint32_t height;
        uint32_t maxNumBuffer;
        size_t paddingX;
        double frameRate;

        // cam specific
        uint32_t frameBytesActual;
        uint32_t frameBytesBuffer;
        std::string serialNumber;
        
        // for header
        uint64_t bitsPerPixel;
        bool packed;
        std::string cameraVendor;
    };

    class Storage {
    public:
        Storage() = default;
        Storage(const Config& config, const std::string& header_format = "ccv", const std::string& output_file_name = "some.ccv");
        ~Storage() {};

        void setConfig(const Config& config);
        Config getConfig();

        void updateHeader(int frames);
        void appendFrame(const void* buffer);
        void appendTail(uint32_t frame_idx, double time_stamp, int64_t computer_time_stamp);
        void initiateHeader();

        std::string getOutputFileName() {
            return _outputFileName;
        }
    private:
        Config _config;
        std::string _outputFileName = "some.ccv";
        std::string _headerFormat = "ccv";
        
        std::unique_ptr<IHeader> _header;
        DiskWriter output_file;
    };
}  // namespace vidre