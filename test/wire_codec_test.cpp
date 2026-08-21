#include <core/transport/wire_protocol.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace wt = vidre::transport;

namespace {

int failures = 0;

#define CHECK(cond)                                                                              \
    do {                                                                                         \
        if (!(cond)) {                                                                           \
            std::cerr << "FAIL: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n";           \
            ++failures;                                                                          \
        }                                                                                        \
    } while (0)

#define CHECK_NO_THROW(expr)                                                                     \
    do {                                                                                         \
        try {                                                                                    \
            (void)(expr);                                                                        \
        } catch (const std::exception& e) {                                                      \
            std::cerr << "FAIL: " #expr " threw " << e.what() << " at " << __FILE__ << ":"       \
                      << __LINE__ << "\n";                                                       \
            ++failures;                                                                          \
        } catch (...) {                                                                          \
            std::cerr << "FAIL: " #expr " threw at " << __FILE__ << ":" << __LINE__ << "\n";     \
            ++failures;                                                                          \
        }                                                                                        \
    } while (0)

vidre::FrameEnvelope makeEnvelope() {
    vidre::FrameEnvelope envelope;
    envelope.cameraName = "cam-a";
    envelope.serialNumber = "SN-123";
    envelope.vendorName = "dummy";
    envelope.width = 64;
    envelope.height = 48;
    envelope.bitsPerPixel = 8;
    envelope.frameId = 42;
    envelope.frameTimestamp = 1000;
    envelope.computerTimeStamp = -7;
    envelope.payload = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    return envelope;
}

bool sameEnvelope(const vidre::FrameEnvelope& a, const vidre::FrameEnvelope& b) {
    return a.cameraName == b.cameraName && a.serialNumber == b.serialNumber && a.vendorName == b.vendorName &&
           a.width == b.width && a.height == b.height && a.bitsPerPixel == b.bitsPerPixel && a.frameId == b.frameId &&
           a.frameTimestamp == b.frameTimestamp && a.computerTimeStamp == b.computerTimeStamp && a.payload == b.payload;
}

vidre::SessionSpec makeSessionSpec() {
    vidre::SessionSpec spec;
    spec.cameraName = "cam-a";
    spec.serialNumber = "SN-123";
    spec.vendorName = "dummy";
    spec.width = 64;
    spec.height = 48;
    spec.bitsPerPixel = 16;
    spec.packed = true;
    spec.frameBytesActual = 6144;
    spec.frameBytesBuffer = 8192;
    spec.maxNumBuffer = 8;
    spec.frameRate = 120.5;
    spec.outputFile = "out/run.ccv";
    spec.headerFormat = "ccv";
    return spec;
}

bool sameSessionSpec(const vidre::SessionSpec& a, const vidre::SessionSpec& b) {
    return a.cameraName == b.cameraName && a.serialNumber == b.serialNumber && a.vendorName == b.vendorName &&
           a.width == b.width && a.height == b.height && a.bitsPerPixel == b.bitsPerPixel && a.packed == b.packed &&
           a.frameBytesActual == b.frameBytesActual && a.frameBytesBuffer == b.frameBytesBuffer &&
           a.maxNumBuffer == b.maxNumBuffer && a.frameRate == b.frameRate && a.outputFile == b.outputFile &&
           a.headerFormat == b.headerFormat;
}

wt::CameraInfo makeCameraInfo() {
    wt::CameraInfo info;
    info.cameraName = "cam-a";
    info.serialNumber = "SN-1";
    info.vendorName = "dummy";
    info.width = 640;
    info.height = 480;
    info.frameBytes = 307200;
    info.bufferSize = 307200;
    info.maxNumBuffers = 500;
    info.frameRate = 60.0;
    info.exposure = 12.5;
    info.gain = 3.25;
    info.bitsPerPixel = 8;
    info.packed = true;
    return info;
}

bool sameCameraInfo(const wt::CameraInfo& a, const wt::CameraInfo& b) {
    return a.cameraName == b.cameraName && a.serialNumber == b.serialNumber && a.vendorName == b.vendorName &&
           a.width == b.width && a.height == b.height && a.frameBytes == b.frameBytes && a.bufferSize == b.bufferSize &&
           a.maxNumBuffers == b.maxNumBuffers && a.frameRate == b.frameRate && a.exposure == b.exposure &&
           a.gain == b.gain && a.bitsPerPixel == b.bitsPerPixel && a.packed == b.packed;
}

bool sameCameraStatus(const wt::CameraStatus& a, const wt::CameraStatus& b) {
    return a.recordingStatus == b.recordingStatus && a.armed == b.armed && a.running == b.running && a.error == b.error;
}

bool sameStats(const wt::RecordingStats& a, const wt::RecordingStats& b) {
    return a.savedFrames == b.savedFrames && a.droppedFrames == b.droppedFrames && a.sentFrames == b.sentFrames;
}

bool sameStatusResponse(const wt::StatusResponse& a, const wt::StatusResponse& b) {
    return a.ok == b.ok && a.message == b.message;
}

bool sameControlResponse(const wt::ControlResponse& a, const wt::ControlResponse& b) {
    if (a.type != b.type || a.cameras.size() != b.cameras.size()) {
        return false;
    }
    for (size_t i = 0; i < a.cameras.size(); ++i) {
        if (!sameCameraInfo(a.cameras[i], b.cameras[i])) {
            return false;
        }
    }
    return sameCameraInfo(a.cameraInfo, b.cameraInfo) && sameStatusResponse(a.status, b.status) &&
           sameCameraStatus(a.cameraStatus, b.cameraStatus) && sameStats(a.stats, b.stats);
}

bool sameStorageCommand(const wt::StorageCommand& a, const wt::StorageCommand& b) {
    if (a.type != b.type) {
        return false;
    }
    switch (a.type) {
        case wt::StorageCommandType::Frame:
            return sameEnvelope(a.frame, b.frame);
        case wt::StorageCommandType::BeginSession:
            return sameSessionSpec(a.beginSession, b.beginSession);
        case wt::StorageCommandType::FinishSession:
            return a.finishFrameCount == b.finishFrameCount;
    }
    return false;
}

void roundTripFrameEnvelope() {
    auto envelope = makeEnvelope();
    vidre::FrameEnvelope decoded;
    CHECK(wt::DecodeFrameEnvelope(wt::EncodeFrameEnvelope(envelope), decoded));
    CHECK(sameEnvelope(envelope, decoded));

    vidre::FrameEnvelope emptyPayload;
    emptyPayload.payload.clear();
    CHECK(wt::DecodeFrameEnvelope(wt::EncodeFrameEnvelope(emptyPayload), decoded));
    CHECK(sameEnvelope(emptyPayload, decoded));
}

void roundTripSessionSpec() {
    auto spec = makeSessionSpec();
    vidre::SessionSpec decoded;
    CHECK(wt::DecodeSessionSpec(wt::EncodeSessionSpec(spec), decoded));
    CHECK(sameSessionSpec(spec, decoded));
}

void roundTripStorageCommand() {
    wt::StorageCommand begin;
    begin.type = wt::StorageCommandType::BeginSession;
    begin.beginSession = makeSessionSpec();
    wt::StorageCommand decoded;
    CHECK(wt::DecodeStorageCommand(wt::EncodeStorageCommand(begin), decoded));
    CHECK(sameStorageCommand(begin, decoded));

    wt::StorageCommand frame;
    frame.type = wt::StorageCommandType::Frame;
    frame.frame = makeEnvelope();
    CHECK(wt::DecodeStorageCommand(wt::EncodeStorageCommand(frame), decoded));
    CHECK(sameStorageCommand(frame, decoded));

    wt::StorageCommand finish;
    finish.type = wt::StorageCommandType::FinishSession;
    finish.finishFrameCount = 123;
    CHECK(wt::DecodeStorageCommand(wt::EncodeStorageCommand(finish), decoded));
    CHECK(sameStorageCommand(finish, decoded));
}

void roundTripCameraInfo() {
    auto info = makeCameraInfo();
    wt::CameraInfo decoded;
    CHECK(wt::DecodeCameraInfo(wt::EncodeCameraInfo(info), decoded));
    CHECK(sameCameraInfo(info, decoded));
}

void roundTripCameraStatus() {
    wt::CameraStatus status;
    status.recordingStatus = 2;
    status.armed = true;
    status.running = true;
    status.error = false;
    wt::CameraStatus decoded;
    CHECK(wt::DecodeCameraStatus(wt::EncodeCameraStatus(status), decoded));
    CHECK(sameCameraStatus(status, decoded));
}

void roundTripRecordingStats() {
    wt::RecordingStats stats;
    stats.savedFrames = 100;
    stats.droppedFrames = 3;
    stats.sentFrames = 97;
    wt::RecordingStats decoded;
    CHECK(wt::DecodeRecordingStats(wt::EncodeRecordingStats(stats), decoded));
    CHECK(sameStats(stats, decoded));
}

void roundTripStatusResponse() {
    wt::StatusResponse response;
    response.ok = true;
    response.message = "started cam-a";
    wt::StatusResponse decoded;
    CHECK(wt::DecodeStatusResponse(wt::EncodeStatusResponse(response), decoded));
    CHECK(sameStatusResponse(response, decoded));
}

void roundTripControlRequest() {
    const std::vector<wt::ControlRequestType> types = {
        wt::ControlRequestType::ListCameras,
        wt::ControlRequestType::GetCameraInfo,
        wt::ControlRequestType::StartRecording,
        wt::ControlRequestType::StopRecording,
        wt::ControlRequestType::StartStreaming,
        wt::ControlRequestType::StopStreaming,
        wt::ControlRequestType::SetExposure,
        wt::ControlRequestType::SetGain,
        wt::ControlRequestType::GetStatus,
        wt::ControlRequestType::GetStats,
        wt::ControlRequestType::SetPreview,
    };
    for (const auto type : types) {
        wt::ControlRequest request;
        request.type = type;
        request.cameraName = "cam-a";
        request.value = 3.5;
        request.flag = true;
        wt::ControlRequest decoded;
        CHECK(wt::DecodeControlRequest(wt::EncodeControlRequest(request), decoded));
        CHECK(decoded.type == type);
        // The encoder only serializes the fields each request type carries.
        if (type != wt::ControlRequestType::ListCameras) {
            CHECK(decoded.cameraName == "cam-a");
        }
        if (type == wt::ControlRequestType::SetExposure || type == wt::ControlRequestType::SetGain) {
            CHECK(decoded.value == 3.5);
        }
        if (type == wt::ControlRequestType::SetPreview) {
            CHECK(decoded.flag);
        }
    }
}

void roundTripControlResponse() {
    wt::ControlResponse cameraList;
    cameraList.type = wt::ControlResponseType::CameraList;
    cameraList.cameras.push_back(makeCameraInfo());
    cameraList.cameras.push_back(makeCameraInfo());
    cameraList.cameras.back().cameraName = "cam-b";
    cameraList.cameras.back().serialNumber = "SN-2";
    wt::ControlResponse decoded;
    CHECK(wt::DecodeControlResponse(wt::EncodeControlResponse(cameraList), decoded));
    CHECK(sameControlResponse(cameraList, decoded));

    wt::ControlResponse cameraInfo;
    cameraInfo.type = wt::ControlResponseType::CameraInfo;
    cameraInfo.cameraInfo = makeCameraInfo();
    CHECK(wt::DecodeControlResponse(wt::EncodeControlResponse(cameraInfo), decoded));
    CHECK(sameControlResponse(cameraInfo, decoded));

    wt::ControlResponse status;
    status.type = wt::ControlResponseType::Status;
    status.status.ok = true;
    status.status.message = "ok";
    CHECK(wt::DecodeControlResponse(wt::EncodeControlResponse(status), decoded));
    CHECK(sameControlResponse(status, decoded));

    wt::ControlResponse cameraStatus;
    cameraStatus.type = wt::ControlResponseType::CameraStatus;
    cameraStatus.cameraStatus.recordingStatus = 2;
    cameraStatus.cameraStatus.armed = true;
    CHECK(wt::DecodeControlResponse(wt::EncodeControlResponse(cameraStatus), decoded));
    CHECK(sameControlResponse(cameraStatus, decoded));

    wt::ControlResponse stats;
    stats.type = wt::ControlResponseType::Stats;
    stats.stats.savedFrames = 5;
    stats.stats.droppedFrames = 1;
    CHECK(wt::DecodeControlResponse(wt::EncodeControlResponse(stats), decoded));
    CHECK(sameControlResponse(stats, decoded));
}

void roundTripSpecialDecoders() {
    // The legacy per-camera decoders are fed by the request sub-messages.
    wt::ControlRequest request;
    request.type = wt::ControlRequestType::GetCameraInfo;
    request.cameraName = "cam-a";
    wt::ControlRequest decoded;
    CHECK(wt::DecodeCameraRequest(wt::EncodeControlRequest(request), decoded));
    CHECK(decoded.cameraName == "cam-a");

    request.type = wt::ControlRequestType::SetExposure;
    request.value = 12.5;
    CHECK(wt::DecodeSetExposureRequest(wt::EncodeControlRequest(request), decoded));
    CHECK(decoded.cameraName == "cam-a" && decoded.value == 12.5);

    request.type = wt::ControlRequestType::SetGain;
    request.value = -3.25;
    CHECK(wt::DecodeSetGainRequest(wt::EncodeControlRequest(request), decoded));
    CHECK(decoded.cameraName == "cam-a" && decoded.value == -3.25);

    request.type = wt::ControlRequestType::SetPreview;
    request.flag = true;
    CHECK(wt::DecodePreviewRequest(wt::EncodeControlRequest(request), decoded));
    CHECK(decoded.cameraName == "cam-a" && decoded.flag);

    // The camera list decoder consumes the CameraList control response.
    wt::ControlResponse cameraList;
    cameraList.type = wt::ControlResponseType::CameraList;
    cameraList.cameras.push_back(makeCameraInfo());
    std::vector<wt::CameraInfo> cameras;
    CHECK(wt::DecodeCameraInfoList(wt::EncodeControlResponse(cameraList), cameras));
    CHECK(cameras.size() == 1);
    CHECK(sameCameraInfo(cameras[0], cameraList.cameras[0]));
}

void splitEndpointEdgeCases() {
    const auto host1 = wt::SplitEndpoint("127.0.0.1:1234");
    CHECK(host1.first == "127.0.0.1" && host1.second == 1234);

    const auto noPort = wt::SplitEndpoint("myhost");
    CHECK(noPort.first == "myhost" && noPort.second == 50051);

    const auto empty = wt::SplitEndpoint("");
    CHECK(empty.first == "" && empty.second == 50051);

    const auto leadingColon = wt::SplitEndpoint(":1234");
    CHECK(leadingColon.first == ":1234" && leadingColon.second == 50051);

    const auto trailingColon = wt::SplitEndpoint("myhost:");
    CHECK(trailingColon.first == "myhost:" && trailingColon.second == 50051);

    const auto garbagePort = wt::SplitEndpoint("myhost:abc");
    CHECK(garbagePort.first == "myhost" && garbagePort.second == 0);

    const auto zeroPort = wt::SplitEndpoint("myhost:0");
    CHECK(zeroPort.first == "myhost" && zeroPort.second == 0);

    const auto ipv6 = wt::SplitEndpoint("::1:8080");
    CHECK(ipv6.first == "::1" && ipv6.second == 8080);

    const auto doubleColon = wt::SplitEndpoint("myhost:1234:5678");
    CHECK(doubleColon.first == "myhost:1234" && doubleColon.second == 5678);
}

void corruptAndTruncatedInput() {
    const auto envelope = wt::EncodeFrameEnvelope(makeEnvelope());
    const auto spec = wt::EncodeSessionSpec(makeSessionSpec());
    const auto command = wt::EncodeStorageCommand([] {
        wt::StorageCommand frame;
        frame.type = wt::StorageCommandType::Frame;
        frame.frame = makeEnvelope();
        return frame;
    }());
    const auto request = wt::EncodeControlRequest([] {
        wt::ControlRequest r;
        r.type = wt::ControlRequestType::StartRecording;
        r.cameraName = "cam-a";
        return r;
    }());
    const auto response = wt::EncodeControlResponse([] {
        wt::ControlResponse r;
        r.type = wt::ControlResponseType::CameraStatus;
        r.cameraStatus.recordingStatus = 2;
        return r;
    }());

    // Every prefix truncation must decode without crashing; some are valid at
    // field boundaries, but none may throw or read out of bounds.
    for (size_t cut = 0; cut <= envelope.size(); ++cut) {
        const std::vector<uint8_t> truncated(envelope.begin(), envelope.begin() + static_cast<long>(cut));
        vidre::FrameEnvelope env;
        CHECK_NO_THROW(wt::DecodeFrameEnvelope(truncated, env));
    }
    for (size_t cut = 0; cut <= spec.size(); ++cut) {
        const std::vector<uint8_t> truncated(spec.begin(), spec.begin() + static_cast<long>(cut));
        vidre::SessionSpec decoded;
        CHECK_NO_THROW(wt::DecodeSessionSpec(truncated, decoded));
    }
    for (size_t cut = 0; cut <= command.size(); ++cut) {
        const std::vector<uint8_t> truncated(command.begin(), command.begin() + static_cast<long>(cut));
        wt::StorageCommand decoded;
        CHECK_NO_THROW(wt::DecodeStorageCommand(truncated, decoded));
    }
    for (size_t cut = 0; cut <= request.size(); ++cut) {
        const std::vector<uint8_t> truncated(request.begin(), request.begin() + static_cast<long>(cut));
        wt::ControlRequest decoded;
        CHECK_NO_THROW(wt::DecodeControlRequest(truncated, decoded));
    }
    for (size_t cut = 0; cut <= response.size(); ++cut) {
        const std::vector<uint8_t> truncated(response.begin(), response.begin() + static_cast<long>(cut));
        wt::ControlResponse decoded;
        CHECK_NO_THROW(wt::DecodeControlResponse(truncated, decoded));
    }

    // Random garbage must never crash or hang either.
    for (int seed = 0; seed < 16; ++seed) {
        std::vector<uint8_t> garbage(128 + seed * 13);
        uint64_t state = static_cast<uint64_t>(seed) + 0x9E3779B97F4A7C15ULL;
        for (auto& byte : garbage) {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            byte = static_cast<uint8_t>(state >> 33);
        }
        vidre::FrameEnvelope env;
        vidre::SessionSpec decodedSpec;
        wt::StorageCommand decodedCommand;
        wt::ControlRequest decodedRequest;
        wt::ControlResponse decodedResponse;
        CHECK_NO_THROW(wt::DecodeFrameEnvelope(garbage, env));
        CHECK_NO_THROW(wt::DecodeSessionSpec(garbage, decodedSpec));
        CHECK_NO_THROW(wt::DecodeStorageCommand(garbage, decodedCommand));
        CHECK_NO_THROW(wt::DecodeControlRequest(garbage, decodedRequest));
        CHECK_NO_THROW(wt::DecodeControlResponse(garbage, decodedResponse));
    }
}

}  // namespace

int main() {
    roundTripFrameEnvelope();
    roundTripSessionSpec();
    roundTripStorageCommand();
    roundTripCameraInfo();
    roundTripCameraStatus();
    roundTripRecordingStats();
    roundTripStatusResponse();
    roundTripControlRequest();
    roundTripControlResponse();
    roundTripSpecialDecoders();
    splitEndpointEdgeCases();
    corruptAndTruncatedInput();

    if (failures != 0) {
        std::cerr << failures << " wire_codec_test failure(s)\n";
        return 1;
    }
    std::cout << "wire_codec_test: OK\n";
    return 0;
}
