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
        _storage.appendFrame(static_cast<const void*>(payload), payloadSize);
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
        if (_endpoint.find(':') == std::string::npos) {
            std::cerr << "Remote storage: endpoint '" << _endpoint
                      << "' has no port (storage service default is 50053)\n";
        }
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
        enqueue(std::move(command));
    }

    void RemoteStorageSink::enqueue(transport::StorageCommand command)
    {
        const bool lifecycle = command.type != transport::StorageCommandType::Frame;
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.size() >= _maxQueue) {
            if (!lifecycle) {
                _dropped.fetch_add(1, std::memory_order_relaxed);
                return;
            }
            // Never drop lifecycle commands: evict the last ordinary frame from the back instead.
            auto markerIt = _lifecycleMarkers.end();
            while (markerIt != _lifecycleMarkers.begin()) {
                --markerIt;
                if (!*markerIt) {
                    break;
                }
            }
            if (markerIt == _lifecycleMarkers.begin() && *markerIt) {
                _dropped.fetch_add(1, std::memory_order_relaxed);
                return;
            }
            _queue.erase(_queue.begin() + std::distance(_lifecycleMarkers.begin(), markerIt));
            _lifecycleMarkers.erase(markerIt);
            _dropped.fetch_add(1, std::memory_order_relaxed);
        }
        _queue.push_back(transport::EncodeStorageCommand(std::move(command)));
        _lifecycleMarkers.push_back(lifecycle);
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
        enqueue(std::move(command));
    }

    void RemoteStorageSink::finish(int frameCount)
    {
        transport::StorageCommand command;
        command.type = transport::StorageCommandType::FinishSession;
        command.finishFrameCount = static_cast<uint32_t>(frameCount);
        enqueue(std::move(command));

        std::unique_lock<std::mutex> lock(_mutex);
        if (!_cv.wait_for(lock, std::chrono::seconds(5), [this] {
                return _queue.empty() && !_busy.load(std::memory_order_relaxed);
            })) {
            std::cerr << "Remote storage: finish timed out waiting for the queue to drain\n";
        }
    }

    bool RemoteStorageSink::connectAndSend(boost::asio::io_context& io, boost::asio::ip::tcp::socket& socket,
                                           const std::string& host, uint16_t port)
    {
        if (!transport::ConnectWithTimeout(io, socket, host, port, std::chrono::seconds(5))) {
            return false;
        }
        transport::SetSocketTimeouts(socket, std::chrono::seconds(10));
        return true;
    }

    void RemoteStorageSink::workerLoop()
    {
        const auto [host, port] = transport::SplitEndpoint(_endpoint, transport::kDefaultStoragePort);
        boost::asio::io_context ioContext;
        boost::asio::ip::tcp::socket socket(ioContext);
        bool connected = false;

        while (_running.load(std::memory_order_relaxed)) {
            std::vector<uint8_t> command;
            bool lifecycle = false;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait_for(lock, std::chrono::milliseconds(100), [this] {
                    return !_queue.empty() || !_running.load(std::memory_order_relaxed);
                });
                if (_queue.empty()) {
                    continue;
                }
                _busy.store(true, std::memory_order_relaxed);
                command = std::move(_queue.front());
                _queue.pop_front();
                lifecycle = _lifecycleMarkers.front();
                _lifecycleMarkers.pop_front();
            }

            if (!connected) {
                connected = connectAndSend(ioContext, socket, host, port);
                if (!connected) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    {
                        std::lock_guard<std::mutex> lock(_mutex);
                        if (lifecycle) {
                            // A session begin/finish must not be lost: retry it on reconnect.
                            _queue.push_front(std::move(command));
                            _lifecycleMarkers.push_front(true);
                        } else {
                            _dropped.fetch_add(1, std::memory_order_relaxed);
                        }
                        _busy.store(false, std::memory_order_relaxed);
                        _cv.notify_all();
                    }
                    continue;
                }
            }

            const bool ok = transport::WriteFramed(socket, command);
            if (ok) {
                _sent.fetch_add(1, std::memory_order_relaxed);
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _busy.store(false, std::memory_order_relaxed);
                    _cv.notify_all();
                }
            } else {
                connected = false;
                transport::ShutdownAndClose(socket);
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    if (lifecycle) {
                        _queue.push_front(std::move(command));
                        _lifecycleMarkers.push_front(true);
                    } else {
                        _dropped.fetch_add(1, std::memory_order_relaxed);
                    }
                    _busy.store(false, std::memory_order_relaxed);
                    _cv.notify_all();
                }
            }
        }
    }

}  // namespace vidre
