#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <core/runtime/shared_frame_header.hpp>

namespace boost {
    namespace asio {
        class io_context;
    }
}

namespace boost::asio {
    namespace ip {
        class tcp;
    }
}

namespace vidre {

    class Storage;

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
        RemoteStorageSink(std::string endpoint, FrameEnvelope baseEnvelope);

        void begin() override;
        void publishFrame(const SharedFrameHeader& header, const uint8_t* payload, size_t payloadSize) override;
        void finish(int frameCount) override;

    private:
        std::string _endpoint;
        FrameEnvelope _baseEnvelope;
        void sendEnvelope(const FrameEnvelope& envelope);
    };

}  // namespace vidre