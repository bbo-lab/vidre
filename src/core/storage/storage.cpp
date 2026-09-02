#include <core/storage/storage.hpp>
#include <core/runtime/shared_frame_header.hpp>
#include <core/header/ccv/ccv_format.hpp>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iostream>


namespace vidre {

    namespace {
        std::string LocalTimestamp() {
            const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm local{};
#ifdef _WIN32
            localtime_s(&local, &now);
#else
            localtime_r(&now, &local);
#endif
            char buffer[32];
            if (std::strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%S", &local) == 0) {
                return "00000000T000000";
            }
            return buffer;
        }
    }  // namespace

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
    }

    void Storage::setConfig(const Config& config) {
        _config = config;
    }

    Config Storage::getConfig() {
        return _config;
    }

    bool Storage::hasError() const {
        return _failed || output_file.hasError();
    }

    void Storage::updateHeader(int frames) {
        if (!output_file.isOpen()) {
            return;
        }
        _header->updateHeader(output_file, frames);
        output_file.close();
    }

    void Storage::appendFrame(const void* buffer, size_t payloadSize) {
        if (!output_file.isOpen()) {
            _failed = true;
            return;
        }
        _header->writeFrame(buffer, payloadSize, output_file);
    }

    void Storage::appendTail(uint32_t frame_idx, double time_stamp, int64_t computer_time_stamp) {
        if (!output_file.isOpen()) {
            _failed = true;
            return;
        }
        CCVFrameTail tail{};
        tail.frameIndex = frame_idx;
        tail.timeStamp = time_stamp;
        tail.timeStamp /= (_config.cameraVendor == "basler") ? 1e9 : 1e3;

        for (size_t i = 0; i < sizeof(int64_t) && i < sizeof(tail.padding); i++) {
            tail.padding[i] = static_cast<uint8_t>((computer_time_stamp >> (i * 8)) & 0xFF);
        }

        _header->writeTail(output_file, tail);
    }

    void Storage::initiateHeader() { // todo -> start all unified time  for all cameras
        _failed = false;
        try {
            
            const std::filesystem::path configured(_configuredFileName.empty() ? _outputFileName : _configuredFileName);
            if (_configuredFileName.empty()) {
                _configuredFileName = configured.string();
            }
            const std::filesystem::path newPath =
                configured.parent_path() / (LocalTimestamp() + "_" + configured.stem().string() + configured.extension().string());

            if (!configured.parent_path().empty()) {
                std::filesystem::create_directories(configured.parent_path());
            }

            _outputFileName = newPath.string();
            output_file.close();
            if (!output_file.open(_outputFileName)) {
                std::cerr << "Error: unable to open output file: " << _outputFileName << "\n";
                _failed = true;
                return;
            }
            _header->initiateHeader(output_file);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            _failed = true;
        }
    }
}  // namespace vidre
