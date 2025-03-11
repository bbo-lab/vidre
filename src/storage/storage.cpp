#include "vidre/storage.h"
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

    Storage::Storage(const Config& config, std::string output_file_name_)
    {
        config_ = config; 

        // Shared Memory
        boost::interprocess::shared_memory_object::remove(config_.sourceSerialNumber.c_str());
        size_t capacity = (sizeof(SharedFrameHeader) + config_.sourceWidth * config_.sourceHeight) * 
                          (config_.sourceMaxNumBuffer + 1);

        // !!! Just to show for yaml_test, (default dummy  cam set up has too low capacity)
        capacity *= capacity > 32000 ? 1 : 1000;
        try {
            shared_memory_ = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::open_or_create, config_.sourceSerialNumber.c_str(), capacity
        );
        } catch (boost::interprocess::interprocess_exception const& iExcept) {
            std::cout <<"BOOST CREATING SHARED MEMORY EXCEPTION: \n" << iExcept.what() << std::endl;
            return;
        }

        // ccv staff
        ccv_header.headerSize += (ccv_header.cameraType.length + 1) + (ccv_header.imageType.length + 1);
        ccv_header.bytesPerPixel = 1; // PixelType_Mono8;
        ccv_header.bitsPerPixel = 8; // PixelType_Mono8;
        ccv_header.width = config_.sourceWidth;
        ccv_header.height = config_.sourceHeight;
        ccv_header.frameBytesOnDisk = ccv_header.width * ccv_header.height * ccv_header.bytesPerPixel;
        ccv_header.frameBytesOnDisk += 21; // size of tail
        ccv_header.frameRate = config_.sourceFrameRate;  
        ccv_header.packed = 0;
        ccv_header.frameCount = 0;

        // initial write of header
        output_file_name = output_file_name_;
        std::ofstream file(output_file_name, std::ios::out | std::ios::binary | std::ios::trunc);
        writeHeader(file, ccv_header);
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
    
    void Storage::UpdateCCVHeader()
    {
        ++ccv_header.frameCount; // todo: skipped frames logic
        std::ofstream file(output_file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0, std::ios::beg);
        writeHeader(file, ccv_header);
    }

    void Storage::AppendCCVFrame(const void* buffer, ccv_frame_tail tail)
    {
        std::ofstream file(output_file_name, std::ios::out | std::ios::binary | std::ios::app);

        // write frame
        file.write(static_cast<const char*>(buffer), ccv_header.frameBytesOnDisk - 21);

        // write tail
        writeField(file, tail.frameIndex);
        writeField(file, tail.timeStamp);
        writeField(file, tail.padding);
    }
}  // namespace vidre