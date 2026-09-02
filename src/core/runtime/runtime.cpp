#include <core/runtime/runtime.hpp>
#include <core/runtime/shared_frame_header.hpp>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>


namespace vidre
{
    namespace {
        constexpr auto kCameraStartTimeout = std::chrono::seconds(5);
        constexpr auto kIdlePollInterval = std::chrono::milliseconds(1);
    }

    RuntimeInstance::~RuntimeInstance() {
        try {
            stop();
        } catch (...) {
            
        }
    }

    void RuntimeInstance::setCamera(std::unique_ptr<ICamera> camera) {
        if (_camera != nullptr) {
            std::cout << "Camera is set already \n";
            return;
        }

        if (camera == nullptr) {
            std::cout << "Recieved nullptr camera \n";
            return;
        }

        _camera = std::move(camera);

        _frameWidth = _camera->GetFrameWidth();
        _frameHeight = _camera->GetFrameHeight();
        _frameBytes = _camera->GetFrameBytes();
        _slotBytes = _camera->GetBufferSize();
        _isAptinaVendor = (_camera->GetVendorName() == "aptina");

        if (_slotBytes < _frameBytes) {
            std::cerr << "WARNING: camera '" << _cameraName << "' reports a buffer slot (" << _slotBytes
                      << " bytes) smaller than its frame (" << _frameBytes << " bytes); frames will be truncated\n";
            _frameBytes = _slotBytes;
        }
    }

    void RuntimeInstance::setCameraName(std::string cameraName) {
        _cameraName = std::move(cameraName);
    }

    void RuntimeInstance::setTransportMode(TransportMode transportMode) {
        _transportMode = transportMode;
    }

    void RuntimeInstance::setStorageEndpoint(std::string storageEndpoint) {
        _storageEndpoint = std::move(storageEndpoint);
    }

    void RuntimeInstance::setFrameSink(std::unique_ptr<IFrameSink> frameSink) {
        _frameSink = std::move(frameSink);
    }

    Config RuntimeInstance::buildCameraConfig(uint64_t bits_per_pixel, uint8_t packed) const {
        Config cameraConfig{};
        cameraConfig.paddingX = _camera->GetFrameOffsetX();
        cameraConfig.width = _camera->GetFrameWidth();
        cameraConfig.height = _camera->GetFrameHeight();
        cameraConfig.maxNumBuffer = _camera->GetMaxNumBuffer();
        cameraConfig.frameRate = _camera->GetFrameRate();
        cameraConfig.serialNumber = _camera->GetSerialNumber();
        cameraConfig.frameBytesActual = _camera->GetFrameBytes();
        cameraConfig.frameBytesBuffer = _camera->GetBufferSize();
        cameraConfig.cameraVendor = _camera->GetVendorName();
        cameraConfig.bitsPerPixel = bits_per_pixel;
        cameraConfig.packed = packed;
        return cameraConfig;
    }

    void RuntimeInstance::createStorage(std::string output_file_name, std::string header_format, uint64_t bits_per_pixel, uint8_t packed) {
        if (_camera == nullptr) {
            std::cout << "Camera is not set \n";
            return;
        }

        if (isRunning()) {
            std::cerr << "createStorage ignored: camera '" << _cameraName << "' is running\n";
            return;
        }

        // Store params for recreating on subsequent runs
        _storageOutputFileName = output_file_name;
        _storageHeaderFormat = header_format;
        _storageBitsPerPixel = bits_per_pixel;
        _storagePacked = packed;
        _runtimeConfigured = true;
        _localStorageEnabled = !_storageOutputFileName.empty() && _transportMode != TransportMode::RemoteStorage;

        const auto cameraConfig = buildCameraConfig(bits_per_pixel, packed);

        _frameSink.reset();
        _storage.reset();
        _buffer.reset();

        if (_localStorageEnabled) {
            _storage = std::make_unique<vidre::Storage>(cameraConfig, header_format, output_file_name);
        }

        _buffer = std::make_unique<vidre::Buffer>(cameraConfig);

        if (_transportMode == TransportMode::RemoteStorage && !_storageEndpoint.empty()) {
            SessionSpec sessionSpec;
            sessionSpec.cameraName = _cameraName;
            sessionSpec.serialNumber = _camera->GetSerialNumber();
            sessionSpec.vendorName = _camera->GetVendorName();
            sessionSpec.width = cameraConfig.width;
            sessionSpec.height = cameraConfig.height;
            sessionSpec.bitsPerPixel = bits_per_pixel;
            sessionSpec.packed = packed != 0;
            sessionSpec.frameBytesActual = cameraConfig.frameBytesActual;
            sessionSpec.frameBytesBuffer = cameraConfig.frameBytesBuffer;
            sessionSpec.maxNumBuffer = cameraConfig.maxNumBuffer;
            sessionSpec.frameRate = cameraConfig.frameRate;
            sessionSpec.outputFile = output_file_name;
            sessionSpec.headerFormat = header_format;

            FrameEnvelope baseEnvelope;
            baseEnvelope.cameraName = _cameraName;
            baseEnvelope.serialNumber = _camera->GetSerialNumber();
            baseEnvelope.vendorName = _camera->GetVendorName();
            baseEnvelope.width = cameraConfig.width;
            baseEnvelope.height = cameraConfig.height;
            baseEnvelope.bitsPerPixel = bits_per_pixel;
            _frameSink = std::make_unique<RemoteStorageSink>(_storageEndpoint, std::move(baseEnvelope), std::move(sessionSpec));
        } else if (_localStorageEnabled && _storage) {
            _frameSink = std::make_unique<LocalStorageSink>(*_storage);
        } else {
            if (_transportMode == TransportMode::RemoteStorage) {
                std::cerr << "WARNING: Remote storage mode but no storage endpoint set for camera '"
                          << _cameraName << "'; frames will be discarded\n";
            } else if (!_localStorageEnabled) {
                std::cerr << "WARNING: No output file configured for camera '"
                          << _cameraName << "'; frames will not be saved\n";
            }
        }

        _camera->SetSharedMemory(_buffer->getSharedMemory(), _buffer->getSharedMemoryPtrs(), cameraConfig.serialNumber);
    }

