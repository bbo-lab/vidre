#include "ccv_format.hpp"
#include "vidre/storage.hpp"

#include <fstream>
#include <iostream>

namespace vidre {
    vidre::CamCommandoHeader::CamCommandoHeader() {
        headerSize += (cameraType.length + 1) + (imageType.length + 1);
        
        bytesPerPixel = 1; // PixelType_Mono8; 
        bitsPerPixel = 8; // PixelType_Mono8;
    }
    vidre::CamCommandoHeader::CamCommandoHeader(
        vidre::Config config,
        std::string camera_type, 
        std::string image_type
    ) {
        cameraType = GreenbergString(camera_type);
        imageType = GreenbergString(image_type);

        headerSize += (cameraType.length + 1) + (imageType.length + 1);
        
        bytesPerPixel = 1; // PixelType_Mono8; 
        bitsPerPixel = 8; // PixelType_Mono8;

        width = config.width;
        height = config.height;

        frameBytesOnDisk = width * height * bytesPerPixel;
        frameBytesOnDisk += 21; // size of tail
        
        frameRate = config.frameRate;  

#if CCV_HEADER_VERSION >= 120
        packed = 0;
#endif
        frameCount = 0;
    }

    void vidre::CamCommandoHeader::initiateHeader(FILE* out_file) {
        writeHeader(out_file);
    }

    void vidre::CamCommandoHeader::updateHeader(FILE* out_file, int frames) {
        frameCount = frames;
        rewind(out_file);
        writeHeader(out_file);
    }

    void vidre::CamCommandoHeader::writeFrame(const void* buffer, FILE* out_file) {
        fwrite(reinterpret_cast<const char*>(buffer), 1, frameBytesOnDisk - 21, out_file);
        // file.write(static_cast<const char*>(buffer), frameBytesOnDisk - 21);
    }

    void vidre::CamCommandoHeader::writeTail(FILE* out_file, const CCVFrameTail& tail) {
        writeField(out_file, tail.frameIndex);
        writeField(out_file, tail.timeStamp);
        writeField(out_file, tail.padding);
    }

    template <typename T>
    void vidre::CamCommandoHeader::writeField(FILE* outFile, const T& value) {
        fwrite(reinterpret_cast<const char *>(&value), 1, sizeof(T), outFile);
        // outFile.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <>
    void vidre::CamCommandoHeader::writeField<std::string>(FILE* outFile, const std::string& value) {
        fwrite(value.c_str(), 1, value.length() + 1, outFile);
        // outFile.write(value.c_str(), value.length() + 1);
    }

    void vidre::CamCommandoHeader::writeHeader(FILE* outFile) {
        writeField(outFile, headerSize);
        writeField(outFile, cameraType.length);
        writeField<std::string>(outFile, cameraType.data);
        writeField(outFile, headerVersion);
        writeField(outFile, imageType.length);
        writeField<std::string>(outFile, imageType.data);

        writeField(outFile, bytesPerPixel);
        writeField(outFile, bitsPerPixel);
        writeField(outFile, frameBytesOnDisk);

        writeField(outFile, width);
        writeField(outFile, height);
        writeField(outFile, frameRate);
#if CCV_HEADER_VERSION >= 120
        writeField(outFile, packed);
#endif
        writeField(outFile, frameCount);
#if CCV_HEADER_VERSION >= 120
        writeField(outFile, sensor.offset);
        writeField(outFile, sensor.size);
        writeField(outFile, sensor.clock);
        writeField(outFile, sensor.exposure);
        writeField(outFile, sensor.gain);
#endif
    }
}