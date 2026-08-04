#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <memory>

#include <core/runtime/iruntime_instance.hpp>
#include <core/storage/storage.hpp>
#include <core/buffer/buffer.hpp>
#include <camera/camera.hpp>
#include <core/transport/transport.hpp>

namespace vidre {
    struct Config;

    class RuntimeInstance : public IRuntimeInstance {
    public:
        RuntimeInstance() = default;
        RuntimeInstance(std::string configuration, std::string header_format = "ccv");
        ~RuntimeInstance() = default;
        
        void retrieveFramesWithSave();
        void showFrames();
        void run() override;
        void show() override;
        void stop() override;

        void setCamera(std::unique_ptr<ICamera> camera);
        void setCameraName(std::string cameraName);
        void setTransportMode(TransportMode transportMode);
        void setStorageEndpoint(std::string storageEndpoint);
        void setFrameSink(std::unique_ptr<IFrameSink> frameSink);
        void createStorage(std::string output_file_name = "some.ccv", std::string header_format = "ccv", uint64_t bits_per_pixel = 8, uint8_t packed = 0);

        bool checkCamera() override;

        bool isRunning() override;
        
        RecordingStatus getRecordingStatus() override;
        
        ICamera* getCamera() override { return _camera.get(); }
        
        unsigned int getFrameWidth() override { return _camera ? _camera->GetFrameWidth() : 0; }
        unsigned int getFrameHeight() override { return _camera ? _camera->GetFrameHeight() : 0; }
        unsigned int getFrameBytes() override { return _camera ? _camera->GetFrameBytes() : 0; }
        
        std::tuple<const uint8_t*, int, int, int> getLatestFrame() override {
            if (_latestFrameData == nullptr) {
                return {nullptr, 0, 0, 0};
            }
            if (_camera->GetVendorName() == "aptina") {
                uint32_t rgbWidth, rgbHeight, rgbBitDepth;
                unsigned char *pRGB = _camera->RGBFrame(_latestFrameData, &rgbWidth, &rgbHeight, &rgbBitDepth);
                return {pRGB, rgbWidth, rgbHeight, _currentFrame};
            }
            return {_latestFrameData, getFrameWidth(), getFrameHeight(), _currentFrame}; 
        }

        uint64_t getBitsPerPixel() override {
            return _storageBitsPerPixel;
        }

        int getDroppedFrames() override;
        int getSavedFramesNumber() override;
        std::string getOutputFileName() override;

        void setPreviewRequested(bool requested) override {
            _previewRequested.store(requested, std::memory_order_relaxed);
        }

        bool previewRequested() const {
            return _previewRequested.load(std::memory_order_relaxed);
        }

        private:
        int _currentFrame = 0;
        int _droppedFrames = 0;
        std::thread _thread;
        std::unique_ptr<ICamera> _camera;
        std::unique_ptr<Storage> _storage;
        std::unique_ptr<Buffer> _buffer;
        std::unique_ptr<IFrameSink> _frameSink;
        std::string _cameraName;
        std::string _storageEndpoint;
        TransportMode _transportMode = TransportMode::LocalStorage;
        bool _saveTIFF = false;
        bool _presentFrame = false;
        uint8_t* _latestFrameData = nullptr;
        RecordingStatus _recordingStatus = RecordingStatus::Stopped;
        std::atomic<bool> _previewRequested{false};
        bool _runtimeConfigured = false;
        bool _localStorageEnabled = false;

        // Stored parameters for recreating storage/buffer on subsequent runs
        std::string _storageOutputFileName;
        std::string _storageHeaderFormat;
        uint64_t _storageBitsPerPixel = 8;
        uint8_t _storagePacked = 0;

        void resetForNextRun();
        void recreateStorageAndBuffer();
        Config buildCameraConfig(uint64_t bits_per_pixel, uint8_t packed) const;
    };
}  // namespace vidre
