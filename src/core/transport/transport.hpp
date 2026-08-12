#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <core/runtime/shared_frame_header.hpp>

namespace vidre {

    class Storage;

    namespace transport {
        struct StorageCommand;
    }

    enum class TransportMode {
        LocalStorage,
        RemoteStorage
    };

    struct FrameEnvelope {
        std::string cameraName;
        std::string serialNumber;
        std::string vendorName;
        uint32_t width = 0;
        uint32_t height = 0;
        uint64_t bitsPerPixel = 0;
        uint64_t frameId = 0;
        uint64_t frameTimestamp = 0;
        int64_t computerTimeStamp = 0;
        std::vector<uint8_t> payload;
    };

    struct SessionSpec {
        std::string cameraName;
        std::string serialNumber;
        std::string vendorName;
        uint32_t width = 0;
        uint32_t height = 0;
        uint64_t bitsPerPixel = 0;
        bool packed = false;
        uint32_t frameBytesActual = 0;
        uint32_t frameBytesBuffer = 0;
        uint32_t maxNumBuffer = 0;
        double frameRate = 0.0;
        std::string outputFile;
        std::string headerFormat = "ccv";
    };

    class IFrameSink {
    public:
        virtual ~IFrameSink() = default;
        virtual void begin() = 0;
        virtual void publishFrame(const SharedFrameHeader& header, const uint8_t* payload, size_t payloadSize) = 0;
        virtual void finish(int frameCount) = 0;
    };

    class LocalStorageSink final : public IFrameSink {
    public:
        explicit LocalStorageSink(Storage& storage);

        void begin() override;
        void publishFrame(const SharedFrameHeader& header, const uint8_t* payload, size_t payloadSize) override;
        void finish(int frameCount) override;

    private:
        Storage& _storage;
    };

    class RemoteStorageSink final : public IFrameSink {
    public:
        RemoteStorageSink(std::string endpoint, FrameEnvelope baseEnvelope, SessionSpec sessionSpec);
        ~RemoteStorageSink() override;

        void begin() override;
        void publishFrame(const SharedFrameHeader& header, const uint8_t* payload, size_t payloadSize) override;
        void finish(int frameCount) override;

        int64_t sentFrames() const { return _sent.load(std::memory_order_relaxed); }
        int64_t droppedFrames() const { return _dropped.load(std::memory_order_relaxed); }

    private:
        void enqueue(transport::StorageCommand command);
        void workerLoop();
        bool connectAndSend(boost::asio::ip::tcp::socket& socket, const std::string& host, uint16_t port);

        std::string _endpoint;
        FrameEnvelope _baseEnvelope;
        SessionSpec _sessionSpec;

        std::mutex _mutex;
        std::condition_variable _cv;
        std::deque<std::vector<uint8_t>> _queue;
        std::deque<bool> _lifecycleMarkers;
        size_t _maxQueue = 64;

        std::atomic<bool> _running{false};
        std::atomic<bool> _busy{false};
        std::atomic<int64_t> _sent{0};
        std::atomic<int64_t> _dropped{0};
        std::thread _thread;
    };

}  // namespace vidre