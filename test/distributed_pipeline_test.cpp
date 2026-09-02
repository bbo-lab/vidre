#include <core/runtime/iruntime_instance.hpp>
#include <core/transport/control_client.hpp>

#include "test_helpers.hpp"

#include <boost/process.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace bp = boost::process;

namespace {

int failures = 0;

#define CHECK(cond)                                                                            \
    do {                                                                                       \
        if (!(cond)) {                                                                         \
            std::cerr << "FAIL: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n";         \
            ++failures;                                                                        \
        }                                                                                      \
    } while (0)

class ChildGuard {
public:
    explicit ChildGuard(bp::child&& child) : _child(std::move(child)) {}
    ~ChildGuard() {
        if (_child.running()) {
            _child.terminate();
            _child.wait();
        }
    }
    ChildGuard(const ChildGuard&) = delete;
    ChildGuard& operator=(const ChildGuard&) = delete;

    bp::child& get() {
        return _child;
    }

private:
    bp::child _child;
};

bool isRecording(vidre::ControlClient& client, const std::string& cameraName) {
    return client.status(cameraName).recordingStatus == static_cast<int32_t>(vidre::RecordingStatus::RecordingFine);
}

bool isStopped(vidre::ControlClient& client, const std::string& cameraName) {
    return client.status(cameraName).recordingStatus == static_cast<int32_t>(vidre::RecordingStatus::Stopped);
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "usage: distributed_pipeline_test <vidre_camera_service> <vidre_storage_service>\n";
        return 2;
    }
    const std::string cameraService = argv[1];
    const std::string storageService = argv[2];

    vidre::test::TempDir dir;
    const auto configPath = dir.path() / "setup.yaml";
    const std::string yaml = "cam1:\n"
                             "  type: camera\n"
                             "  vendor: dummy\n"
                             "  width: 64\n"
                             "  height: 48\n"
                             "  frame_rate: 100\n"
                             "  buffers: 16\n"
                             "  serial: dist-serial-1\n";
    CHECK(vidre::test::writeFile(configPath, yaml));

    const uint16_t storagePort = vidre::test::findFreePort();
    ChildGuard storageChild(bp::child(
        storageService,
        "--listen-address", "127.0.0.1:" + std::to_string(storagePort),
        "--output-dir", dir.path().string(),
        bp::std_out > (dir.path() / "storage.out").string(),
        bp::std_err > (dir.path() / "storage.err").string()));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));  // let the storage service bind

    const uint16_t controlPort = vidre::test::findFreePort();
    const uint16_t previewPort = vidre::test::findFreePort();
    ChildGuard cameraChild(bp::child(
        cameraService,
        "--config", configPath.string(),
        "--control-address", "127.0.0.1:" + std::to_string(controlPort),
        "--preview-address", "127.0.0.1:" + std::to_string(previewPort),
        "--storage-endpoint", "127.0.0.1:" + std::to_string(storagePort),
        "--autostart",
        bp::std_out > (dir.path() / "camera.out").string(),
        bp::std_err > (dir.path() / "camera.err").string()));

    vidre::ControlClient client("127.0.0.1:" + std::to_string(controlPort));
    CHECK(vidre::test::pollUntil([&] { return client.connect(); }, std::chrono::seconds(15)));
    CHECK(client.connected());

    // Autostart begins recording immediately; wait until it is confirmed.
    CHECK(vidre::test::pollUntil([&] { return isRecording(client, "cam1"); }, std::chrono::seconds(15)));

    std::this_thread::sleep_for(std::chrono::seconds(2));
    CHECK(client.stats("cam1").savedFrames > 0);

    CHECK(client.stopRecording("cam1"));
    CHECK(vidre::test::pollUntil([&] { return isStopped(client, "cam1"); }, std::chrono::seconds(10)));

    // The camera service has delivered the whole session to the storage service.
    CHECK(vidre::test::waitForCcvFile(dir.path(), "", 1, std::chrono::seconds(10)));
    const auto files = vidre::test::findFiles(dir.path(), ".ccv");
    CHECK(files.size() == 1);

    vidre::test::CcvInfo info;
    CHECK(vidre::test::readCcvInfo(files[0], info));
    CHECK(info.width == 64);
    CHECK(info.height == 48);
    CHECK(vidre::test::finalizedFrames(files[0], info) > 0);

    cameraChild.get().terminate();
    cameraChild.get().wait();
    storageChild.get().terminate();
    storageChild.get().wait();

    if (failures != 0) {
        std::cerr << failures << " distributed_pipeline_test failure(s)\n";
        return 1;
    }
    std::cout << "distributed_pipeline_test: OK\n";
    return 0;
}
