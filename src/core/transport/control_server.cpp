#include <core/transport/control_server.hpp>
#include <core/transport/camera_backend.hpp>
#include <core/transport/wire_protocol.hpp>

#include <iostream>

namespace vidre {

    namespace {
        void logMessage(const ControlServer::LogCallback& log, const std::string& message) {
            if (log) {
                log(message);
            } else {
                std::cout << message << std::endl;
            }
        }
    }  // namespace

    ControlServer::ControlServer(CameraBackend* backend, std::string address, LogCallback log)
        : _backend(backend)
        , _address(std::move(address))
        , _log(std::move(log))
    {
    }

    ControlServer::~ControlServer() {
        stop();
    }

    void ControlServer::start()
    {
        if (_running.load(std::memory_order_relaxed)) {
            return;
        }
        _running.store(true);
        _thread = std::thread(&ControlServer::acceptLoop, this);
    }

    void ControlServer::stop()
    {
        if (!_running.load(std::memory_order_relaxed)) {
            return;
        }
        _running.store(false);
        if (_acceptor) {
            try {
                _acceptor->close();
            } catch (...) {
            }
        }
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void ControlServer::acceptLoop()
    {
        try {
            const auto [host, port] = transport::SplitEndpoint(_address);
            boost::asio::ip::tcp::resolver resolver(_io);
            const auto endpoints = resolver.resolve(host, std::to_string(port));
            _acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(_io, *endpoints.begin());
        } catch (const std::exception& e) {
            logMessage(_log, std::string("Control server: failed to bind ") + _address + ": " + e.what());
            _running.store(false);
            return;
        }

        logMessage(_log, "Control server listening on " + _address);

        while (_running.load(std::memory_order_relaxed)) {
            try {
                auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_io);
                _acceptor->accept(*socket);
                std::thread(&ControlServer::handleConnection, this, socket).detach();
            } catch (const std::exception&) {
                if (_running.load(std::memory_order_relaxed)) {
                    logMessage(_log, "Control server: accept failed");
                }
                break;
            }
        }
    }

    void ControlServer::handleConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    {
        while (_running.load(std::memory_order_relaxed)) {
            std::vector<uint8_t> raw;
            if (!transport::ReadFramed(*socket, raw)) {
                break;
            }

            transport::ControlRequest request;
            if (!transport::DecodeControlRequest(raw, request)) {
                continue;
            }

            transport::ControlResponse response;
            switch (request.type) {
                case transport::ControlRequestType::ListCameras:
                    response.type = transport::ControlResponseType::CameraList;
                    response.cameras = _backend->listCameras();
                    break;
                case transport::ControlRequestType::GetCameraInfo:
                    response.type = transport::ControlResponseType::CameraInfo;
                    response.cameraInfo = _backend->cameraInfo(request.cameraName);
                    break;
                case transport::ControlRequestType::StartRecording:
                    response.type = transport::ControlResponseType::Status;
                    response.status = _backend->start(request.cameraName);
                    break;
                case transport::ControlRequestType::StopRecording:
                    response.type = transport::ControlResponseType::Status;
                    response.status = _backend->stop(request.cameraName);
                    break;
                case transport::ControlRequestType::StartStreaming:
                    response.type = transport::ControlResponseType::Status;
                    response.status = _backend->show(request.cameraName);
                    break;
                case transport::ControlRequestType::StopStreaming:
                    response.type = transport::ControlResponseType::Status;
                    response.status = _backend->stopShow(request.cameraName);
                    break;
                case transport::ControlRequestType::SetExposure:
                    response.type = transport::ControlResponseType::Status;
                    response.status = _backend->setExposure(request.cameraName, request.value);
                    break;
                case transport::ControlRequestType::SetGain:
                    response.type = transport::ControlResponseType::Status;
                    response.status = _backend->setGain(request.cameraName, request.value);
                    break;
                case transport::ControlRequestType::GetStatus:
                    response.type = transport::ControlResponseType::CameraStatus;
                    response.cameraStatus = _backend->status(request.cameraName);
                    break;
                case transport::ControlRequestType::GetStats:
                    response.type = transport::ControlResponseType::Stats;
                    response.stats = _backend->stats(request.cameraName);
                    break;
                case transport::ControlRequestType::SetPreview:
                    response.type = transport::ControlResponseType::Status;
                    response.status = _backend->setPreview(request.cameraName, request.flag);
                    break;
            }

            if (!transport::WriteFramed(*socket, transport::EncodeControlResponse(response))) {
                break;
            }
        }
    }

}  // namespace vidre
