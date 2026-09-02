#include <core/runtime/iruntime_instance.hpp>
#include <core/transport/control_client.hpp>
#include <core/transport/preview_client.hpp>
#include <core/transport/storage_server.hpp>

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
    if (argc < 2) {
        std::cerr << "usage: camera_service_e2e_test <vidre_camera_service>\n";
        return 2;
    }
    const std::string cameraService = argv[1];

    vidre::test::TempDir dir;
    const auto configPath = dir.path() / "setup.yaml";
    const std::string yaml = "cam1:\n"
                             "  type: camera\n"
                             "  vendor: dummy\n"
                             "  width: 64\n"
                             "  height: 48\n"
                             "  frame_rate: 100\n"
                             "  buffers: 16\n"
                             "  serial: e2e-serial-1\n";
    CHECK(vidre::test::writeFile(configPath, yaml));

    const uint16_t storagePort = vidre::test::findFreePort();
    vidre::StorageServer storage("127.0.0.1:" + std::to_string(storagePort), dir.path().string());
    storage.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const uint16_t controlPort = vidre::test::findFreePort();
    const uint16_t previewPort = vidre::test::findFreePort();

    ChildGuard service(bp::child(
        cameraService,
        "--config", configPath.string(),
        "--control-address", "127.0.0.1:" + std::to_string(controlPort),
        "--preview-address", "127.0.0.1:" + std::to_string(previewPort),
        "--storage-endpoint", "127.0.0.1:" + std::to_string(storagePort),
        bp::std_out > (dir.path() / "service.out").string(),
        bp::std_err > (dir.path() / "service.err").string()));

    vidre::ControlClient client("127.0.0.1:" + std::to_string(controlPort));
    CHECK(vidre::test::pollUntil([&] { return client.connect(); }, std::chrono::seconds(15)));
    CHECK(client.connected());

    std::vector<vidre::transport::CameraInfo> cameras;
    CHECK(client.listCameras(cameras));
    CHECK(cameras.size() == 1);
    CHECK(cameras[0].cameraName == "cam1");
    CHECK(cameras[0].width == 64);
    CHECK(cameras[0].height == 48);

    CHECK(client.startRecording("cam1"));
    CHECK(vidre::test::pollUntil([&] { return isRecording(client, "cam1"); }, std::chrono::seconds(15)));

    std::this_thread::sleep_for(std::chrono::seconds(2));
    CHECK(client.stats("cam1").savedFrames > 0);

    // Preview: request it over control, then receive frames on the preview port.
    CHECK(client.setPreview("cam1", true));
    vidre::PreviewClient preview("127.0.0.1", previewPort, "cam1");
    preview.start();
    CHECK(vidre::test::pollUntil(
        [&] {
            vidre::FrameEnvelope envelope;
            return preview.latest(envelope) && envelope.frameId > 0;
        },
        std::chrono::seconds(10)));
    preview.stop();
    CHECK(client.setPreview("cam1", false));

    CHECK(client.stopRecording("cam1"));
    CHECK(vidre::test::pollUntil([&] { return isStopped(client, "cam1"); }, std::chrono::seconds(10)));

    // Stop the service; the storage server must have finalized the session.
    service.get().terminate();
    service.get().wait();

    CHECK(vidre::test::waitForCcvFile(dir.path(), "", 1, std::chrono::seconds(5)));
    const auto files = vidre::test::findFiles(dir.path(), ".ccv");
    CHECK(files.size() == 1);

    vidre::test::CcvInfo info;
    CHECK(vidre::test::readCcvInfo(files[0], info));
    CHECK(info.width == 64);
    CHECK(info.height == 48);
    CHECK(vidre::test::finalizedFrames(files[0], info) > 0);

    storage.stop();

    if (failures != 0) {
        std::cerr << failures << " camera_service_e2e_test failure(s)\n";
        return 1;
    }
    std::cout << "camera_service_e2e_test: OK\n";
    return 0;
}
