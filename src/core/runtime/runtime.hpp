#pragma once

#include <thread>
#include <string>
#include <iostream>

#include <core/storage/storage.hpp>
#include <core/buffer/buffer.hpp>
#include <camera/camera.hpp>

namespace vidre {
    struct Config;

    enum class RecordingStatus {
        Stopped,
        Streaming,
        RecordingFine,
        RecordingDroppedFrames,
        RecordingError
    };

    class RuntimeInstance {
    public:
        RuntimeInstance() = default;
        RuntimeInstance(std::string configuration, std::string header_format = "ccv");
        ~RuntimeInstance() = default;
        
        void retrieveFramesWithSave();
        void showFrames();
        void run();
        void show();
        void stop();

        void setCamera(std::unique_ptr<ICamera> camera);
        void createStorage(std::string output_file_name = "some.ccv", std::string header_format = "ccv", uint64_t bits_per_pixel = 8, uint8_t packed = 0);

        bool checkCamera();

        bool isRunning();
        
        RecordingStatus getRecordingStatus();
        
        // Get camera pointer for direct access (e.g., for live preview)
        ICamera* getCamera() { return _camera.get(); }
        
        // Get camera configuration
        unsigned int getFrameWidth() { return _camera ? _camera->GetFrameWidth() : 0; }
        unsigned int getFrameHeight() { return _camera ? _camera->GetFrameHeight() : 0; }
        unsigned int getFrameBytes() { return _camera ? _camera->GetFrameBytes() : 0; }
        
        std::tuple<const uint8_t*, int, int, int> getLatestFrame() {
            if (_camera->GetVendorName() == "aptina") {
                uint32_t rgbWidth, rgbHeight, rgbBitDepth;
                unsigned char *pRGB = _camera->RGBFrame(_latestFrameData, &rgbWidth, &rgbHeight, &rgbBitDepth);
                return {pRGB, rgbWidth, rgbHeight, _currentFrame};
            }
            return {_latestFrameData, getFrameWidth(), getFrameHeight(), _currentFrame}; 
        }

        uint64_t getBitsPerPixel() {
            return _storageBitsPerPixel;
        }

        std::string getOutputFileName() {
            return _storage->getOutputFileName();
        }

        private:
        int _currentFrame = 0;
        std::thread _thread;
        std::unique_ptr<ICamera> _camera;
        std::unique_ptr<Storage> _storage;
        std::unique_ptr<Buffer> _buffer;
        bool _saveTIFF = false;
        bool _presentFrame = false;
        uint8_t* _latestFrameData = nullptr;
        RecordingStatus _recordingStatus = RecordingStatus::Stopped;

        // Stored parameters for recreating storage/buffer on subsequent runs
        std::string _storageOutputFileName;
        std::string _storageHeaderFormat;
        uint64_t _storageBitsPerPixel = 8;
        uint8_t _storagePacked = 0;
        bool _storageConfigured = false;

        void resetForNextRun();
        void recreateStorageAndBuffer();
    };
}  // namespace vidre