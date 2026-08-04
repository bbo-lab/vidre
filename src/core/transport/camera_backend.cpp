#include <core/transport/camera_backend.hpp>

#include <algorithm>

namespace vidre {

    void CameraBackend::setRuntimes(std::unordered_map<std::string, std::unique_ptr<RuntimeInstance>> runtimes)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _runtimes = std::move(runtimes);
    }

    RuntimeInstance* CameraBackend::find(const std::string& cameraName)
    {
        auto it = _runtimes.find(cameraName);
        if (it == _runtimes.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    std::vector<transport::CameraInfo> CameraBackend::listCameras()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<transport::CameraInfo> result;
        for (const auto& [name, runtime] : _runtimes) {
            if (runtime && runtime->checkCamera()) {
                result.push_back(buildCameraInfo(name, runtime.get()));
            }
        }
        return result;
    }

    transport::CameraInfo CameraBackend::buildCameraInfo(const std::string& name, RuntimeInstance* runtime)
    {
        transport::CameraInfo info;
        info.cameraName = name;
        ICamera* camera = runtime->getCamera();
        if (camera) {
            info.serialNumber = camera->GetSerialNumber();
            info.vendorName = camera->GetVendorName();
            info.exposure = camera->GetExposure();
            info.gain = camera->GetGain();
            info.bufferSize = camera->GetBufferSize();
            info.maxNumBuffers = camera->GetMaxNumBuffer();
            info.frameRate = camera->GetFrameRate();
        }
        info.width = runtime->getFrameWidth();
        info.height = runtime->getFrameHeight();
        info.frameBytes = runtime->getFrameBytes();
        info.bitsPerPixel = runtime->getBitsPerPixel();
        return info;
    }

    transport::CameraInfo CameraBackend::cameraInfo(const std::string& cameraName)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr || !runtime->checkCamera()) {
            return {};
        }
        return buildCameraInfo(cameraName, runtime);
    }

    transport::StatusResponse CameraBackend::start(const std::string& cameraName)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr) {
            return {false, "Camera not found: " + cameraName};
        }
        if (runtime->isRunning()) {
            return {false, "Camera already running: " + cameraName};
        }
        runtime->run();
        return {true, "Started: " + cameraName};
    }

    transport::StatusResponse CameraBackend::stop(const std::string& cameraName)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr) {
            return {false, "Camera not found: " + cameraName};
        }
        runtime->stop();
        return {true, "Stopped: " + cameraName};
    }

    transport::StatusResponse CameraBackend::show(const std::string& cameraName)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr) {
            return {false, "Camera not found: " + cameraName};
        }
        if (runtime->isRunning()) {
            return {false, "Camera already running: " + cameraName};
        }
        runtime->show();
        return {true, "Streaming: " + cameraName};
    }

    transport::StatusResponse CameraBackend::stopShow(const std::string& cameraName)
    {
        return stop(cameraName);
    }

    transport::StatusResponse CameraBackend::setExposure(const std::string& cameraName, double exposure)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr || runtime->getCamera() == nullptr) {
            return {false, "Camera not found: " + cameraName};
        }
        runtime->getCamera()->SetExposure(exposure);
        return {true, "Exposure set: " + cameraName};
    }

    transport::StatusResponse CameraBackend::setGain(const std::string& cameraName, double gain)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr || runtime->getCamera() == nullptr) {
            return {false, "Camera not found: " + cameraName};
        }
        runtime->getCamera()->SetGain(gain);
        return {true, "Gain set: " + cameraName};
    }

    transport::StatusResponse CameraBackend::setPreview(const std::string& cameraName, bool enabled)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr) {
            return {false, "Camera not found: " + cameraName};
        }
        runtime->setPreviewRequested(enabled);
        return {true, "Preview " + std::string(enabled ? "enabled" : "disabled") + ": " + cameraName};
    }

    transport::CameraStatus CameraBackend::status(const std::string& cameraName)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        transport::CameraStatus result;
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr) {
            return result;
        }
        result.recordingStatus = static_cast<int32_t>(runtime->getRecordingStatus());
        ICamera* camera = runtime->getCamera();
        result.armed = camera != nullptr && camera->IsArmed();
        result.running = runtime->isRunning();
        result.error = camera != nullptr && camera->HadError();
        return result;
    }

    transport::RecordingStats CameraBackend::stats(const std::string& cameraName)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        transport::RecordingStats result;
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr) {
            return result;
        }
        result.savedFrames = runtime->getSavedFramesNumber();
        result.droppedFrames = runtime->getDroppedFrames();
        return result;
    }

    bool CameraBackend::latestFrame(const std::string& cameraName, FrameEnvelope& envelope)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        RuntimeInstance* runtime = find(cameraName);
        if (runtime == nullptr || !runtime->isRunning() || !runtime->previewRequested()) {
            return false;
        }

        const auto [data, width, height, frameId] = runtime->getLatestFrame();
        if (data == nullptr || width == 0 || height == 0) {
            return false;
        }

        ICamera* camera = runtime->getCamera();
        const bool aptina = camera != nullptr && camera->GetVendorName() == "aptina";
        const uint32_t channels = aptina ? 3U : 1U;
        const size_t payloadSize = aptina ? static_cast<size_t>(width) * height * channels : runtime->getFrameBytes();

        envelope.cameraName = cameraName;
        envelope.serialNumber = camera ? camera->GetSerialNumber() : "";
        envelope.vendorName = camera ? camera->GetVendorName() : "";
        envelope.width = width;
        envelope.height = height;
        envelope.bitsPerPixel = runtime->getBitsPerPixel();
        envelope.frameId = static_cast<uint64_t>(frameId);
        envelope.payload.assign(data, data + payloadSize);
        return true;
    }

    void CameraBackend::stopAll()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& [name, runtime] : _runtimes) {
            if (runtime && runtime->isRunning()) {
                runtime->stop();
            }
        }
    }

}  // namespace vidre
