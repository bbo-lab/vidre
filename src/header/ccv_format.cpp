#include "ccv_format.hpp"

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

    void vidre::CamCommandoHeader::initiateHeader(const std::string& out_file) {
        std::ofstream file(out_file, std::ios::out | std::ios::binary | std::ios::trunc);
        writeHeader(file);
    }

    void vidre::CamCommandoHeader::updateHeader(const std::string& out_file) {
        ++frameCount;
        std::ofstream file(out_file, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0, std::ios::beg);
        writeHeader(file);
    }

    void vidre::CamCommandoHeader::writeFrame(const void* buffer, const std::string& out_file) {
        std::ofstream file(out_file, std::ios::out | std::ios::binary | std::ios::app);
        file.write(static_cast<const char*>(buffer), frameBytesOnDisk - 21);
    }

    void vidre::CamCommandoHeader::writeTail(const std::string& out_file, const CCVFrameTail& tail) {
        std::ofstream file(out_file, std::ios::out | std::ios::binary | std::ios::app); 
        writeField(file, tail.frameIndex);
        writeField(file, tail.timeStamp);
        writeField(file, tail.padding);
    }

    template <typename T>
    void vidre::CamCommandoHeader::writeField(std::ofstream& outFile, const T& value) {
        outFile.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <>
    void vidre::CamCommandoHeader::writeField<std::string>(std::ofstream& outFile, const std::string& value) {
        outFile.write(value.c_str(), value.length() + 1);
    }

    void vidre::CamCommandoHeader::writeHeader(std::ofstream& outFile) {
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