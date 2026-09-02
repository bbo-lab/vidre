#pragma once

#include <config.h>

#include <core/header/header.hpp>
#include <vector>


namespace vidre {
    struct Config;
    class DiskWriter;

    struct GreenbergString {
        uint32_t length; 
        std::string data;

        GreenbergString() {
            data = "basler";
            length = data.length();
        }

        GreenbergString(std::string input) {
            data = input;
            length = data.length();
        }
    };

    struct SensorInfo {
        uint32_t offset[2] = {0, 0};
        uint32_t size[2] = {0, 0};
        int64_t clock = 0;
        double exposure = 0.0;
        double gain = 0.0;
    };

    struct CCVFrameTail {
        uint32_t frameIndex;
        double timeStamp;
        uint8_t padding[9]; //   use part of 9 bytes to add computer timestamp;
    };

    class CamCommandoHeader : public IHeader {
    public:
        CamCommandoHeader() = delete;
        CamCommandoHeader(Config config);
        virtual void initiateHeader(DiskWriter& out_file) override;
        virtual void updateHeader(DiskWriter& out_file, int frames) override;
        virtual void writeFrame(const void* buffer, size_t payloadSize, DiskWriter& out_file) override;        
        void writeTail(DiskWriter& out_file, const CCVFrameTail& tail) override;
    
    private:
        std::vector<uint8_t> serializeHeader();

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

        // Actual aptina buffer to be saved is smaller than minimum allocated
        uint64_t aptinaBuffer = 0;


        double frameRate;

#if CCV_HEADER_VERSION >= 120
        uint8_t packed;
#endif

        uint32_t frameCount;

#if CCV_HEADER_VERSION >= 120
        SensorInfo sensor{};
        std::vector<uint8_t> packedBuffer;
#endif
    };
}  // namespace vidre