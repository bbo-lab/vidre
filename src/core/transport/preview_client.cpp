#include <core/transport/preview_client.hpp>
#include <core/transport/wire_protocol.hpp>

#include <chrono>
#include <iostream>
#include <thread>

namespace vidre {

    PreviewClient::PreviewClient(std::string host, uint16_t port, std::string cameraName)
        : _host(std::move(host))
        , _port(port)
        , _cameraName(std::move(cameraName))
    {
    }

    PreviewClient::~PreviewClient() {
        stop();
    }

    void PreviewClient::start()
    {
        if (_running.load(std::memory_order_relaxed)) {
            return;
        }
        _running.store(true);
        _thread = std::thread(&PreviewClient::recvLoop, this);
    }

    void PreviewClient::stop()
    {
        if (!_running.load(std::memory_order_relaxed)) {
            return;
        }
        _running.store(false);
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_socket) {
                boost::system::error_code ec;
                _socket->close(ec);
            }
        }
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    bool PreviewClient::latest(FrameEnvelope& envelope) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_latest.frameId == 0) {
            return false;
        }
        envelope = _latest;
        return true;
    }

    void PreviewClient::recvLoop()
    {
        while (_running.load(std::memory_order_relaxed)) {
            try {
                boost::asio::ip::tcp::resolver resolver(_io);
                const auto endpoints = resolver.resolve(_host, std::to_string(_port));
                boost::asio::ip::tcp::socket socket(_io);
                boost::asio::connect(socket, endpoints);

                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    if (!_running.load(std::memory_order_relaxed)) {
                        socket.close();
                        _socket.reset();
                        break;
                    }
                    _socket = std::make_unique<boost::asio::ip::tcp::socket>(std::move(socket));
                }

                const std::vector<uint8_t> name(_cameraName.begin(), _cameraName.end());
                if (!transport::WriteFramed(*_socket, name)) {
                    break;
                }

                while (_running.load(std::memory_order_relaxed)) {
                    std::vector<uint8_t> raw;
                    if (!transport::ReadFramed(*_socket, raw)) {
                        break;
                    }
                    FrameEnvelope envelope;
                    if (!transport::DecodeFrameEnvelope(raw, envelope)) {
                        continue;
                    }
                    {
                        std::lock_guard<std::mutex> lock(_mutex);
                        _latest = std::move(envelope);
                    }
                }

                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _socket.reset();
                }
            } catch (const std::exception& e) {
                std::cerr << "Preview client error (" << _cameraName << "): " << e.what() << "\n";
            }

            if (_running.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }

}  // namespace vidre
