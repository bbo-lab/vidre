#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include <boost/asio.hpp>

#include <core/transport/transport.hpp>

namespace vidre {

    class CameraBackend;

    class PreviewServer {
    public:
        using LogCallback = std::function<void(const std::string&)>;

        PreviewServer(CameraBackend* backend, std::string address, LogCallback log = {});
        ~PreviewServer();

        void start();
        void stop();

    private:
        void acceptLoop();
        void handleConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

        CameraBackend* _backend;
        std::string _address;
        LogCallback _log;
        std::atomic<bool> _running{false};
        std::thread _thread;
        boost::asio::io_context _io;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    };

}  // namespace vidre
