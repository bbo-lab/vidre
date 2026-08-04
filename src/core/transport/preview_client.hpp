#pragma once

#include <atomic>
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
        std::atomic<int64_t> _frameId{0};
        boost::asio::io_context _io;
    };

}  // namespace vidre
