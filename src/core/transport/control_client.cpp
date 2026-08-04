#include <core/transport/control_client.hpp>
#include <core/transport/wire_protocol.hpp>

#include <iostream>

namespace vidre {

    ControlClient::ControlClient(std::string endpoint)
        : _endpoint(std::move(endpoint))
    {
    }

    ControlClient::~ControlClient() {
        disconnect();
    }

    bool ControlClient::connect()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return connectLocked();
    }

    void ControlClient::disconnect()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_socket) {
            try {
                _socket->close();
            } catch (...) {
            }
            _socket.reset();
        }
    }

    bool ControlClient::connected() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _socket != nullptr && _socket->is_open();
    }

    bool ControlClient::sendRequest(const transport::ControlRequest& request, transport::ControlResponse& response)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_socket || !_socket->is_open()) {
            if (!connectLocked()) {
                return false;
            }
        }
        const auto payload = transport::EncodeControlRequest(request);
        if (!transport::WriteFramed(*_socket, payload)) {
            _socket.reset();
            return false;
        }
        std::vector<uint8_t> raw;
        if (!transport::ReadFramed(*_socket, raw)) {
            _socket.reset();
            return false;
        }
        return transport::DecodeControlResponse(raw, response);
    }

    bool ControlClient::connectLocked()
    {
        try {
            const auto [host, port] = transport::SplitEndpoint(_endpoint);
            boost::asio::ip::tcp::resolver resolver(_io);
            const auto endpoints = resolver.resolve(host, std::to_string(port));
            _socket = std::make_unique<boost::asio::ip::tcp::socket>(_io);
            boost::asio::connect(*_socket, endpoints);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Control client connect failed (" << _endpoint << "): " << e.what() << "\n";
            _socket.reset();
            return false;
        }
    }

    bool ControlClient::listCameras(std::vector<transport::CameraInfo>& cameras)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::ListCameras;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::CameraList) {
            return false;
        }
        cameras = std::move(response.cameras);
        return true;
    }

    transport::CameraInfo ControlClient::cameraInfo(const std::string& cameraName)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::GetCameraInfo;
        request.cameraName = cameraName;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::CameraInfo) {
            return {};
        }
        return response.cameraInfo;
    }

    bool ControlClient::startRecording(const std::string& cameraName)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::StartRecording;
        request.cameraName = cameraName;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::Status) {
            return false;
        }
        return response.status.ok;
    }

    bool ControlClient::stopRecording(const std::string& cameraName)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::StopRecording;
        request.cameraName = cameraName;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::Status) {
            return false;
        }
        return response.status.ok;
    }

    bool ControlClient::startStreaming(const std::string& cameraName)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::StartStreaming;
        request.cameraName = cameraName;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::Status) {
            return false;
        }
        return response.status.ok;
    }

    bool ControlClient::stopStreaming(const std::string& cameraName)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::StopStreaming;
        request.cameraName = cameraName;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::Status) {
            return false;
        }
        return response.status.ok;
    }

    bool ControlClient::setExposure(const std::string& cameraName, double exposure)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::SetExposure;
        request.cameraName = cameraName;
        request.value = exposure;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::Status) {
            return false;
        }
        return response.status.ok;
    }

    bool ControlClient::setGain(const std::string& cameraName, double gain)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::SetGain;
        request.cameraName = cameraName;
        request.value = gain;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::Status) {
            return false;
        }
        return response.status.ok;
    }

    transport::CameraStatus ControlClient::status(const std::string& cameraName)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::GetStatus;
        request.cameraName = cameraName;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::CameraStatus) {
            return {};
        }
        return response.cameraStatus;
    }

    transport::RecordingStats ControlClient::stats(const std::string& cameraName)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::GetStats;
        request.cameraName = cameraName;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::Stats) {
            return {};
        }
        return response.stats;
    }

    bool ControlClient::setPreview(const std::string& cameraName, bool enabled)
    {
        transport::ControlRequest request;
        request.type = transport::ControlRequestType::SetPreview;
        request.cameraName = cameraName;
        request.flag = enabled;
        transport::ControlResponse response;
        if (!sendRequest(request, response) || response.type != transport::ControlResponseType::Status) {
            return false;
        }
        return response.status.ok;
    }

}  // namespace vidre
