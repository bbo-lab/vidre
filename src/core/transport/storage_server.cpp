#include <core/transport/storage_server.hpp>
#include <core/transport/wire_protocol.hpp>

#include <core/storage/storage.hpp>

#include <filesystem>
#include <iostream>

namespace vidre {

    namespace {
        void logMessage(const StorageServer::LogCallback& log, const std::string& message) {
            if (log) {
                log(message);
            } else {
                std::cout << message << std::endl;
            }
        }
    }  // namespace

    StorageServer::StorageServer(std::string address, std::string outputDir, LogCallback log)
        : _address(std::move(address))
        , _outputDir(std::move(outputDir))
        , _log(std::move(log))
    {
    }

    StorageServer::~StorageServer() {
        stop();
    }

    void StorageServer::start()
    {
        if (_running.load(std::memory_order_relaxed)) {
            return;
        }
        _running.store(true);
        _thread = std::thread(&StorageServer::acceptLoop, this);
    }

    void StorageServer::stop()
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

    void StorageServer::acceptLoop()
    {
        try {
            const auto [host, port] = transport::SplitEndpoint(_address);
            boost::asio::ip::tcp::resolver resolver(_io);
            const auto endpoints = resolver.resolve(host, std::to_string(port));
            _acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(_io, *endpoints.begin());
        } catch (const std::exception& e) {
            logMessage(_log, std::string("Storage server: failed to bind ") + _address + ": " + e.what());
            _running.store(false);
            return;
        }

        logMessage(_log, "Storage server listening on " + _address);

        while (_running.load(std::memory_order_relaxed)) {
            try {
                auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_io);
                _acceptor->accept(*socket);
                std::thread(&StorageServer::handleConnection, this, socket).detach();
            } catch (const std::exception&) {
                if (_running.load(std::memory_order_relaxed)) {
                    logMessage(_log, "Storage server: accept failed");
                }
                break;
            }
        }
    }

    void StorageServer::handleConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    {
        std::unique_ptr<Storage> storage;
        int64_t receivedFrames = 0;
        int64_t lastFrameID = 0;
        int64_t droppedFrames = 0;
        std::string cameraName;

        auto finishSession = [&](int frameCount) {
            if (storage) {
                storage->updateHeader(frameCount);
                storage.reset();
                logMessage(_log, "Storage session closed for " + cameraName
                            + " (" + std::to_string(frameCount) + " frames, "
                            + std::to_string(droppedFrames) + " dropped)");
            }
        };

        while (_running.load(std::memory_order_relaxed)) {
            std::vector<uint8_t> raw;
            if (!transport::ReadFramed(*socket, raw)) {
                break;
            }

            transport::StorageCommand command;
            if (!transport::DecodeStorageCommand(raw, command)) {
                continue;
            }

            switch (command.type) {
                case transport::StorageCommandType::BeginSession: {
                    finishSession(static_cast<int>(receivedFrames));
                    const SessionSpec& spec = command.beginSession;

                    Config config{};
                    config.width = spec.width;
                    config.height = spec.height;
                    config.maxNumBuffer = spec.maxNumBuffer;
                    config.frameRate = spec.frameRate;
                    config.frameBytesActual = spec.frameBytesActual;
                    config.frameBytesBuffer = spec.frameBytesBuffer;
                    config.serialNumber = spec.serialNumber;
                    config.bitsPerPixel = spec.bitsPerPixel;
                    config.packed = spec.packed;
                    config.cameraVendor = spec.vendorName;
                    config.paddingX = 0;

                    std::string outputFile = spec.outputFile;
                    if (!outputFile.empty() && outputFile.find('/') == std::string::npos
                        && outputFile.find('\\') == std::string::npos && !_outputDir.empty()) {
                        outputFile = (std::filesystem::path(_outputDir) / outputFile).string();
                    }

                    storage = std::make_unique<Storage>(config, spec.headerFormat, outputFile);
                    storage->initiateHeader();
                    receivedFrames = 0;
                    lastFrameID = 0;
                    droppedFrames = 0;
                    cameraName = spec.cameraName;
                    logMessage(_log, "Storage session opened for " + cameraName + " -> " + outputFile);
                    break;
                }
                case transport::StorageCommandType::Frame: {
                    if (!storage) {
                        continue;
                    }
                    const FrameEnvelope& envelope = command.frame;
                    if (envelope.frameId > 0) {
                        if (lastFrameID > 0 && envelope.frameId > lastFrameID + 1) {
                            droppedFrames += static_cast<int64_t>(envelope.frameId - lastFrameID - 1);
                        }
                        lastFrameID = static_cast<int64_t>(envelope.frameId);
                    }
                    storage->appendFrame(envelope.payload.data());
                    storage->appendTail(static_cast<uint32_t>(envelope.frameId),
                                        static_cast<double>(envelope.frameTimestamp),
                                        envelope.computerTimeStamp);
                    receivedFrames++;
                    break;
                }
                case transport::StorageCommandType::FinishSession:
                    finishSession(static_cast<int>(command.finishFrameCount));
                    return;
            }
        }

        finishSession(static_cast<int>(receivedFrames));
    }

}  // namespace vidre
