#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <core/runtime/runtime.hpp>
#include <core/transport/wire_protocol.hpp>

namespace vidre {

    class CameraBackend {
    public:
        CameraBackend() = default;
        ~CameraBackend() = default;

        void setRuntimes(std::unordered_map<std::string, std::unique_ptr<RuntimeInstance>> runtimes);

        std::vector<transport::CameraInfo> listCameras();
        transport::CameraInfo cameraInfo(const std::string& cameraName);
        transport::StatusResponse start(const std::string& cameraName);
        transport::StatusResponse stop(const std::string& cameraName);
        transport::StatusResponse show(const std::string& cameraName);
        transport::StatusResponse stopShow(const std::string& cameraName);
        transport::StatusResponse setExposure(const std::string& cameraName, double exposure);
        transport::StatusResponse setGain(const std::string& cameraName, double gain);
        transport::StatusResponse setPreview(const std::string& cameraName, bool enabled);
        transport::CameraStatus status(const std::string& cameraName);
        transport::RecordingStats stats(const std::string& cameraName);
        bool latestFrame(const std::string& cameraName, FrameEnvelope& envelope);

        void stopAll();

    private:
        transport::CameraInfo buildCameraInfo(const std::string& name, RuntimeInstance* runtime);
        RuntimeInstance* find(const std::string& cameraName);

        std::mutex _mutex;
        std::unordered_map<std::string, std::unique_ptr<RuntimeInstance>> _runtimes;
    };

}  // namespace vidre
