#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include <boost/asio.hpp>

namespace vidre {

    class StorageServer {
    public:
        using LogCallback = std::function<void(const std::string&)>;

        StorageServer(std::string address, std::string outputDir, LogCallback log = {});
        ~StorageServer();

        void start();
        void stop();

    private:
        void acceptLoop();
        void handleConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

        std::string _address;
        std::string _outputDir;
        LogCallback _log;
        std::atomic<bool> _running{false};
        std::thread _thread;
        boost::asio::io_context _io;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    };

}  // namespace vidre
