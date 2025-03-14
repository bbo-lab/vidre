#include "vidre/storage.hpp"
#include "vidre/shared_frame_header.hpp"
namespace vidre {

    Storage::Storage(const Config& config, std::string outputFileName) {
        _config = config; 

        // Shared Memory
        boost::interprocess::shared_memory_object::remove(_config.sourceSerialNumber.c_str());
        size_t capacity = (sizeof(SharedFrameHeader) + _config.sourceWidth * _config.sourceHeight) * 
                          (_config.sourceMaxNumBuffer + 1);

        _sharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::open_or_create, _config.sourceSerialNumber.c_str(), capacity);
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
        formatConverter.Convert(pylonImage, buffer, _CCVHeader.frameBytesOnDisk - 21, _config.sourcePixelType, _config.sourceWidth, _config.sourceHeight,
                                _config.sourcePaddingX, Pylon::EImageOrientation::ImageOrientation_TopDown);

        std::string fileName = std::to_string(frameID) + ".tiff";
        Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Tiff, fileName.c_str(), pylonImage);
    }

    boost::interprocess::managed_shared_memory* Storage::getSharedMemory() {
        return _sharedMemory.get(); 
    }
    
    void Storage::updateCCVHeader()
    {
        ++_CCVHeader.frameCount;
        std::ofstream file(_outputFileName, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0, std::ios::beg);
        writeHeader(file, _CCVHeader);
    }

    void Storage::appendCCVFrame(const void* buffer, CCVFrameTail tail) {
        
        std::ofstream file(_outputFileName, std::ios::out | std::ios::binary | std::ios::app);

        // write frame
        file.write(static_cast<const char*>(buffer), _CCVHeader.frameBytesOnDisk - 21);

        // write tail
        writeField(file, tail.frameIndex);
        writeField(file, tail.timeStamp);
        writeField(file, tail.padding);
    }
}  // namespace vidre