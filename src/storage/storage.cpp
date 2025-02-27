#include "vidre/storage.h"
#include "vidre/shared_frame_header.hpp"

namespace vidre {

    Storage::Storage(const Config& config)
    {
        config_ = config; 
        const std::string newMemoryName = "vidre";
        boost::interprocess::shared_memory_object::remove(newMemoryName.c_str());
        size_t capacity = (sizeof(SharedFrameHeader) + config_.sourceWidth * config_.sourceHeight) * 
                          (config_.sourceMaxNumBuffer + 1);
        shared_memory_ = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::open_or_create, newMemoryName.c_str(), capacity
        );
    }     

    void Storage::SetConfig(const Config& config)
    {
        config_ = config;
    }  
    
    Config Storage::GetConfig()
    {
        return config_;
    }


    void Storage::Save(const void *data_ptr, size_t data_size, int frame) {
        Pylon::CPylonImage pylonImage;

        Pylon::CImageFormatConverter formatConverter;
        formatConverter.OutputPixelFormat = Pylon::PixelType_BGR8packed;
        formatConverter.Convert(pylonImage, data_ptr, data_size, config_.sourcePixelType, config_.sourceWidth, config_.sourceHeight,
                                config_.sourcePaddingX, Pylon::EImageOrientation::ImageOrientation_TopDown);

        std::string file_name = std::to_string(frame) + ".png";
        Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Tiff, file_name.c_str(), pylonImage);
    }

    boost::interprocess::managed_shared_memory* Storage::GetSharedMemory()
    {
        return shared_memory_.get(); 
    }
    
}  // namespace vidre