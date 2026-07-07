#include <core/transport/transport.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sstream>
#include <array>

#include <boost/asio.hpp>

#include <core/storage/storage.hpp>

namespace vidre {

namespace {

void appendVarint(std::vector<uint8_t>& buffer, uint64_t value)
{
    while (value >= 0x80) {
        buffer.push_back(static_cast<uint8_t>(value) | 0x80U);
        value >>= 7U;
    }
    buffer.push_back(static_cast<uint8_t>(value));
}

uint64_t zigZagEncode64(int64_t value)
{
    return (static_cast<uint64_t>(value) << 1U) ^ static_cast<uint64_t>(value >> 63U);
}

void appendTag(std::vector<uint8_t>& buffer, uint32_t fieldNumber, uint32_t wireType)
{
    appendVarint(buffer, (static_cast<uint64_t>(fieldNumber) << 3U) | wireType);
}

void appendLengthDelimited(std::vector<uint8_t>& buffer, uint32_t fieldNumber, const std::string& value)
{
    appendTag(buffer, fieldNumber, 2U);
    appendVarint(buffer, value.size());
    buffer.insert(buffer.end(), value.begin(), value.end());
}

void appendLengthDelimited(std::vector<uint8_t>& buffer, uint32_t fieldNumber, const std::vector<uint8_t>& value)
{
    appendTag(buffer, fieldNumber, 2U);
    appendVarint(buffer, value.size());
    buffer.insert(buffer.end(), value.begin(), value.end());
}

void appendVarintField(std::vector<uint8_t>& buffer, uint32_t fieldNumber, uint64_t value)
{
    appendTag(buffer, fieldNumber, 0U);
    appendVarint(buffer, value);
}

std::vector<uint8_t> encodeFrameEnvelope(const FrameEnvelope& envelope)
{
    std::vector<uint8_t> buffer;
    buffer.reserve(envelope.payload.size() + 128);

    appendLengthDelimited(buffer, 1U, envelope.cameraName);
    appendLengthDelimited(buffer, 2U, envelope.serialNumber);
    appendLengthDelimited(buffer, 3U, envelope.vendorName);
    appendVarintField(buffer, 4U, envelope.width);
    appendVarintField(buffer, 5U, envelope.height);
    appendVarintField(buffer, 6U, envelope.bitsPerPixel);
    appendVarintField(buffer, 7U, envelope.frameId);
    appendVarintField(buffer, 8U, envelope.frameTimestamp);
    appendVarintField(buffer, 9U, zigZagEncode64(envelope.computerTimeStamp));
    appendLengthDelimited(buffer, 10U, envelope.payload);

    return buffer;
}

std::pair<std::string, uint16_t> splitEndpoint(const std::string& endpoint)
{
    const auto separator = endpoint.rfind(':');
    if (separator == std::string::npos || separator == 0 || separator + 1 >= endpoint.size()) {
        return {endpoint, 50051U};
    }

    return {endpoint.substr(0, separator), static_cast<uint16_t>(std::stoi(endpoint.substr(separator + 1)))};
}

std::array<uint8_t, 4> toBigEndian32(uint32_t value)
{
    return {
        static_cast<uint8_t>((value >> 24U) & 0xFFU),
        static_cast<uint8_t>((value >> 16U) & 0xFFU),
        static_cast<uint8_t>((value >> 8U) & 0xFFU),
        static_cast<uint8_t>(value & 0xFFU)
    };
}

}  // namespace

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

    RemoteStorageSink::RemoteStorageSink(std::string endpoint, FrameEnvelope baseEnvelope)
        : _endpoint(std::move(endpoint))
        , _baseEnvelope(std::move(baseEnvelope))
    {
    }

    void RemoteStorageSink::begin()
    {
    }

    void RemoteStorageSink::sendEnvelope(const FrameEnvelope& envelope)
    {
        try {
            const auto payload = encodeFrameEnvelope(envelope);
            const uint32_t payloadSize = static_cast<uint32_t>(payload.size());

            const auto [host, port] = splitEndpoint(_endpoint);
            boost::asio::io_context ioContext;
            boost::asio::ip::tcp::resolver resolver(ioContext);
            boost::asio::ip::tcp::socket socket(ioContext);

            auto endpoints = resolver.resolve(host, std::to_string(port));
            boost::asio::connect(socket, endpoints);

            const auto networkLength = toBigEndian32(payloadSize);
            std::vector<boost::asio::const_buffer> buffers;
            buffers.emplace_back(boost::asio::buffer(networkLength));
            buffers.emplace_back(boost::asio::buffer(payload));
            boost::asio::write(socket, buffers);
        } catch (const std::exception& e) {
            std::cerr << "Remote storage sink write failed: " << e.what() << "\n";
        }
    }

    void RemoteStorageSink::publishFrame(const SharedFrameHeader& header, const uint8_t* payload, size_t payloadSize)
    {
        FrameEnvelope envelope = _baseEnvelope;
        envelope.frameId = static_cast<uint64_t>(header.frameID);
        envelope.frameTimestamp = header.timeStamp;
        envelope.computerTimeStamp = header.computerTimeStamp;
        envelope.payload.assign(payload, payload + payloadSize);

        sendEnvelope(envelope);
    }

    void RemoteStorageSink::finish(int frameCount)
    {
        (void)frameCount;
    }

}  // namespace vidre