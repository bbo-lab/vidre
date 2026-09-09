#include <core/transport/wire_protocol.hpp>

#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

namespace vidre::transport {

namespace {

    constexpr uint32_t kWireVarint = 0;
    constexpr uint32_t kWireFixed64 = 1;
    constexpr uint32_t kWireLengthDelimited = 2;
    constexpr uint32_t kWireFixed32 = 5;

    class Encoder {
    public:
        void Varint(uint64_t value) {
            while (value >= 0x80U) {
                _buffer.push_back(static_cast<uint8_t>(value) | 0x80U);
                value >>= 7U;
            }
            _buffer.push_back(static_cast<uint8_t>(value));
        }

        void Tag(uint32_t fieldNumber, uint32_t wireType) {
            Varint((static_cast<uint64_t>(fieldNumber) << 3U) | wireType);
        }

        void StringField(uint32_t fieldNumber, const std::string& value) {
            if (value.empty()) {
                return;
            }
            Tag(fieldNumber, kWireLengthDelimited);
            Varint(value.size());
            _buffer.insert(_buffer.end(), value.begin(), value.end());
        }

        void BytesField(uint32_t fieldNumber, const std::vector<uint8_t>& value) {
            if (value.empty()) {
                return;
            }
            Tag(fieldNumber, kWireLengthDelimited);
            Varint(value.size());
            _buffer.insert(_buffer.end(), value.begin(), value.end());
        }

        void VarintField(uint32_t fieldNumber, uint64_t value) {
            Tag(fieldNumber, kWireVarint);
            Varint(value);
        }

        void ZigZagField(uint32_t fieldNumber, int64_t value) {
            const uint64_t encoded = (static_cast<uint64_t>(value) << 1U) ^ static_cast<uint64_t>(value >> 63);
            VarintField(fieldNumber, encoded);
        }

        void DoubleField(uint32_t fieldNumber, double value) {
            Tag(fieldNumber, kWireFixed64);
            uint64_t bits = 0;
            std::memcpy(&bits, &value, sizeof(bits));
            for (int i = 0; i < 8; ++i) {
                _buffer.push_back(static_cast<uint8_t>(bits >> (i * 8)));
            }
        }

        void BoolField(uint32_t fieldNumber, bool value) {
            VarintField(fieldNumber, value ? 1U : 0U);
        }

        
        void MessageField(uint32_t fieldNumber, const std::vector<uint8_t>& message) {
            Tag(fieldNumber, kWireLengthDelimited);
            Varint(message.size());
            _buffer.insert(_buffer.end(), message.begin(), message.end());
        }

        std::vector<uint8_t> Take() {
            return std::move(_buffer);
        }

    private:
        std::vector<uint8_t> _buffer;
    };

    class Decoder {
    public:
        explicit Decoder(const std::vector<uint8_t>& data)
            : _data(data)
            , _offset(0)
        {
        }

        bool Ok() const {
            return _offset <= _data.size();
        }

        bool NextField(uint32_t& fieldNumber, uint32_t& wireType) {
            if (_offset >= _data.size()) {
                return false;
            }
            uint64_t tag = 0;
            if (!ReadVarint(tag)) {
                return false;
            }
            fieldNumber = static_cast<uint32_t>(tag >> 3U);
            wireType = static_cast<uint32_t>(tag & 0x7U);
            return true;
        }

        bool ReadVarint(uint64_t& value) {
            value = 0;
            for (int shift = 0; shift < 64; shift += 7) {
                if (_offset >= _data.size()) {
                    return false;
                }
                const uint8_t byte = _data[_offset++];
                value |= static_cast<uint64_t>(byte & 0x7FU) << shift;
                if ((byte & 0x80U) == 0) {
                    return true;
                }
            }
            return false;
        }

        bool ReadLengthDelimited(std::vector<uint8_t>& value) {
            uint64_t length = 0;
            if (!ReadVarint(length)) {
                return false;
            }
            if (length > _data.size() - _offset) {
                return false;
            }
            value.assign(_data.begin() + static_cast<std::ptrdiff_t>(_offset),
                         _data.begin() + static_cast<std::ptrdiff_t>(_offset + length));
            _offset += static_cast<size_t>(length);
            return true;
        }

