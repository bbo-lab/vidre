#include <core/runtime/runtime.hpp>

#include "test_helpers.hpp"

#include <camera/dummy/dummy_camera.hpp>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>

namespace {

int failures = 0;

#define CHECK(cond)                                                                            \
    do {                                                                                       \
        if (!(cond)) {                                                                         \
            std::cerr << "FAIL: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n";         \
            ++failures;                                                                        \
        }                                                                                      \
    } while (0)

}  // namespace

int main() {
    vidre::test::TempDir dir;
    const auto requestedFile = dir.path() / "roundtrip.ccv";

    int savedFrames = 0;
    std::string outputFile;

    {
        auto camera = std::make_unique<vidre::DummyCamera>();
        camera->SetMaxNumBuffer(16);
        camera->SetFrameWidth(64);
        camera->SetFrameHeight(48);
        camera->SetFrameRate(100.0);
        camera->Init(vidre::test::uniqueId("ccv_roundtrip_serial"));

        vidre::RuntimeInstance runtime;
        runtime.setCamera(std::move(camera));
        runtime.setCameraName("ccvtest");
        runtime.createStorage(requestedFile.string(), "ccv", 8, 0);
        CHECK(runtime.checkCamera());
        CHECK(!std::filesystem::exists(requestedFile));

        runtime.run();
        std::this_thread::sleep_for(std::chrono::milliseconds(600));

        savedFrames = runtime.getSavedFramesNumber();
        CHECK(savedFrames > 0);

        outputFile = runtime.getOutputFileName();
        CHECK(!outputFile.empty());
        CHECK(outputFile != requestedFile.string());
        CHECK(std::filesystem::path(outputFile).filename().string().find("roundtrip.ccv") != std::string::npos);
        CHECK(std::filesystem::exists(outputFile));
        CHECK(!std::filesystem::exists(requestedFile));

        runtime.stop();
    }  // runtime destroyed -> DiskWriter flushed on POSIX

    vidre::test::CcvInfo info;
    CHECK(vidre::test::readCcvInfo(outputFile, info));
    CHECK(info.width == 64);
    CHECK(info.height == 48);
    CHECK(info.frameBytesOnDisk == 64 * 48 + 21);

    const auto fileSize = std::filesystem::file_size(outputFile);
    const uint64_t framesInFile = vidre::test::finalizedFrames(outputFile, info);
    CHECK(framesInFile > 0);
#ifdef _WIN32
    // On Windows the header is rewritten with the final frame count, which must
    // be consistent with the number of frames physically present in the file.
    CHECK(info.frameCount == static_cast<uint32_t>(vidre::test::frameCountFromSize(fileSize, info)));
#endif
    CHECK(vidre::test::checkFrameSequence(outputFile, info, framesInFile));

    if (failures != 0) {
        std::cerr << failures << " ccv_roundtrip_test failure(s)\n";
        return 1;
    }
    std::cout << "ccv_roundtrip_test: OK\n";
    return 0;
}