    void RuntimeInstance::publishFrame(const void* result) {
        if (!_previewRequested.load(std::memory_order_relaxed) || _frameBytes == 0) {
            _previewValid.store(false, std::memory_order_release);
            return;
        }

        auto& slot = _previewSlots[static_cast<size_t>(_previewWrite)];
        if (slot.size() != _frameBytes) {
            slot.assign(_frameBytes, 0);
        }
        std::memcpy(slot.data(), result, _frameBytes);

        // Hand the finished slot over and take back whichever slot the reader is not using.
        _previewWrite = _previewLatest.exchange(_previewWrite, std::memory_order_acq_rel);
        _previewHasNew.store(true, std::memory_order_release);
        _previewValid.store(true, std::memory_order_release);
    }

    std::tuple<const uint8_t*, int, int, int> RuntimeInstance::getLatestFrame() {
        if (!_camera || !_previewValid.load(std::memory_order_acquire)) {
            return {nullptr, 0, 0, 0};
        }
        if (_previewHasNew.exchange(false, std::memory_order_acq_rel)) {
            _previewRead = _previewLatest.exchange(_previewRead, std::memory_order_acq_rel);
        }
        auto& slot = _previewSlots[static_cast<size_t>(_previewRead)];
        if (slot.empty()) {
            return {nullptr, 0, 0, 0};
        }
        const int frame = _currentFrame.load(std::memory_order_relaxed);
        if (_isAptinaVendor) {
            uint32_t rgbWidth = 0, rgbHeight = 0, rgbBitDepth = 0;
            unsigned char* pRGB = _camera->RGBFrame(slot.data(), &rgbWidth, &rgbHeight, &rgbBitDepth);
            return {pRGB, static_cast<int>(rgbWidth), static_cast<int>(rgbHeight), frame};
        }
        return {slot.data(), static_cast<int>(getFrameWidth()), static_cast<int>(getFrameHeight()), frame};
    }

    bool RuntimeInstance::waitForCameraStart() {
        const auto deadline = std::chrono::steady_clock::now() + kCameraStartTimeout;
        while (!_camera->IsRunning()) {
            if (_camera->HadError() || std::chrono::steady_clock::now() > deadline) {
                std::cerr << "ERROR: camera '" << _cameraName << "' did not start\n";
                return false;
            }
            std::this_thread::sleep_for(kIdlePollInterval);
        }
        return true;
    }

    void RuntimeInstance::consumeFrames(const std::function<void(void*, const SharedFrameHeader*)>& onFrame) {
        while (_camera->IsRunning()) {
            const auto [result, header] = _buffer->pop();
            if (result == nullptr) {
                std::this_thread::sleep_for(kIdlePollInterval);
                continue;
            }
            onFrame(result, header);
        }

        const int numSlots = _buffer->numSlots();
        for (int i = 0; i < numSlots; ++i) {
            const auto [result, header] = _buffer->pop();
            if (result == nullptr) {
                break;
            }
            onFrame(result, header);
        }
    }

    void RuntimeInstance::retrieveFramesWithSave() {
        if (!waitForCameraStart()) {
            _recordingStatus = RecordingStatus::RecordingError;
            if (_frameSink) {
                _frameSink->finish(0);
            }
            _consumerActive.store(false, std::memory_order_release);
            return;
        }

        _recordingStatus = RecordingStatus::RecordingFine;

        consumeFrames([this](void* result, const SharedFrameHeader* header) {
            publishFrame(result);
            _currentFrame++;

            if (_frameSink) {
                _frameSink->publishFrame(*header, static_cast<const uint8_t*>(result), _frameBytes);
            }
        });

        if (_camera->HadError()) {
            _recordingStatus = RecordingStatus::RecordingError;
        } else if (_buffer->getDroppedFrames() > 0) {
            _recordingStatus = RecordingStatus::RecordingDroppedFrames;
        }

        if (_frameSink) {
            _frameSink->finish(_buffer->getSavedFramesNumber());
        }

        std::cout << "SAVED : " << _buffer->getSavedFramesNumber();
        if (_buffer->getDroppedFrames() > 0) {
            std::cout << "  DROPPED : " << _buffer->getDroppedFrames();
        }
        std::cout << std::endl;
        _consumerActive.store(false, std::memory_order_release);
    }