        bool ReadString(std::string& value) {
            std::vector<uint8_t> raw;
            if (!ReadLengthDelimited(raw)) {
                return false;
            }
            value.assign(raw.begin(), raw.end());
            return true;
        }

        bool ReadBytes(std::vector<uint8_t>& value) {
            return ReadLengthDelimited(value);
        }

        bool ReadZigZag(int64_t& value) {
            uint64_t encoded = 0;
            if (!ReadVarint(encoded)) {
                return false;
            }
            value = static_cast<int64_t>(encoded >> 1U) ^ -static_cast<int64_t>(encoded & 1U);
            return true;
        }

        bool ReadFixed64(double& value) {
            if (_offset + 8 > _data.size()) {
                return false;
            }
            uint64_t bits = 0;
            std::memcpy(&bits, _data.data() + _offset, 8);
            _offset += 8;
            std::memcpy(&value, &bits, sizeof(bits));
            return true;
        }

        bool ReadFixed32(uint32_t& value) {
            if (_offset + 4 > _data.size()) {
                return false;
            }
            value = 0;
            std::memcpy(&value, _data.data() + _offset, 4);
            _offset += 4;
            return true;
        }

        bool Skip(uint32_t wireType) {
            switch (wireType) {
                case kWireVarint: {
                    uint64_t ignored = 0;
                    return ReadVarint(ignored);
                }
                case kWireFixed64:
                    if (_offset + 8 > _data.size()) {
                        return false;
                    }
                    _offset += 8;
                    return true;
                case kWireFixed32:
                    if (_offset + 4 > _data.size()) {
                        return false;
                    }
                    _offset += 4;
                    return true;
                case kWireLengthDelimited: {
                    std::vector<uint8_t> ignored;
                    return ReadLengthDelimited(ignored);
                }
                default:
                    return false;
            }
        }

    private:
        const std::vector<uint8_t>& _data;
        size_t _offset;
    };

    std::array<uint8_t, 4> ToBigEndian32(uint32_t value) {
        return {
            static_cast<uint8_t>((value >> 24U) & 0xFFU),
            static_cast<uint8_t>((value >> 16U) & 0xFFU),
            static_cast<uint8_t>((value >> 8U) & 0xFFU),
            static_cast<uint8_t>(value & 0xFFU)
        };
    }

    uint32_t FromBigEndian32(const uint8_t* bytes) {
        return (static_cast<uint32_t>(bytes[0]) << 24U)
             | (static_cast<uint32_t>(bytes[1]) << 16U)
             | (static_cast<uint32_t>(bytes[2]) << 8U)
             | static_cast<uint32_t>(bytes[3]);
    }

}  // namespace

    std::vector<uint8_t> EncodeFrameEnvelope(const FrameEnvelope& envelope)
    {
        Encoder encoder;
        encoder.StringField(1, envelope.cameraName);
        encoder.StringField(2, envelope.serialNumber);
        encoder.StringField(3, envelope.vendorName);
        encoder.VarintField(4, envelope.width);
        encoder.VarintField(5, envelope.height);
        encoder.VarintField(6, envelope.bitsPerPixel);
        encoder.VarintField(7, envelope.frameId);
        encoder.VarintField(8, envelope.frameTimestamp);
        encoder.ZigZagField(9, envelope.computerTimeStamp);
        encoder.BytesField(10, envelope.payload);
        return encoder.Take();
    }

