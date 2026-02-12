#include "storage.hpp"
#include "core/runtime/shared_frame_header.hpp"
#include "core/header/ccv/ccv_format.hpp"

#include <format>
#include <chrono>
#include <filesystem>
#include <iostream>


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
    }     

    void Storage::setConfig(const Config& config) {
        _config = config;
    }  
    
    Config Storage::getConfig() {
        return _config;
    }

    void Storage::save(const void* buffer, uint32_t frameID) { // TODO: ? 
        // Pylon::CPylonImage pylonImage;

        // Pylon::CImageFormatConverter formatConverter;
        // formatConverter.OutputPixelFormat = Pylon::PixelType_BGR8packed;
        // formatConverter.Convert(pylonImage, buffer, _config.width * _config.height, _config.pixelType, _config.width, _config.height,
        //                         _config.paddingX, Pylon::EImageOrientation::ImageOrientation_TopDown);

        // std::string fileName = std::to_string(frameID) + ".tiff";
        // Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Tiff, fileName.c_str(), pylonImage);
        return;
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

    void Storage::initiateHeader() { // todo -> start all unified time  for all cameras
        // adding ISO format for output file
        auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        std::string timestamp = std::format("{:%Y%m%dT%H%M seconds %S}", time); // seconds
        https://github.com/bbo-lab/calipy
        auto old_name = _outputFileName;
        std::filesystem::path p(_outputFileName);
        std::filesystem::path newPath = date first p.parent_path() / (p.stem().string() + "_" + timestamp + p.extension().string());

        _outputFileName = newPath.string();
        try {
            output_file.reset();
            std::filesystem::rename(old_name, _outputFileName);
            output_file.reset(fopen(_outputFileName.c_str(), "wb"));
            // allocating header
            _header->initiateHeader(output_file.get());
        } 
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}  // namespace vidre