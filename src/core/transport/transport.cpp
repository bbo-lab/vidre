#include <core/transport/transport.hpp>
#include <core/transport/wire_protocol.hpp>

#include <chrono>
#include <cstring>
#include <iostream>

#include <boost/asio.hpp>

#include <core/storage/storage.hpp>

namespace vidre {

    LocalStorageSink::LocalStorageSink(Storage& storage)
        : _storage(storage)
    {
    }

    void LocalStorageSink::begin() {
        _storage.initiateHeader();
    }

    void LocalStorageSink::publishFrame(const SharedFrameHeader& header, const uint8_t* payload, size_t payloadSize) {
        (void)payloadSize;
        _storage.appendFrame(static_cast<const void*>(payload));
        _storage.appendTail(static_cast<uint32_t>(header.frameID), header.timeStamp, header.computerTimeStamp);
    }

    void LocalStorageSink::finish(int frameCount) {
        _storage.updateHeader(frameCount);
    }

    RemoteStorageSink::RemoteStorageSink(std::string endpoint, FrameEnvelope baseEnvelope, SessionSpec sessionSpec)
        : _endpoint(std::move(endpoint))
        , _baseEnvelope(std::move(baseEnvelope))
        , _sessionSpec(std::move(sessionSpec))
    {
        _running.store(true);
        _thread = std::thread(&RemoteStorageSink::workerLoop, this);
    }

    RemoteStorageSink::~RemoteStorageSink() {
        _running.store(false);
        _cv.notify_all();
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void RemoteStorageSink::begin()
    {
        transport::StorageCommand command;
        command.type = transport::StorageCommandType::BeginSession;
        command.beginSession = _sessionSpec;
        enqueue(transport::EncodeStorageCommand(command));
    }

    void RemoteStorageSink::enqueue(std::vector<uint8_t> command)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.size() >= _maxQueue) {
            _dropped.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        _queue.push_back(std::move(command));
        _cv.notify_one();
    }

    void RemoteStorageSink::publishFrame(const SharedFrameHeader& header, const uint8_t* payload, size_t payloadSize)
    {
        FrameEnvelope envelope = _baseEnvelope;
        envelope.frameId = static_cast<uint64_t>(header.frameID);
        envelope.frameTimestamp = header.timeStamp;
        envelope.computerTimeStamp = header.computerTimeStamp;
        envelope.payload.assign(payload, payload + payloadSize);

        transport::StorageCommand command;
        command.type = transport::StorageCommandType::Frame;
        command.frame = std::move(envelope);
        enqueue(transport::EncodeStorageCommand(command));
    }

    void RemoteStorageSink::finish(int frameCount)
    {
        transport::StorageCommand command;
        command.type = transport::StorageCommandType::FinishSession;
        command.finishFrameCount = static_cast<uint32_t>(frameCount);
        enqueue(transport::EncodeStorageCommand(command));

        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock, [this] { return _queue.empty() && !_busy.load(std::memory_order_relaxed); });
    }

    bool RemoteStorageSink::connectAndSend(boost::asio::ip::tcp::socket& socket, const std::string& host, uint16_t port)
    {
        try {
            boost::asio::ip::tcp::resolver resolver(socket.get_executor().context());
            auto endpoints = resolver.resolve(host, std::to_string(port));
            boost::asio::connect(socket, endpoints);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Remote storage connect failed: " << e.what() << "\n";
            return false;
        }
    }

    void RemoteStorageSink::workerLoop()
    {
        const auto [host, port] = transport::SplitEndpoint(_endpoint);
        boost::asio::io_context ioContext;
        boost::asio::ip::tcp::socket socket(ioContext);
        bool connected = false;

        while (_running.load(std::memory_order_relaxed)) {
            std::vector<uint8_t> command;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait_for(lock, std::chrono::milliseconds(100), [this] {
                    return !_queue.empty() || !_running.load(std::memory_order_relaxed);
                });
                if (_queue.empty()) {
                    continue;
                }
                command = std::move(_queue.front());
                _queue.pop_front();
            }

            if (!connected) {
                connected = connectAndSend(socket, host, port);
                if (!connected) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    _dropped.fetch_add(1, std::memory_order_relaxed);
                    continue;
                }
            }

            _busy.store(true, std::memory_order_relaxed);
            const bool ok = transport::WriteFramed(socket, command);
            _busy.store(false, std::memory_order_relaxed);
            _cv.notify_all();

            if (ok) {
                _sent.fetch_add(1, std::memory_order_relaxed);
            } else {
                connected = false;
                try {
                    socket.close();
                } catch (...) {
                }
                _dropped.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

}  // namespace vidre
