#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <core/transport/transport.hpp>

namespace vidre {

    class PreviewClient {
    public:
        PreviewClient(std::string host, uint16_t port, std::string cameraName);
        ~PreviewClient();

        void start();
        void stop();
        bool isRunning() const { return _running.load(std::memory_order_relaxed); }

        bool latest(FrameEnvelope& envelope) const;

    private:
        void recvLoop();

        std::string _host;
        uint16_t _port;
        std::string _cameraName;
        std::atomic<bool> _running{false};
        std::thread _thread;
        mutable std::mutex _mutex;
        FrameEnvelope _latest;
        boost::asio::io_context _io;
        std::unique_ptr<boost::asio::ip::tcp::socket> _socket;
    };

}  // namespace vidre
