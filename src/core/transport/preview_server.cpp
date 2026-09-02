#include <core/transport/preview_server.hpp>
#include <core/transport/camera_backend.hpp>
#include <core/transport/wire_protocol.hpp>

#include <chrono>
#include <iostream>
#include <thread>

namespace vidre {

    namespace {
        void logMessage(const PreviewServer::LogCallback& log, const std::string& message) {
            if (log) {
                log(message);
            } else {
                std::cout << message << std::endl;
            }
        }
    }  // namespace

    PreviewServer::PreviewServer(CameraBackend* backend, std::string address, LogCallback log)
        : _backend(backend)
        , _address(std::move(address))
        , _log(std::move(log))
    {
    }

    PreviewServer::~PreviewServer() {
        stop();
    }

    void PreviewServer::start()
    {
        if (_running.load(std::memory_order_relaxed)) {
            return;
        }
        _running.store(true);
        _thread = std::thread(&PreviewServer::acceptLoop, this);
    }

    void PreviewServer::stop()
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

    void PreviewServer::acceptLoop()
    {
        try {
            const auto [host, port] = transport::SplitEndpoint(_address);
            if (port == 0) {
                throw std::runtime_error("invalid port in address '" + _address + "'");
            }
            boost::asio::ip::tcp::resolver resolver(_io);
            const auto endpoints = resolver.resolve(host, std::to_string(port));
            _acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(_io, *endpoints.begin());
        } catch (const std::exception& e) {
            logMessage(_log, std::string("Preview server: failed to bind ") + _address + ": " + e.what());
            _running.store(false);
            return;
        }

        logMessage(_log, "Preview server listening on " + _address);

        while (_running.load(std::memory_order_relaxed)) {
            try {
                auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_io);
                _acceptor->accept(*socket);
                std::thread(&PreviewServer::handleConnection, this, socket).detach();
            } catch (const std::exception&) {
                if (_running.load(std::memory_order_relaxed)) {
                    logMessage(_log, "Preview server: accept failed");
                }
                break;
            }
        }
    }

    void PreviewServer::handleConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    {
        std::vector<uint8_t> nameRaw;
        if (!transport::ReadFramed(*socket, nameRaw)) {
            return;
        }
        const std::string cameraName(nameRaw.begin(), nameRaw.end());

        uint64_t lastFrameId = 0;
        bool firstFrame = true;

        while (_running.load(std::memory_order_relaxed)) {
            FrameEnvelope envelope;
            if (_backend->latestFrame(cameraName, envelope)) {
                if (firstFrame || envelope.frameId != lastFrameId) {
                    if (!transport::WriteFramed(*socket, transport::EncodeFrameEnvelope(envelope))) {
                        break;
                    }
                    lastFrameId = envelope.frameId;
                    firstFrame = false;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    }

}  // namespace vidre
