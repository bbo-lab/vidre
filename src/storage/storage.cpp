#include "vidre/storage.h"
#include "vidre/shared_frame_header.hpp"

namespace vidre {

    Storage::Storage(const Config& config)
    {
        config_ = config; 

        // Shared Memory
        const std::string newMemoryName = "vidre";
        boost::interprocess::shared_memory_object::remove(newMemoryName.c_str());
        size_t capacity = (sizeof(SharedFrameHeader) + config_.sourceWidth * config_.sourceHeight) * 
                          (config_.sourceMaxNumBuffer + 1);
        shared_memory_ = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::open_or_create, newMemoryName.c_str(), capacity
        );


        // ccv staff
        ccv_header.headerSize = sizeof(vidre::ccv_cam_header);
        ccv_header.bytesPerPixel = 1; // PixelType_Mono8;
        ccv_header.bitsPerPixel = 8; // PixelType_Mono8;
        ccv_header.width = config_.sourceWidth;
        ccv_header.height = config_.sourceHeight;
        ccv_header.frameBytesOnDisk = ccv_header.width * ccv_header.height * ccv_header.bytesPerPixel;
        ccv_header.frameRate = config_.sourceFrameRate;  
        ccv_header.packed = 0;
        ccv_header.frameCount = 0;

        // initial write of header
        std::ofstream file(output_file_name, std::ios::out | std::ios::binary | std::ios::trunc);
        file.write(reinterpret_cast<char*>(&ccv_header), sizeof(ccv_header));
        file.close();
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
    
    void Storage::UpdateCCVHeader(uint32_t frame_id)
    {
        ccv_header.frameCount = frame_id; // todo: skipped frames logic
        std::fstream file(output_file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<char*>(&ccv_header), sizeof(ccv_header));
        file.close();
    }

    void Storage::AppendCCVFrame(const void* buffer, ccv_frame_tail tail)
    {
        std::ofstream file(output_file_name, std::ios::out | std::ios::binary | std::ios::app);

        // write frame
        file.write(static_cast<const char*>(buffer), ccv_header.frameBytesOnDisk);

        // write tail
        file.write(reinterpret_cast<char*>(&tail), sizeof(tail));

        file.close();
    }
}  // namespace vidre