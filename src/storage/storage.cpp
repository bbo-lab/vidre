#include "vidre/storage.hpp"
#include "vidre/shared_frame_header.hpp"
#include "ccv_format.hpp"
namespace vidre {

    Storage::Storage(const Config& config, const std::string& header_format, const std::string& output_file_name) {
        _config = config; 

        boost::interprocess::shared_memory_object::remove(_config.sourceSerialNumber.c_str());
        size_t capacity = (sizeof(SharedFrameHeader) + _config.sourceWidth * _config.sourceHeight) * 
                          (_config.sourceMaxNumBuffer + 1);

        _sharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::open_or_create, _config.sourceSerialNumber.c_str(), capacity);

        // todo: enum
        _headerFormat = header_format;
        if (_headerFormat == "ccv") {
            _header = CamCommandoHeader(config)
        }

        _outputFileName = output_file_name;
        _header.initiateHeader(_outputFileName);
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
    
    void Storage::updateHeader() {
        _header.updateHeader(_outputFileName);
    }

    void Storage::appendFrame(const void* buffer) {
        _header.writeFrame(buffer, _outputFileName);
    }

    void Storage::appendTail(uint32_t frame_idx, double time_stamp) {
        if (_headerFormat == "ccv") {
            vidre::CCVFrameTail tail;
            tail.frameIndex = frame_idx // + result->numSkippedBefore? TODO: think about skipped logic
            tail.timeStamp = time_stamp;
            tail.timeStamp /= 1e9;
            _header.writeTail(_outputFileName, tail);
        }
    }
}  // namespace vidre