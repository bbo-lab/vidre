#pragma once

#include <mutex>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <core/transport/wire_protocol.hpp>

namespace vidre {

    class ControlClient {
    public:
        explicit ControlClient(std::string endpoint);
        ~ControlClient();

        bool connect();
        void disconnect();
        bool connected() const;

        bool listCameras(std::vector<transport::CameraInfo>& cameras);
        transport::CameraInfo cameraInfo(const std::string& cameraName);
        bool startRecording(const std::string& cameraName);
        bool stopRecording(const std::string& cameraName);
        bool startStreaming(const std::string& cameraName);
        bool stopStreaming(const std::string& cameraName);
        bool setExposure(const std::string& cameraName, double exposure);
        bool setGain(const std::string& cameraName, double gain);
        transport::CameraStatus status(const std::string& cameraName);
        transport::RecordingStats stats(const std::string& cameraName);
        bool setPreview(const std::string& cameraName, bool enabled);

    private:
        bool sendRequest(const transport::ControlRequest& request, transport::ControlResponse& response);
        bool connectLocked();

        std::string _endpoint;
        boost::asio::io_context _io;
        std::unique_ptr<boost::asio::ip::tcp::socket> _socket;
        mutable std::mutex _mutex;
    };

}  // namespace vidre
