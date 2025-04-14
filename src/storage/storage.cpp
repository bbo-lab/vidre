#include "vidre/storage.hpp"
#include "vidre/shared_frame_header.hpp"

#include "header/ccv_format.hpp"

namespace vidre {

    Storage::Storage(const Config& config, const std::string& header_format, const std::string& output_file_name) 
    : output_file(fopen(output_file_name.c_str(), "wb"), fclose)
    {
        _config = config; 

        // todo: enum
        _headerFormat = header_format;
        if (_headerFormat == "ccv") {
            _header = std::make_unique<CamCommandoHeader>(config);
        }

        _outputFileName = output_file_name;

        _header->initiateHeader(output_file.get());
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
        formatConverter.Convert(pylonImage, buffer, _config.width * _config.height, _config.pixelType, _config.width, _config.height,
                                _config.paddingX, Pylon::EImageOrientation::ImageOrientation_TopDown);

        std::string fileName = std::to_string(frameID) + ".tiff";
        Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Tiff, fileName.c_str(), pylonImage);
    }

    void Storage::updateHeader(int frames) {
        _header->updateHeader(output_file.get(), frames);
    }

    void Storage::appendFrame(const void* buffer) {
        _header->writeFrame(buffer, output_file.get());
    }

    void Storage::appendTail(uint32_t frame_idx, double time_stamp, long computer_time_stamp) {
        if (_headerFormat == "ccv") {
            vidre::CCVFrameTail tail;
            tail.frameIndex = frame_idx;  // + result->numSkippedBefore? TODO: think about skipped logic
            tail.timeStamp = time_stamp;
            tail.timeStamp /= 1e9;

            for (int i = 0; i < sizeof(long) && i < 9; i++) {
                tail.padding[i] = (computer_time_stamp >> (i * 8)) & 0xFF;
            }
            
            _header->writeTail<CamCommandoHeader>(output_file.get(), tail);
        }
    }
}  // namespace vidre