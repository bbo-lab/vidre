#include <core/transport/storage_server.hpp>
#include <core/transport/wire_protocol.hpp>

#include "test_helpers.hpp"

#include <boost/asio.hpp>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace asio = boost::asio;

namespace {

int failures = 0;

#define CHECK(cond)                                                                            \
    do {                                                                                       \
        if (!(cond)) {                                                                         \
            std::cerr << "FAIL: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n";         \
            ++failures;                                                                        \
        }                                                                                      \
    } while (0)

constexpr uint32_t kWidth = 64;
constexpr uint32_t kHeight = 48;
constexpr uint32_t kFrameBytes = kWidth * kHeight;

vidre::SessionSpec makeSessionSpec(const std::string& cameraName) {
    vidre::SessionSpec spec;
    spec.cameraName = cameraName;
    spec.serialNumber = "sn-" + cameraName;
    spec.vendorName = "dummy";
    spec.width = kWidth;
    spec.height = kHeight;
    spec.bitsPerPixel = 8;
    spec.packed = false;
    spec.frameBytesActual = kFrameBytes;
    spec.frameBytesBuffer = kFrameBytes;
    spec.maxNumBuffer = 8;
    spec.frameRate = 0.0;
    spec.outputFile = "";
    spec.headerFormat = "ccv";
    return spec;
}

bool connectTo(asio::ip::tcp::socket& socket, asio::io_context& io, uint16_t port) {
    try {
        asio::ip::tcp::resolver resolver(io);
        const auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool sendCommand(asio::ip::tcp::socket& socket, const vidre::transport::StorageCommand& command) {
    return vidre::transport::WriteFramed(socket, vidre::transport::EncodeStorageCommand(command));
}

void verifySessionFile(const std::filesystem::path& path, const std::vector<uint32_t>& expectedIndices) {
    const uint64_t expectedFrames = expectedIndices.size();
    vidre::test::CcvInfo info;
    CHECK(vidre::test::readCcvInfo(path, info));
    CHECK(info.width == kWidth);
    CHECK(info.height == kHeight);
    CHECK(info.frameBytesOnDisk == kFrameBytes + 21);
#ifdef _WIN32
    CHECK(info.frameCount == static_cast<uint32_t>(expectedFrames));
#else
    CHECK(vidre::test::frameCountFromSize(std::filesystem::file_size(path), info) == expectedFrames);
#endif
    const auto indices = vidre::test::readFrameIndices(path, info, expectedFrames);
    CHECK(indices == expectedIndices);
    if (indices != expectedIndices) {
        std::cerr << "  stored frame indices:";
        for (const auto index : indices) {
            std::cerr << " " << index;
        }
        std::cerr << "\n";
    }
}

}  // namespace

int main() {
    vidre::test::TempDir dir;
    const uint16_t port = vidre::test::findFreePort();

    std::vector<std::string> logs;
    vidre::StorageServer server("127.0.0.1:" + std::to_string(port), dir.path().string(),
                                [&logs](const std::string& message) { logs.push_back(message); });
    server.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    asio::io_context io;

    // Session A: BeginSession -> frames -> FinishSession.
    {
        asio::ip::tcp::socket socket(io);
        CHECK(connectTo(socket, io, port));

        vidre::transport::StorageCommand begin;
        begin.type = vidre::transport::StorageCommandType::BeginSession;
        begin.beginSession = makeSessionSpec("sess-a");
        CHECK(sendCommand(socket, begin));

        std::vector<uint8_t> payload(kFrameBytes, 0x11);
        for (uint32_t i = 1; i <= 5; ++i) {
            vidre::transport::StorageCommand frame;
            frame.type = vidre::transport::StorageCommandType::Frame;
            frame.frame.cameraName = "sess-a";
            frame.frame.serialNumber = "sn-sess-a";
            frame.frame.vendorName = "dummy";
            frame.frame.width = kWidth;
            frame.frame.height = kHeight;
            frame.frame.bitsPerPixel = 8;
            frame.frame.frameId = i;
            frame.frame.frameTimestamp = 1000 + i;
            frame.frame.computerTimeStamp = 2000 + i;
            frame.frame.payload = payload;
            CHECK(sendCommand(socket, frame));
        }

        vidre::transport::StorageCommand finish;
        finish.type = vidre::transport::StorageCommandType::FinishSession;
        finish.finishFrameCount = 5;
        CHECK(sendCommand(socket, finish));
        socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket.close();

        CHECK(vidre::test::waitForCcvFile(dir.path(), "sess-a", 5, std::chrono::seconds(5)));
        const auto files = vidre::test::findFiles(dir.path(), ".ccv");
        CHECK(files.size() == 1);
        verifySessionFile(files[0], {1, 2, 3, 4, 5});
    }

    // Session B: BeginSession -> frames with a gap -> disconnect without finish.
    // The server must finalize from its own received-frame counter.
    {
        asio::ip::tcp::socket socket(io);
        CHECK(connectTo(socket, io, port));

        vidre::transport::StorageCommand begin;
        begin.type = vidre::transport::StorageCommandType::BeginSession;
        begin.beginSession = makeSessionSpec("sess-b");
        CHECK(sendCommand(socket, begin));

        std::vector<uint8_t> payload(kFrameBytes, 0x22);
        const std::vector<uint64_t> ids = {1, 5};  // gap of 3 dropped
        for (const auto id : ids) {
            vidre::transport::StorageCommand frame;
            frame.type = vidre::transport::StorageCommandType::Frame;
            frame.frame.cameraName = "sess-b";
            frame.frame.serialNumber = "sn-sess-b";
            frame.frame.vendorName = "dummy";
            frame.frame.width = kWidth;
            frame.frame.height = kHeight;
            frame.frame.bitsPerPixel = 8;
            frame.frame.frameId = id;
            frame.frame.frameTimestamp = 1000 + id;
            frame.frame.computerTimeStamp = 2000 + id;
            frame.frame.payload = payload;
            CHECK(sendCommand(socket, frame));
        }

        // Abrupt disconnect: no FinishSession is sent.
        socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket.close();

        CHECK(vidre::test::waitForCcvFile(dir.path(), "sess-b", 2, std::chrono::seconds(5)));
        const auto files = vidre::test::findFiles(dir.path(), ".ccv");
        CHECK(files.size() == 2);

        const auto fileB = std::find_if(files.begin(), files.end(), [](const auto& path) {
            return path.filename().string().find("sess-b") != std::string::npos;
        });
        CHECK(fileB != files.end());
        verifySessionFile(*fileB, {1, 5});

        // The server's own drop accounting shows the gap.
        bool sawGap = false;
        for (const auto& entry : logs) {
            if (entry.find("sess-b") != std::string::npos && entry.find("3 dropped") != std::string::npos) {
                sawGap = true;
            }
        }
        CHECK(sawGap);
    }

    server.stop();

    if (failures != 0) {
        std::cerr << failures << " storage_server_session_test failure(s)\n";
        return 1;
    }
    std::cout << "storage_server_session_test: OK\n";
    return 0;
}