    bool DecodeFrameEnvelope(const std::vector<uint8_t>& data, FrameEnvelope& envelope)
    {
        envelope = FrameEnvelope{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    envelope.cameraName = std::move(value);
                    break;
                }
                case 2: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    envelope.serialNumber = std::move(value);
                    break;
                }
                case 3: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    envelope.vendorName = std::move(value);
                    break;
                }
                case 4: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    envelope.width = static_cast<uint32_t>(value);
                    break;
                }
                case 5: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    envelope.height = static_cast<uint32_t>(value);
                    break;
                }
                case 6: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    envelope.bitsPerPixel = value;
                    break;
                }
                case 7: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    envelope.frameId = value;
                    break;
                }
                case 8: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    envelope.frameTimestamp = value;
                    break;
                }
                case 9: {
                    int64_t value = 0;
                    if (!decoder.ReadZigZag(value)) {
                        return false;
                    }
                    envelope.computerTimeStamp = value;
                    break;
                }
                case 10: {
                    std::vector<uint8_t> value;
                    if (!decoder.ReadBytes(value)) {
                        return false;
                    }
                    envelope.payload = std::move(value);
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::vector<uint8_t> EncodeSessionSpec(const SessionSpec& spec)
    {
        Encoder encoder;
        encoder.StringField(1, spec.cameraName);
        encoder.StringField(2, spec.serialNumber);
        encoder.StringField(3, spec.vendorName);
        encoder.VarintField(4, spec.width);
        encoder.VarintField(5, spec.height);
        encoder.VarintField(6, spec.bitsPerPixel);
        encoder.BoolField(7, spec.packed);
        encoder.VarintField(8, spec.frameBytesActual);
        encoder.VarintField(9, spec.frameBytesBuffer);
        encoder.VarintField(10, spec.maxNumBuffer);
        encoder.DoubleField(11, spec.frameRate);
        encoder.StringField(12, spec.outputFile);
        encoder.StringField(13, spec.headerFormat);
        return encoder.Take();
    }

    bool DecodeSessionSpec(const std::vector<uint8_t>& data, SessionSpec& spec)
    {
        spec = SessionSpec{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    spec.cameraName = std::move(value);
                    break;
                }
                case 2: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    spec.serialNumber = std::move(value);
                    break;
                }
                case 3: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    spec.vendorName = std::move(value);
                    break;
                }
                case 4: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    spec.width = static_cast<uint32_t>(value);
                    break;
                }
                case 5: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    spec.height = static_cast<uint32_t>(value);
                    break;
                }
                case 6: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    spec.bitsPerPixel = value;
                    break;
                }
                case 7: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    spec.packed = value != 0;
                    break;
                }
                case 8: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    spec.frameBytesActual = static_cast<uint32_t>(value);
                    break;
                }
                case 9: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    spec.frameBytesBuffer = static_cast<uint32_t>(value);
                    break;
                }
                case 10: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    spec.maxNumBuffer = static_cast<uint32_t>(value);
                    break;
                }
                case 11: {
                    double value = 0.0;
                    if (!decoder.ReadFixed64(value)) {
                        return false;
                    }
                    spec.frameRate = value;
                    break;
                }
                case 12: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    spec.outputFile = std::move(value);
                    break;
                }
                case 13: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    spec.headerFormat = std::move(value);
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::vector<uint8_t> EncodeStorageCommand(const StorageCommand& command)
    {
        Encoder encoder;
        switch (command.type) {
            case StorageCommandType::Frame:
                encoder.MessageField(1, EncodeFrameEnvelope(command.frame));
                break;
            case StorageCommandType::BeginSession:
                encoder.MessageField(2, EncodeSessionSpec(command.beginSession));
                break;
            case StorageCommandType::FinishSession:
                encoder.VarintField(3, command.finishFrameCount);
                break;
        }
        return encoder.Take();
    }

    bool DecodeStorageCommand(const std::vector<uint8_t>& data, StorageCommand& command)
    {
        command = StorageCommand{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::vector<uint8_t> raw;
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    command.type = StorageCommandType::Frame;
                    if (!DecodeFrameEnvelope(raw, command.frame)) {
                        return false;
                    }
                    break;
                }
                case 2: {
                    std::vector<uint8_t> raw;
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    command.type = StorageCommandType::BeginSession;
                    if (!DecodeSessionSpec(raw, command.beginSession)) {
                        return false;
                    }
                    break;
                }
                case 3: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    command.type = StorageCommandType::FinishSession;
                    command.finishFrameCount = static_cast<uint32_t>(value);
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::vector<uint8_t> EncodeCameraInfo(const CameraInfo& info)
    {
        Encoder encoder;
        encoder.StringField(1, info.cameraName);
        encoder.StringField(2, info.serialNumber);
        encoder.StringField(3, info.vendorName);
        encoder.VarintField(4, info.width);
        encoder.VarintField(5, info.height);
        encoder.VarintField(6, info.frameBytes);
        encoder.VarintField(7, info.bufferSize);
        encoder.VarintField(8, info.maxNumBuffers);
        encoder.DoubleField(9, info.frameRate);
        encoder.DoubleField(10, info.exposure);
        encoder.DoubleField(11, info.gain);
        encoder.VarintField(12, info.bitsPerPixel);
        encoder.BoolField(13, info.packed);
        return encoder.Take();
    }

    bool DecodeCameraInfo(const std::vector<uint8_t>& data, CameraInfo& info)
    {
        info = CameraInfo{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    info.cameraName = std::move(value);
                    break;
                }
                case 2: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    info.serialNumber = std::move(value);
                    break;
                }
                case 3: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    info.vendorName = std::move(value);
                    break;
                }
                case 4: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    info.width = static_cast<uint32_t>(value);
                    break;
                }
                case 5: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    info.height = static_cast<uint32_t>(value);
                    break;
                }
                case 6: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    info.frameBytes = static_cast<uint32_t>(value);
                    break;
                }
                case 7: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    info.bufferSize = static_cast<uint32_t>(value);
                    break;
                }
                case 8: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    info.maxNumBuffers = static_cast<uint32_t>(value);
                    break;
                }
                case 9: {
                    double value = 0.0;
                    if (!decoder.ReadFixed64(value)) {
                        return false;
                    }
                    info.frameRate = value;
                    break;
                }
                case 10: {
                    double value = 0.0;
                    if (!decoder.ReadFixed64(value)) {
                        return false;
                    }
                    info.exposure = value;
                    break;
                }
                case 11: {
                    double value = 0.0;
                    if (!decoder.ReadFixed64(value)) {
                        return false;
                    }
                    info.gain = value;
                    break;
                }
                case 12: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    info.bitsPerPixel = value;
                    break;
                }
                case 13: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    info.packed = value != 0;
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::vector<uint8_t> EncodeCameraStatus(const CameraStatus& status)
    {
        Encoder encoder;
        encoder.VarintField(1, static_cast<uint64_t>(status.recordingStatus));
        encoder.BoolField(2, status.armed);
        encoder.BoolField(3, status.running);
        encoder.BoolField(4, status.error);
        return encoder.Take();
    }

    bool DecodeCameraStatus(const std::vector<uint8_t>& data, CameraStatus& status)
    {
        status = CameraStatus{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    status.recordingStatus = static_cast<int32_t>(value);
                    break;
                }
                case 2: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    status.armed = value != 0;
                    break;
                }
                case 3: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    status.running = value != 0;
                    break;
                }
                case 4: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    status.error = value != 0;
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::vector<uint8_t> EncodeRecordingStats(const RecordingStats& stats)
    {
        Encoder encoder;
        encoder.VarintField(1, static_cast<uint64_t>(stats.savedFrames));
        encoder.VarintField(2, static_cast<uint64_t>(stats.droppedFrames));
        encoder.VarintField(3, static_cast<uint64_t>(stats.sentFrames));
        return encoder.Take();
    }

    bool DecodeRecordingStats(const std::vector<uint8_t>& data, RecordingStats& stats)
    {
        stats = RecordingStats{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    stats.savedFrames = static_cast<int64_t>(value);
                    break;
                }
                case 2: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    stats.droppedFrames = static_cast<int64_t>(value);
                    break;
                }
                case 3: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    stats.sentFrames = static_cast<int64_t>(value);
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::vector<uint8_t> EncodeStatusResponse(const StatusResponse& response)
    {
        Encoder encoder;
        encoder.BoolField(1, response.ok);
        encoder.StringField(2, response.message);
        return encoder.Take();
    }

    bool DecodeStatusResponse(const std::vector<uint8_t>& data, StatusResponse& response)
    {
        response = StatusResponse{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    response.ok = value != 0;
                    break;
                }
                case 2: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    response.message = std::move(value);
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::vector<uint8_t> EncodeControlRequest(const ControlRequest& request)
    {
        Encoder encoder;
        switch (request.type) {
            case ControlRequestType::ListCameras:
                encoder.Tag(1, kWireLengthDelimited);
                encoder.Varint(0);
                break;
            case ControlRequestType::GetCameraInfo: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                encoder.MessageField(2, sub.Take());
                break;
            }
            case ControlRequestType::StartRecording: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                encoder.MessageField(3, sub.Take());
                break;
            }
            case ControlRequestType::StopRecording: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                encoder.MessageField(4, sub.Take());
                break;
            }
            case ControlRequestType::StartStreaming: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                encoder.MessageField(5, sub.Take());
                break;
            }
            case ControlRequestType::StopStreaming: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                encoder.MessageField(6, sub.Take());
                break;
            }
            case ControlRequestType::SetExposure: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                sub.DoubleField(2, request.value);
                encoder.MessageField(7, sub.Take());
                break;
            }
            case ControlRequestType::SetGain: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                sub.DoubleField(2, request.value);
                encoder.MessageField(8, sub.Take());
                break;
            }
            case ControlRequestType::GetStatus: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                encoder.MessageField(9, sub.Take());
                break;
            }
            case ControlRequestType::GetStats: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                encoder.MessageField(10, sub.Take());
                break;
            }
            case ControlRequestType::SetPreview: {
                Encoder sub;
                sub.StringField(1, request.cameraName);
                sub.BoolField(2, request.flag);
                encoder.MessageField(11, sub.Take());
                break;
            }
        }
        return encoder.Take();
    }

    bool DecodeControlRequest(const std::vector<uint8_t>& data, ControlRequest& request)
    {
        request = ControlRequest{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            std::vector<uint8_t> raw;
            switch (field) {
                case 1:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::ListCameras;
                    break;
                case 2:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::GetCameraInfo;
                    if (!DecodeCameraRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 3:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::StartRecording;
                    if (!DecodeCameraRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 4:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::StopRecording;
                    if (!DecodeCameraRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 5:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::StartStreaming;
                    if (!DecodeCameraRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 6:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::StopStreaming;
                    if (!DecodeCameraRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 7:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::SetExposure;
                    if (!DecodeSetExposureRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 8:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::SetGain;
                    if (!DecodeSetGainRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 9:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::GetStatus;
                    if (!DecodeCameraRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 10:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::GetStats;
                    if (!DecodeCameraRequest(raw, request)) {
                        return false;
                    }
                    break;
                case 11:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    request.type = ControlRequestType::SetPreview;
                    if (!DecodePreviewRequest(raw, request)) {
                        return false;
                    }
                    break;
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::vector<uint8_t> EncodeControlResponse(const ControlResponse& response)
    {
        Encoder encoder;
        switch (response.type) {
            case ControlResponseType::CameraList: {
                Encoder sub;
                for (const auto& camera : response.cameras) {
                    sub.MessageField(1, EncodeCameraInfo(camera));
                }
                encoder.MessageField(1, sub.Take());
                break;
            }
            case ControlResponseType::CameraInfo:
                encoder.MessageField(2, EncodeCameraInfo(response.cameraInfo));
                break;
            case ControlResponseType::Status:
                encoder.MessageField(3, EncodeStatusResponse(response.status));
                break;
            case ControlResponseType::CameraStatus:
                encoder.MessageField(4, EncodeCameraStatus(response.cameraStatus));
                break;
            case ControlResponseType::Stats:
                encoder.MessageField(5, EncodeRecordingStats(response.stats));
                break;
        }
        return encoder.Take();
    }

    bool DecodeControlResponse(const std::vector<uint8_t>& data, ControlResponse& response)
    {
        response = ControlResponse{};
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            std::vector<uint8_t> raw;
            switch (field) {
                case 1:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    response.type = ControlResponseType::CameraList;
                    if (!DecodeCameraInfoList(raw, response.cameras)) {
                        return false;
                    }
                    break;
                case 2:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    response.type = ControlResponseType::CameraInfo;
                    if (!DecodeCameraInfo(raw, response.cameraInfo)) {
                        return false;
                    }
                    break;
                case 3:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    response.type = ControlResponseType::Status;
                    if (!DecodeStatusResponse(raw, response.status)) {
                        return false;
                    }
                    break;
                case 4:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    response.type = ControlResponseType::CameraStatus;
                    if (!DecodeCameraStatus(raw, response.cameraStatus)) {
                        return false;
                    }
                    break;
                case 5:
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    response.type = ControlResponseType::Stats;
                    if (!DecodeRecordingStats(raw, response.stats)) {
                        return false;
                    }
                    break;
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    bool DecodeCameraRequest(const std::vector<uint8_t>& data, ControlRequest& request)
    {
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    request.cameraName = std::move(value);
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    bool DecodeSetExposureRequest(const std::vector<uint8_t>& data, ControlRequest& request)
    {
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    request.cameraName = std::move(value);
                    break;
                }
                case 2: {
                    double value = 0.0;
                    if (!decoder.ReadFixed64(value)) {
                        return false;
                    }
                    request.value = value;
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    bool DecodeSetGainRequest(const std::vector<uint8_t>& data, ControlRequest& request)
    {
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    request.cameraName = std::move(value);
                    break;
                }
                case 2: {
                    double value = 0.0;
                    if (!decoder.ReadFixed64(value)) {
                        return false;
                    }
                    request.value = value;
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    bool DecodePreviewRequest(const std::vector<uint8_t>& data, ControlRequest& request)
    {
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::string value;
                    if (!decoder.ReadString(value)) {
                        return false;
                    }
                    request.cameraName = std::move(value);
                    break;
                }
                case 2: {
                    uint64_t value = 0;
                    if (!decoder.ReadVarint(value)) {
                        return false;
                    }
                    request.flag = value != 0;
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    bool DecodeCameraInfoList(const std::vector<uint8_t>& data, std::vector<CameraInfo>& cameras)
    {
        cameras.clear();
        Decoder decoder(data);
        uint32_t field = 0;
        uint32_t wireType = 0;
        while (decoder.NextField(field, wireType)) {
            switch (field) {
                case 1: {
                    std::vector<uint8_t> raw;
                    if (!decoder.ReadLengthDelimited(raw)) {
                        return false;
                    }
                    CameraInfo info;
                    if (!DecodeCameraInfo(raw, info)) {
                        return false;
                    }
                    cameras.push_back(std::move(info));
                    break;
                }
                default:
                    if (!decoder.Skip(wireType)) {
                        return false;
                    }
                    break;
            }
        }
        return decoder.Ok();
    }

    std::pair<std::string, uint16_t> SplitEndpoint(const std::string& endpoint, uint16_t defaultPort)
    {
        // Accepted forms: "host", "host:port", "[v6addr]", "[v6addr]:port".
        std::string host = endpoint;
        std::string portText;
        if (!endpoint.empty() && endpoint.front() == '[') {
            const auto closing = endpoint.find(']');
            if (closing == std::string::npos) {
                std::cerr << "Invalid endpoint '" << endpoint << "' (unterminated IPv6 address)\n";
                return {endpoint, 0U};
            }
            host = endpoint.substr(1, closing - 1);
            if (closing + 1 < endpoint.size()) {
                if (endpoint[closing + 1] != ':') {
                    std::cerr << "Invalid endpoint '" << endpoint << "'\n";
                    return {host, 0U};
                }
                portText = endpoint.substr(closing + 2);
            }
        } else {
            const auto separator = endpoint.rfind(':');
            // A bare IPv6 address without brackets has several ':'; treat it as host only.
            if (separator != std::string::npos && endpoint.find(':') == separator) {
                host = endpoint.substr(0, separator);
                portText = endpoint.substr(separator + 1);
            }
        }

        if (portText.empty()) {
            return {host, defaultPort};
        }
        try {
            size_t consumed = 0;
            const long port = std::stol(portText, &consumed);
            if (consumed != portText.size() || port < 1 || port > 65535) {
                throw std::out_of_range("port out of range");
            }
            return {host, static_cast<uint16_t>(port)};
        } catch (const std::exception&) {
            std::cerr << "Invalid port in endpoint '" << endpoint << "'\n";
            return {host, 0U};
        }
    }

    void SetSocketTimeouts(boost::asio::ip::tcp::socket& socket, std::chrono::milliseconds timeout)
    {
        if (!socket.is_open()) {
            return;
        }
        const auto handle = socket.native_handle();
#ifdef _WIN32
        const DWORD ms = static_cast<DWORD>(timeout.count());
        setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&ms), sizeof(ms));
        setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&ms), sizeof(ms));
#else
        timeval tv{};
        tv.tv_sec = static_cast<decltype(tv.tv_sec)>(timeout.count() / 1000);
        tv.tv_usec = static_cast<decltype(tv.tv_usec)>((timeout.count() % 1000) * 1000);
        setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
    }

    bool ConnectWithTimeout(boost::asio::io_context& io, boost::asio::ip::tcp::socket& socket,
                            const std::string& host, uint16_t port, std::chrono::milliseconds timeout)
    {
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver resolver(io);
        const auto endpoints = resolver.resolve(host, std::to_string(port), ec);
        if (ec) {
            std::cerr << "Resolve failed for " << host << ":" << port << ": " << ec.message() << "\n";
            return false;
        }

        boost::system::error_code result = boost::asio::error::would_block;
        bool completed = false;
        io.restart();
        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint&) {
                result = error;
                completed = true;
            });
        io.run_for(timeout);
        if (!completed) {
            // Timed out: cancel the pending connect and let its handler run so nothing dangles.
            socket.close(ec);
            io.run();
            result = boost::asio::error::timed_out;
        }
        io.restart();
        if (result) {
            std::cerr << "Connect to " << host << ":" << port << " failed: " << result.message() << "\n";
            socket.close(ec);
            return false;
        }
        return true;
    }

    void ShutdownAndClose(boost::asio::ip::tcp::socket& socket)
    {
        boost::system::error_code ec;
        // shutdown() is what wakes a thread blocked in a synchronous read/write on POSIX.
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket.close(ec);
    }

    bool WriteAll(boost::asio::ip::tcp::socket& socket, const void* data, size_t size)
    {
        try {
            boost::asio::write(socket, boost::asio::buffer(data, size));
            return true;
        } catch (...) {
            return false;
        }
    }

    bool ReadAll(boost::asio::ip::tcp::socket& socket, void* data, size_t size)
    {
        try {
            boost::asio::read(socket, boost::asio::buffer(data, size));
            return true;
        } catch (...) {
            return false;
        }
    }

    bool WriteFramed(boost::asio::ip::tcp::socket& socket, const std::vector<uint8_t>& payload)
    {
        const auto header = ToBigEndian32(static_cast<uint32_t>(payload.size()));
        std::vector<boost::asio::const_buffer> buffers;
        buffers.emplace_back(boost::asio::buffer(header.data(), header.size()));
        buffers.emplace_back(boost::asio::buffer(payload));
        try {
            boost::asio::write(socket, buffers);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool ReadFramed(boost::asio::ip::tcp::socket& socket, std::vector<uint8_t>& payload, size_t maxLength)
    {
        std::array<uint8_t, 4> header{};
        if (!ReadAll(socket, header.data(), header.size())) {
            return false;
        }
        const uint32_t length = FromBigEndian32(header.data());
        if (length > maxLength) {
            std::cerr << "Framed message of " << length << " bytes exceeds the limit of " << maxLength << "\n";
            return false;
        }
        payload.resize(length);
        if (length == 0) {
            return true;
        }
        return ReadAll(socket, payload.data(), payload.size());
    }

}  // namespace vidre::transport
