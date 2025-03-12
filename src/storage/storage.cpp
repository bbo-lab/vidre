#include "vidre/storage.hpp"
#include "vidre/shared_frame_header.hpp"

template <typename T>
void writeField(std::ofstream& outFile, const T& value) {
    outFile.write(reinterpret_cast<const char*>(&value), sizeof(T));
}
void writeString(std::ofstream& outFile, const std::string& str) {
    outFile.write(str.c_str(), str.length() + 1); // Write the string, including null terminator
}
void writeHeader(std::ofstream& outFile, vidre::ccv_cam_header h)
{
        writeField(outFile, h.headerSize);
        writeField(outFile, h.cameraType.length);
        writeString(outFile, h.cameraType.data);
        writeField(outFile, h.headerVersion);
        writeField(outFile, h.imageType.length);
        writeString(outFile, h.imageType.data);

        writeField(outFile, h.bytesPerPixel);
        writeField(outFile, h.bitsPerPixel);
        writeField(outFile, h.frameBytesOnDisk);

        writeField(outFile, h.width);
        writeField(outFile, h.height);
        writeField(outFile, h.frameRate);

        writeField(outFile, h.packed);

        writeField(outFile, h.frameCount);

        writeField(outFile, h.sensor.offset);
        writeField(outFile, h.sensor.size);
        writeField(outFile, h.sensor.clock);
        writeField(outFile, h.sensor.exposure);
        writeField(outFile, h.sensor.gain);
}

namespace vidre {

    Storage::Storage(const Config& config, std::string outputFileName) {
        _config = config; 

        // Shared Memory
        boost::interprocess::shared_memory_object::remove(_config.sourceSerialNumber.c_str());
        size_t capacity = (sizeof(SharedFrameHeader) + _config.sourceWidth * _config.sourceHeight) * 
                          (_config.sourceMaxNumBuffer + 1);

        _sharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::open_or_create, _config.sourceSerialNumber.c_str(), capacity);

        // ccv staff
        _CCVHeader.headerSize += (_CCVHeader.cameraType.length + 1) + (_CCVHeader.imageType.length + 1);
        
        // todo : from config and resolve depending on it
        _CCVHeader.bytesPerPixel = 1; // PixelType_Mono8; 
        _CCVHeader.bitsPerPixel = 8; // PixelType_Mono8;

        _CCVHeader.width = _config.sourceWidth;
        _CCVHeader.height = _config.sourceHeight;
        _CCVHeader.frameBytesOnDisk = _CCVHeader.width * _CCVHeader.height * _CCVHeader.bytesPerPixel;
        _CCVHeader.frameBytesOnDisk += 21; // size of tail
        _CCVHeader.frameRate = _config.sourceFrameRate;  

#if CCV_HEADER_VERSION >= 0.12
        _CCVHeader.packed = 0;
#endif
        _CCVHeader.frameCount = 0; // not constant

        // initial write of header
        _outputFileName = outputFileName;
        std::ofstream file(_outputFileName, std::ios::out | std::ios::binary | std::ios::trunc);
        writeHeader(file, _CCVHeader);
    }     

    void Storage::setConfig(const Config& config) {
        _config = config;
    }  
    
    Config Storage::getConfig() {
        return _config;
    }

    void Storage::save(const void* buffer, uint32_t frameID) {
        Pylon::CPylonImage pylonImage;

        Pylon::CImageFormatConverter formatConverter;
        formatConverter.OutputPixelFormat = Pylon::PixelType_BGR8packed;
        formatConverter.Convert(pylonImage, buffer, _CCVHeader.frameBytesOnDisk, _config.sourcePixelType, _config.sourceWidth, _config.sourceHeight,
                                _config.sourcePaddingX, Pylon::EImageOrientation::ImageOrientation_TopDown);

        std::string fileName = std::to_string(frameID) + ".tiff";
        Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Tiff, fileName.c_str(), pylonImage);
    }

    boost::interprocess::managed_shared_memory* Storage::getSharedMemory() {
        return _sharedMemory.get(); 
    }
    
    void Storage::UpdateCCVHeader()
    {
        ++_CCVHeader.frameCount;
        std::ofstream file(_outputFile, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0, std::ios::beg);
        writeHeader(file, _CCVHeader);
    }

    void Storage::appendCCVFrame(const void* buffer, ccv_frame_tail tail) {
        fseek(_outputFile.get(), 0, SEEK_END);

        // write frame
        file.write(static_cast<const char*>(buffer), _CCVHeader.frameBytesOnDisk - 21);

        // write tail
        writeField(file, tail.frameIndex);
        writeField(file, tail.timeStamp);
        writeField(file, tail.padding);
    }
}  // namespace vidre