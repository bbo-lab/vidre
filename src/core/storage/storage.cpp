#include <core/storage/storage.hpp>
#include <core/runtime/shared_frame_header.hpp>
#include <core/header/ccv/ccv_format.hpp>

#include <format>
#include <chrono>
#include <filesystem>
#include <iostream>


namespace vidre {

    Storage::Storage(const Config& config, const std::string& header_format, const std::string& output_file_name)
    {
        _config = config; 

        if (header_format == "ccv") {
            _headerFormat = HeaderFormat::CCV;
            _header = std::make_unique<CamCommandoHeader>(config);
        }

        if (!_header) {
            std::cerr << "Unknown header format: " << header_format << ", falling back to ccv\n";
            _headerFormat = HeaderFormat::CCV;
            _header = std::make_unique<CamCommandoHeader>(config);
        }

        _outputFileName = output_file_name;
        if (!output_file.open(_outputFileName)) {
             std::cerr << "Failed to open file!\n";
        }
    }     

    void Storage::setConfig(const Config& config) {
        _config = config;
    }  
    
    Config Storage::getConfig() {
        return _config;
    }

    void Storage::updateHeader(int frames) {
        _header->updateHeader(output_file, frames);
    }

    void Storage::appendFrame(const void* buffer) {
        _header->writeFrame(buffer, output_file);
    }

    void Storage::appendTail(uint32_t frame_idx, double time_stamp, int64_t computer_time_stamp) {
        CCVFrameTail tail{};
        tail.frameIndex = frame_idx;
        tail.timeStamp = time_stamp;
        tail.timeStamp /= (_config.cameraVendor == "basler") ? 1e9 : 1e3;
        tail.computerTimeStamp = computer_time_stamp;

        _header->writeTail(output_file, tail);
    }

    void Storage::initiateHeader() { // todo -> start all unified time  for all cameras
        // adding ISO format for output file
        auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        std::string timestamp = std::format("{:%Y%m%dT%H%M%S}", time); // seconds

        auto old_name = _outputFileName;
        std::filesystem::path p(_outputFileName);
        std::filesystem::path newPath =  p.parent_path() / (timestamp + "_" + p.stem().string() + p.extension().string());

        _outputFileName = newPath.string();
        try {
            output_file.close();
            std::filesystem::rename(old_name, _outputFileName);
            if (!output_file.open(_outputFileName)) {
                std::cerr << "Error: unable to open output file: " << _outputFileName << "\n";
                return;
            }
            _header->initiateHeader(output_file);
        } 
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}  // namespace vidre
