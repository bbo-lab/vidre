#include "vidre/storage.h"
#include "vidre/shared_frame_header.hpp"

namespace vidre {

    Storage::Storage(const Config& config, std::string outputFileName)
    {
        _config = config; 

        // Shared Memory
        const std::string newMemoryName = outputFileName;
        boost::interprocess::shared_memory_object::remove(newMemoryName.c_str());
        size_t capacity = (sizeof(SharedFrameHeader) + _config.sourceWidth * _config.sourceHeight) * 
                          (_config.sourceMaxNumBuffer + 1);

        _sharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::open_or_create, newMemoryName.c_str(), capacity);

        // ccv staff
        _CCVHeader.headerSize = sizeof(vidre::ccv_cam_header);
        
        // todo : from config and resolve depending on it
        _CCVHeader.bytesPerPixel = 1; // PixelType_Mono8; 
        _CCVHeader.bitsPerPixel = 8; // PixelType_Mono8;

        _CCVHeader.width = _config.sourceWidth;
        _CCVHeader.height = _config.sourceHeight;
        _CCVHeader.frameBytesOnDisk = _CCVHeader.width * _CCVHeader.height * _CCVHeader.bytesPerPixel;
        _CCVHeader.frameRate = _config.sourceFrameRate;  
        _CCVHeader.packed = 0;
        _CCVHeader.frameCount = 0; // not constant

        // initial write of header
        _outputFileName = outputFileName;
        std::ofstream file(_outputFileName, std::ios::out | std::ios::binary | std::ios::trunc);
        file.write(reinterpret_cast<char*>(&_CCVHeader), sizeof(_CCVHeader));
        file.close();
    }     

    void Storage::SetConfig(const Config& config)
    {
        _config = config;
    }  
    
    Config Storage::GetConfig()
    {
        return _config;
    }


    void Storage::Save(const void *buffer, size_t dataSize, int frameID) {
        Pylon::CPylonImage pylonImage;

        Pylon::CImageFormatConverter formatConverter;
        formatConverter.OutputPixelFormat = Pylon::PixelType_BGR8packed;
        formatConverter.Convert(pylonImage, buffer, dataSize, _config.sourcePixelType, _config.sourceWidth, _config.sourceHeight,
                                _config.sourcePaddingX, Pylon::EImageOrientation::ImageOrientation_TopDown);

        std::string fileName = std::to_string(frameID) + ".png";
        Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Tiff, fileName.c_str(), pylonImage);
    }

    boost::interprocess::managed_shared_memory* Storage::GetSharedMemory()
    {
        return shared_memory_.get(); 
    }
    
    void Storage::UpdateCCVHeader(uint32_t frameID)
    {
        _CCVHeader.frameCount = frameID; // todo: skipped frames logic
        std::fstream file(_outputFileName, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<char*>(&_CCVHeader), sizeof(_CCVHeader));
        file.close();
    }

    void Storage::AppendCCVFrame(const void* buffer, ccv_frame_tail tail)
    {
        std::ofstream file(_outputFileName, std::ios::out | std::ios::binary | std::ios::app);

        // write frame
        file.write(static_cast<const char*>(buffer), _CCVHeader.frameBytesOnDisk);

        // write tail
        file.write(reinterpret_cast<char*>(&tail), sizeof(tail));

        file.close();
    }
}  // namespace vidre