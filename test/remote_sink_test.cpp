#include <core/runtime/shared_frame_header.hpp>
#include <core/transport/storage_server.hpp>
#include <core/transport/transport.hpp>

#include "test_helpers.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

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

vidre::FrameEnvelope makeBaseEnvelope() {
    vidre::FrameEnvelope base;
    base.cameraName = "rsink";
    base.serialNumber = "sn-rsink";
    base.vendorName = "dummy";
    base.width = kWidth;
    base.height = kHeight;
    base.bitsPerPixel = 8;
    return base;
}

vidre::SessionSpec makeSessionSpec() {
    vidre::SessionSpec spec;
    spec.cameraName = "rsink";
    spec.serialNumber = "sn-rsink";
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

void verifySessionFile(const std::filesystem::path& path, uint64_t expectedFrames) {
    vidre::test::CcvInfo info;
    CHECK(vidre::test::readCcvInfo(path, info));
    CHECK(info.width == kWidth);
    CHECK(info.height == kHeight);
#ifdef _WIN32
    CHECK(info.frameCount == static_cast<uint32_t>(expectedFrames));
#else
    CHECK(vidre::test::frameCountFromSize(std::filesystem::file_size(path), info) == expectedFrames);
#endif
    CHECK(vidre::test::checkFrameSequence(path, info, expectedFrames));
}

}  // namespace

int main() {
    vidre::test::TempDir dir;
    const auto baseEnvelope = makeBaseEnvelope();
    const auto sessionSpec = makeSessionSpec();
    std::vector<uint8_t> payload(kFrameBytes, 0x5A);

    // Happy path: begin -> 3 frames -> finish, all over real TCP.
    {
        const uint16_t port = vidre::test::findFreePort();
        vidre::StorageServer server("127.0.0.1:" + std::to_string(port), dir.path().string());
        server.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        {
            vidre::RemoteStorageSink sink("127.0.0.1:" + std::to_string(port), baseEnvelope, sessionSpec);
            sink.begin();

            vidre::SharedFrameHeader header;
            for (int i = 1; i <= 3; ++i) {
                header.frameID.store(i, std::memory_order_relaxed);
                header.timeStamp = static_cast<uint64_t>(i * 100);
                header.computerTimeStamp = i * 1000;
                sink.publishFrame(header, payload.data(), payload.size());
            }

            CHECK(vidre::test::pollUntil([&] { return sink.sentFrames() >= 4; }, std::chrono::seconds(5)));

            const auto start = std::chrono::steady_clock::now();
            sink.finish(3);  // returns once the queue is drained (server alive)
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
            CHECK(elapsed.count() < 4000);
            CHECK(sink.sentFrames() == 5);  // begin + 3 frames + finish
        }

        CHECK(vidre::test::waitForCcvFile(dir.path(), "rsink", 3, std::chrono::seconds(5)));
        const auto files = vidre::test::findFiles(dir.path(), ".ccv");
        CHECK(files.size() == 1);
        verifySessionFile(files[0], 3);

        server.stop();
    }

    // Hang regression: the server dies mid-session; finish() must return within
    // its 5 s timeout instead of blocking forever.
    {
        const uint16_t port = vidre::test::findFreePort();

        // Accept exactly one connection, then slam the door.
        std::thread killer([port] {
            try {
                boost::asio::io_context io;
                boost::asio::ip::tcp::acceptor acceptor(io,
                                                        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
                boost::asio::ip::tcp::socket socket(io);
                acceptor.accept(socket);
                socket.close();
                acceptor.close();
            } catch (...) {
            }
        });

        {
            vidre::RemoteStorageSink sink("127.0.0.1:" + std::to_string(port), baseEnvelope, sessionSpec);
            sink.begin();
            vidre::SharedFrameHeader header;
            header.frameID.store(1, std::memory_order_relaxed);
            header.timeStamp = 100;
            header.computerTimeStamp = 1000;
            sink.publishFrame(header, payload.data(), payload.size());

            std::this_thread::sleep_for(std::chrono::milliseconds(300));  // let the worker hit the dead peer
            killer.join();

            const auto start = std::chrono::steady_clock::now();
            sink.finish(1);  // must time out, not hang
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
            CHECK(elapsed.count() >= 4000);
            CHECK(elapsed.count() < 6000);
        }  // sink destructor joins the worker within ~1 s
    }

    if (failures != 0) {
        std::cerr << failures << " remote_sink_test failure(s)\n";
        return 1;
    }
    std::cout << "remote_sink_test: OK\n";
    return 0;
}
