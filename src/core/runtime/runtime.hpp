#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

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
        ~RuntimeInstance() override;

        RuntimeInstance(const RuntimeInstance&) = delete;
        RuntimeInstance& operator=(const RuntimeInstance&) = delete;

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

        void setPreviewRequested(bool requested) override {
            _previewRequested.store(requested, std::memory_order_relaxed);
        }

        RecordingStatus getRecordingStatus() override {
            return _recordingStatus.load(std::memory_order_relaxed);
        }

        ICamera* getCamera() override { return _camera.get(); }

        unsigned int getFrameWidth() override { return _frameWidth ? _frameWidth : (_camera ? _camera->GetFrameWidth() : 0); }
        unsigned int getFrameHeight() override { return _frameHeight ? _frameHeight : (_camera ? _camera->GetFrameHeight() : 0); }
        unsigned int getFrameBytes() override { return _frameBytes ? _frameBytes : (_camera ? _camera->GetFrameBytes() : 0); }

        std::tuple<const uint8_t*, int, int, int> getLatestFrame() override;

        uint64_t getBitsPerPixel() override {
            return _storageBitsPerPixel;
        }

        int getDroppedFrames() override;
        int getSavedFramesNumber() override;
        std::string getOutputFileName() override;
        int64_t getSentFrames() override;

        bool previewRequested() const {
            return _previewRequested.load(std::memory_order_relaxed);
        }

    private:
        std::atomic<int> _currentFrame{0};
        std::thread _thread;
        std::atomic<bool> _consumerActive{false};
        std::unique_ptr<ICamera> _camera;
        std::unique_ptr<Storage> _storage;
        std::unique_ptr<Buffer> _buffer;
        std::unique_ptr<IFrameSink> _frameSink;
        std::string _cameraName;
        std::string _storageEndpoint;
        TransportMode _transportMode = TransportMode::LocalStorage;
        std::atomic<RecordingStatus> _recordingStatus{RecordingStatus::Stopped};
        std::atomic<bool> _previewRequested{false};
        bool _runtimeConfigured = false;
        bool _localStorageEnabled = false;

        std::string _storageOutputFileName;
        std::string _storageHeaderFormat;
        uint64_t _storageBitsPerPixel = 8;
        uint8_t _storagePacked = 0;

        unsigned int _frameWidth = 0;
        unsigned int _frameHeight = 0;
        unsigned int _frameBytes = 0;
        unsigned int _slotBytes = 0;
        bool _isAptinaVendor = false;


        std::array<std::vector<uint8_t>, 3> _previewSlots;
        int _previewWrite = 0;
        int _previewRead = 1;
        std::atomic<int> _previewLatest{2};
        std::atomic<bool> _previewHasNew{false};
        std::atomic<bool> _previewValid{false};

        std::atomic<int> _lastSavedFrames{0};
        std::atomic<int> _lastDroppedFrames{0};
        std::atomic<int64_t> _lastSentFrames{0};
        std::mutex _lastRunMutex;
        std::string _lastOutputFileName;

        void consumeFrames(const std::function<void(void*, const SharedFrameHeader*)>& onFrame);
        bool waitForCameraStart();
        void publishFrame(const void* result);
        void snapshotLastRun();
        void resetForNextRun();
        void recreateStorageAndBuffer();
        Config buildCameraConfig(uint64_t bits_per_pixel, uint8_t packed) const;
    };
}  // namespace vidre
