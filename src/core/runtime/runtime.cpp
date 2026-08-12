#include <core/runtime/runtime.hpp>
#include <core/runtime/shared_frame_header.hpp>

#include <chrono>
#include <cstring>
#include <iostream>
#include <algorithm>


namespace vidre
{
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

        // Store params for recreating on subsequent runs
        _storageOutputFileName = output_file_name;
        _storageHeaderFormat = header_format;
        _storageBitsPerPixel = bits_per_pixel;
        _storagePacked = packed;
        _runtimeConfigured = true;
        _localStorageEnabled = !_storageOutputFileName.empty() && _transportMode != TransportMode::RemoteStorage;
        
        const auto cameraConfig = buildCameraConfig(bits_per_pixel, packed);

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
            _frameSink.reset();
        }

        _camera->SetSharedMemory(_buffer->getSharedMemory(), _buffer->getSharedMemoryPtrs(), cameraConfig.serialNumber);
    }

    void RuntimeInstance::publishFrame(void* result) {
        if (!_previewRequested.load(std::memory_order_relaxed) || _frameBytes == 0) {
            _latestFrameData = nullptr;
            return;
        }

        const size_t copyBytes = std::min<size_t>(_frameBytes, _slotBytes);
        if (_previewBuffer.size() < copyBytes) {
            _previewBuffer.resize(copyBytes);
            _previewData = _previewBuffer.data();
        }

        std::memcpy(_previewData, result, copyBytes);
        _latestFrameData = _previewData;
    }

    void RuntimeInstance::consumeFrames(const std::function<void(void*, const SharedFrameHeader*)>& onFrame) {
        while (_camera->IsRunning()) {
            const auto [result, header] = _buffer->pop();
            if (result == nullptr) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            onFrame(result, header);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));

        // Walk the whole ring once to drain every frame still physically present.
        const int numSlots = _buffer->numSlots();
        for (int i = 0; i < numSlots; ++i) {
            const auto [result, header] = _buffer->pop();
            if (result == nullptr) {
                continue;
            }
            onFrame(result, header);
        }
    }

    void RuntimeInstance::retrieveFramesWithSave() {
        while (!_camera->IsRunning()) {
            std::this_thread::yield();
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

        std::cout << "SAVED : " << _buffer->getSavedFramesNumber() << std::endl;
    }

    void RuntimeInstance::showFrames() {
        while (!_camera->IsRunning()) {
            std::this_thread::yield();
        }

        _recordingStatus = RecordingStatus::Streaming;

        consumeFrames([this](void* result, const SharedFrameHeader*) {
            publishFrame(result);
            _currentFrame++;
        });

        if (_camera->HadError()) {
            _recordingStatus = RecordingStatus::RecordingError;
        }
    }

    void RuntimeInstance::run() {
        if (_camera == nullptr) {
            return;
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
        
        _thread = std::thread(&RuntimeInstance::retrieveFramesWithSave, this);
    }
    void RuntimeInstance::show() {
        if (_camera == nullptr) {
            return;
        }

        if (!_buffer) {
            recreateStorageAndBuffer();
        }

        if (_buffer == nullptr) {
            return;
        }

        _camera->Arm();
        
        _camera->Start();
        
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
        
        _camera->Disarm();
        if (_recordingStatus != RecordingStatus::RecordingError) {
            _recordingStatus = RecordingStatus::Stopped;
        }

        resetForNextRun();
    }

    void RuntimeInstance::resetForNextRun() {
        _currentFrame = 0;
        _latestFrameData = nullptr;

        // Release old storage and buffer so they can be recreated on the next run
        _storage.reset();
        _buffer.reset();
    }

    void RuntimeInstance::recreateStorageAndBuffer() {
        if (!_runtimeConfigured || !_camera) {
            return;
        }

        // Recreate storage and buffer fresh for the new run
        createStorage(_localStorageEnabled ? _storageOutputFileName : std::string(), _storageHeaderFormat, _storageBitsPerPixel, _storagePacked);
    }

    bool RuntimeInstance::checkCamera() {
        return _camera != nullptr;
    }
    bool RuntimeInstance::isRunning() {
        return _camera && _camera->IsRunning();
    }

    RecordingStatus RuntimeInstance::getRecordingStatus() {
        return _recordingStatus.load(std::memory_order_relaxed);
    }

    int RuntimeInstance::getDroppedFrames() {
        return _buffer ? _buffer->getDroppedFrames() : 0;
    }

    int RuntimeInstance::getSavedFramesNumber() {
        return _buffer ? _buffer->getSavedFramesNumber() : 0;
    }

    std::string RuntimeInstance::getOutputFileName() {
        return _storage ? _storage->getOutputFileName() : "";
    }

}
