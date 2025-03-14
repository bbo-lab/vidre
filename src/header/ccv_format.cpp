#include "vidre/ccv_format.hpp"

vidre::CamCommandoHeader::CamCommandoHeader() {
    // ccv staff
    headerSize += (cameraType.length + 1) + (imageType.length + 1);
    
    // todo : from config and resolve depending on it
    bytesPerPixel = 1; // PixelType_Mono8; 
    bitsPerPixel = 8; // PixelType_Mono8;

    width = _config.sourceWidth;
    height = _config.sourceHeight;
    frameBytesOnDisk = width * height * bytesPerPixel;
    frameBytesOnDisk += 21; // size of tail
    frameRate = _config.sourceFrameRate;  

#if CCV_HEADER_VERSION >= 120
    packed = 0;
#endif
    frameCount = 0; // not constant

    // initial write of header
    _outputFileName = outputFileName;
    std::ofstream file(_outputFileName, std::ios::out | std::ios::binary | std::ios::trunc);
    writeHeader(file, _CCVHeader);
}

void vidre::CamCommandoHeader::initiateHeader() {
    std::ofstream file(_outputFileName, std::ios::out | std::ios::binary | std::ios::trunc);
    writeHeader(file, _CCVHeader);
}

void vidre::CamCommandoHeader::updateHeader() {
    frameCount;
    std::ofstream file(_outputFileName, std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(0, std::ios::beg);
    writeHeader(file, _CCVHeader);
}

void vidre::CamCommandoHeader::writeFrame() {
    std::ofstream file(_outputFileName, std::ios::out | std::ios::binary | std::ios::app);
    file.write(static_cast<const char*>(buffer), frameBytesOnDisk - 21);
}

void vidre::CamCommandoHeader::writeTail() {
    std::ofstream file(_outputFileName, std::ios::out | std::ios::binary | std::ios::app); 
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