#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

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

        void setPreviewRequested(bool requested) {
            _previewRequested.store(requested, std::memory_order_relaxed);
        }

        RecordingStatus getRecordingStatus() {
            return _recordingStatus.load(std::memory_order_relaxed);
        }

        ICamera* getCamera() { return _camera.get(); }

        unsigned int getFrameWidth() { return _frameWidth ? _frameWidth : (_camera ? _camera->GetFrameWidth() : 0); }
        unsigned int getFrameHeight() { return _frameHeight ? _frameHeight : (_camera ? _camera->GetFrameHeight() : 0); }
        unsigned int getFrameBytes() { return _frameBytes ? _frameBytes : (_camera ? _camera->GetFrameBytes() : 0); }

        std::tuple<const uint8_t*, int, int, int> getLatestFrame() {
            if (!_camera) {
                return {nullptr, 0, 0, 0};
            }
            auto* data = _latestFrameData.load(std::memory_order_relaxed);
            auto frame = _currentFrame.load(std::memory_order_relaxed);
            if (!data) {
                return {nullptr, 0, 0, 0};
            }
            if (_isAptinaVendor) {
                uint32_t rgbWidth, rgbHeight, rgbBitDepth;
                unsigned char *pRGB = _camera->RGBFrame(data, &rgbWidth, &rgbHeight, &rgbBitDepth);
                return {pRGB, rgbWidth, rgbHeight, frame};
            }
            return {data, getFrameWidth(), getFrameHeight(), frame};
        }

        uint64_t getBitsPerPixel() {
            return _storageBitsPerPixel;
        }

        int getDroppedFrames() {
            return _buffer ? _buffer->getDroppedFrames() : 0;
        }

        int getSavedFramesNumber() {
            return _buffer ? _buffer->getSavedFramesNumber() : 0;
        }

        std::string getOutputFileName() {
            return _storage ? _storage->getOutputFileName() : "";
        }

        private:
        std::atomic<int> _currentFrame{0};
        std::thread _thread;
        std::unique_ptr<ICamera> _camera;
        std::unique_ptr<Storage> _storage;
        std::unique_ptr<Buffer> _buffer;
        std::atomic<uint8_t*> _latestFrameData{nullptr};
        std::atomic<RecordingStatus> _recordingStatus{RecordingStatus::Stopped};
        std::atomic<bool> _previewRequested{false};

        std::string _storageOutputFileName;
        std::string _storageHeaderFormat;
        uint64_t _storageBitsPerPixel = 8;
        uint8_t _storagePacked = 0;
        bool _storageConfigured = false;

        unsigned int _frameWidth = 0;
        unsigned int _frameHeight = 0;
        unsigned int _frameBytes = 0;
        unsigned int _slotBytes = 0;
        bool _isAptinaVendor = false;

        std::vector<uint8_t> _previewBuffer;
        uint8_t* _previewData = nullptr;

        void processFrame(void* result, const SharedFrameHeader* header);
        void consumeFrames(const std::function<void(void*, const SharedFrameHeader*)>& onFrame);
        void publishFrame(void* result);
        void resetForNextRun();
        void recreateStorageAndBuffer();
    };
}  // namespace vidre
