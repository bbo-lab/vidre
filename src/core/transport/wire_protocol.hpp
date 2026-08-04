#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#include <core/transport/transport.hpp>

namespace vidre::transport {

    struct CameraInfo {
        std::string cameraName;
        std::string serialNumber;
        std::string vendorName;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t frameBytes = 0;
        uint32_t bufferSize = 0;
        uint32_t maxNumBuffers = 0;
        double frameRate = 0.0;
        double exposure = 0.0;
        double gain = 0.0;
        uint64_t bitsPerPixel = 0;
        bool packed = false;
    };

    struct CameraStatus {
        int32_t recordingStatus = 0;
        bool armed = false;
        bool running = false;
        bool error = false;
    };

    struct RecordingStats {
        int64_t savedFrames = 0;
        int64_t droppedFrames = 0;
        int64_t sentFrames = 0;
    };

    struct StatusResponse {
        bool ok = false;
        std::string message;
    };

    enum class StorageCommandType {
        Frame,
        BeginSession,
        FinishSession
    };

    struct StorageCommand {
        StorageCommandType type = StorageCommandType::Frame;
        FrameEnvelope frame;
        SessionSpec beginSession;
        uint32_t finishFrameCount = 0;
    };

    enum class ControlRequestType {
        ListCameras,
        GetCameraInfo,
        StartRecording,
        StopRecording,
        StartStreaming,
        StopStreaming,
        SetExposure,
        SetGain,
        GetStatus,
        GetStats,
        SetPreview
    };

    struct ControlRequest {
        ControlRequestType type = ControlRequestType::ListCameras;
        std::string cameraName;
        double value = 0.0;
        bool flag = false;
    };

    enum class ControlResponseType {
        CameraList,
        CameraInfo,
        Status,
        CameraStatus,
        Stats
    };

    struct ControlResponse {
        ControlResponseType type = ControlResponseType::Status;
        std::vector<CameraInfo> cameras;
        CameraInfo cameraInfo;
        StatusResponse status;
        CameraStatus cameraStatus;
        RecordingStats stats;
    };

    std::vector<uint8_t> EncodeFrameEnvelope(const FrameEnvelope& envelope);
    bool DecodeFrameEnvelope(const std::vector<uint8_t>& data, FrameEnvelope& envelope);

    std::vector<uint8_t> EncodeSessionSpec(const SessionSpec& spec);
    bool DecodeSessionSpec(const std::vector<uint8_t>& data, SessionSpec& spec);

    std::vector<uint8_t> EncodeStorageCommand(const StorageCommand& command);
    bool DecodeStorageCommand(const std::vector<uint8_t>& data, StorageCommand& command);

    std::vector<uint8_t> EncodeCameraInfo(const CameraInfo& info);
    bool DecodeCameraInfo(const std::vector<uint8_t>& data, CameraInfo& info);

    std::vector<uint8_t> EncodeCameraStatus(const CameraStatus& status);
    bool DecodeCameraStatus(const std::vector<uint8_t>& data, CameraStatus& status);

    std::vector<uint8_t> EncodeRecordingStats(const RecordingStats& stats);
    bool DecodeRecordingStats(const std::vector<uint8_t>& data, RecordingStats& stats);

    std::vector<uint8_t> EncodeStatusResponse(const StatusResponse& response);
    bool DecodeStatusResponse(const std::vector<uint8_t>& data, StatusResponse& response);

    std::vector<uint8_t> EncodeControlRequest(const ControlRequest& request);
    bool DecodeControlRequest(const std::vector<uint8_t>& data, ControlRequest& request);

    std::vector<uint8_t> EncodeControlResponse(const ControlResponse& response);
    bool DecodeControlResponse(const std::vector<uint8_t>& data, ControlResponse& response);

    bool DecodeCameraRequest(const std::vector<uint8_t>& data, ControlRequest& request);
    bool DecodeSetExposureRequest(const std::vector<uint8_t>& data, ControlRequest& request);
    bool DecodeSetGainRequest(const std::vector<uint8_t>& data, ControlRequest& request);
    bool DecodePreviewRequest(const std::vector<uint8_t>& data, ControlRequest& request);
    bool DecodeCameraInfoList(const std::vector<uint8_t>& data, std::vector<CameraInfo>& cameras);

    std::pair<std::string, uint16_t> SplitEndpoint(const std::string& endpoint);
    bool WriteAll(boost::asio::ip::tcp::socket& socket, const void* data, size_t size);
    bool ReadAll(boost::asio::ip::tcp::socket& socket, void* data, size_t size);
    bool WriteFramed(boost::asio::ip::tcp::socket& socket, const std::vector<uint8_t>& payload);
    bool ReadFramed(boost::asio::ip::tcp::socket& socket, std::vector<uint8_t>& payload);

}  // namespace vidre::transport