    void RuntimeInstance::showFrames() {
        if (!waitForCameraStart()) {
            _recordingStatus = RecordingStatus::RecordingError;
            _consumerActive.store(false, std::memory_order_release);
            return;
        }

        _recordingStatus = RecordingStatus::Streaming;

        consumeFrames([this](void* result, const SharedFrameHeader*) {
            publishFrame(result);
            _currentFrame++;
        });

        if (_camera->HadError()) {
            _recordingStatus = RecordingStatus::RecordingError;
        }
        _consumerActive.store(false, std::memory_order_release);
    }

    void RuntimeInstance::run() {
        if (_camera == nullptr) {
            return;
        }

        if (_thread.joinable()) {
            if (_consumerActive.load(std::memory_order_acquire)) {
                return;  // already recording/streaming
            }
            stop();
        }

        // Recreate storage and buffer if they were cleaned up from a previous run
        if (!_buffer) {
            recreateStorageAndBuffer();
        }

        if (_buffer == nullptr) {
            return;
        }

        _camera->Arm();

        if (_frameSink) {
            _frameSink->begin();
        }

        _camera->Start();

        _consumerActive.store(true, std::memory_order_release);
        _thread = std::thread(&RuntimeInstance::retrieveFramesWithSave, this);
    }

    void RuntimeInstance::show() {
        if (_camera == nullptr) {
            return;
        }

        if (_thread.joinable()) {
            if (_consumerActive.load(std::memory_order_acquire)) {
                return;
            }
            stop();
        }

        if (!_buffer) {
            recreateStorageAndBuffer();
        }

        if (_buffer == nullptr) {
            return;
        }

        _camera->Arm();

        _camera->Start();

        _consumerActive.store(true, std::memory_order_release);
        _thread = std::thread(&RuntimeInstance::showFrames, this);
    }

    void RuntimeInstance::stop() {
        if (_camera == nullptr) {
            return;
        }

        _camera->Stop();

        if (_thread.joinable()) {
            _thread.join();
        }
        _consumerActive.store(false, std::memory_order_release);

        _camera->Disarm();
        if (_recordingStatus != RecordingStatus::RecordingError) {
            _recordingStatus = RecordingStatus::Stopped;
        }

        resetForNextRun();
    }

    void RuntimeInstance::snapshotLastRun() {
        if (_buffer) {
            _lastSavedFrames.store(_buffer->getSavedFramesNumber(), std::memory_order_relaxed);
            _lastDroppedFrames.store(getDroppedFrames(), std::memory_order_relaxed);
        }
        if (auto* remoteSink = dynamic_cast<RemoteStorageSink*>(_frameSink.get())) {
            _lastSentFrames.store(remoteSink->sentFrames(), std::memory_order_relaxed);
        }
        if (_storage) {
            std::lock_guard<std::mutex> lock(_lastRunMutex);
            _lastOutputFileName = _storage->getOutputFileName();
        }
    }

    void RuntimeInstance::resetForNextRun() {
        snapshotLastRun();

        _currentFrame = 0;
        _previewValid.store(false, std::memory_order_release);

        _frameSink.reset();
        _storage.reset();
        if (_buffer && _camera) {
            _camera->SetSharedMemory(nullptr, nullptr, "");
        }
        _buffer.reset();
    }

    void RuntimeInstance::recreateStorageAndBuffer() {
        if (!_runtimeConfigured || !_camera) {
            return;
        }

        // Recreate storage and buffer fresh for the new run
        createStorage(_storageOutputFileName, _storageHeaderFormat, _storageBitsPerPixel, _storagePacked);
    }

    bool RuntimeInstance::checkCamera() {
        return _camera != nullptr;
    }

    bool RuntimeInstance::isRunning() {
        return _consumerActive.load(std::memory_order_acquire);
    }

    int RuntimeInstance::getDroppedFrames() {
        if (!_buffer) {
            return _lastDroppedFrames.load(std::memory_order_relaxed);
        }
        int dropped = _buffer->getDroppedFrames();
        if (auto* remoteSink = dynamic_cast<RemoteStorageSink*>(_frameSink.get())) {
            dropped += static_cast<int>(remoteSink->droppedFrames());
        }
        return dropped;
    }

    int RuntimeInstance::getSavedFramesNumber() {
        return _buffer ? _buffer->getSavedFramesNumber() : _lastSavedFrames.load(std::memory_order_relaxed);
    }

    std::string RuntimeInstance::getOutputFileName() {
        if (_storage) {
            return _storage->getOutputFileName();
        }
        std::lock_guard<std::mutex> lock(_lastRunMutex);
        return _lastOutputFileName;
    }

    int64_t RuntimeInstance::getSentFrames() {
        auto* remoteSink = dynamic_cast<RemoteStorageSink*>(_frameSink.get());
        return remoteSink ? remoteSink->sentFrames() : _lastSentFrames.load(std::memory_order_relaxed);
    }

}
