#include <core/runtime/runtime.hpp>
#include <utils/config_loader/config_loader.hpp>

#include "test_helpers.hpp"

#include <camera/camera.hpp>

#include <iostream>
#include <string>
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

}  // namespace

int main() {
    vidre::test::TempDir dir;
    const auto configPath = dir.path() / "setup.yaml";
    const auto outputFile = dir.path() / "out.ccv";

    std::vector<std::string> logs;
    vidre::LoaderLogCallback log = [&logs](const std::string& message) { logs.push_back(message); };

    // Full dummy camera with a storage block.
    {
        const std::string yaml = "cam1:\n"
                                 "  type: camera\n"
                                 "  vendor: dummy\n"
                                 "  width: 64\n"
                                 "  height: 48\n"
                                 "  frame_rate: 120\n"
                                 "  buffers: 16\n"
                                 "  serial: dummy-serial-1\n"
                                 "  storage:\n"
                                 "    output_file: \"" +
                                 vidre::test::yamlEscapePath(outputFile) + "\"\n"
                                 "    header: ccv\n"
                                 "    bits_per_pixel: 16\n";
        CHECK(vidre::test::writeFile(configPath, yaml));

        auto cameras = vidre::LoadSystemSetup(configPath.string(), {}, log);
        CHECK(cameras.size() == 1);
        CHECK(cameras.count("cam1") == 1);
        auto* runtime = cameras["cam1"].get();
        CHECK(runtime != nullptr);
        CHECK(runtime->getFrameWidth() == 64);
        CHECK(runtime->getFrameHeight() == 48);
        CHECK(runtime->getBitsPerPixel() == 16);
        CHECK(runtime->getCamera() != nullptr);
        CHECK(runtime->getCamera()->GetSerialNumber() == "dummy-serial-1");
        CHECK(runtime->getOutputFileName() == outputFile.string());

        bool sawCreation = false;
        for (const auto& entry : logs) {
            if (entry.find("Created Dummy camera: cam1") != std::string::npos ||
                entry.find("Configured storage for cam1") != std::string::npos) {
                sawCreation = true;
            }
        }
        CHECK(sawCreation);
    }

    // Dummy camera without a storage block: camera loads, but no output file.
    logs.clear();
    {
        const std::string yaml = "cam1:\n"
                                 "  type: camera\n"
                                 "  vendor: dummy\n"
                                 "  width: 32\n"
                                 "  height: 24\n";
        CHECK(vidre::test::writeFile(configPath, yaml));
        auto cameras = vidre::LoadSystemSetup(configPath.string(), {}, log);
        CHECK(cameras.size() == 1);
        CHECK(cameras["cam1"]->getFrameWidth() == 32);
        CHECK(cameras["cam1"]->getFrameHeight() == 24);
        CHECK(cameras["cam1"]->getOutputFileName().empty());
    }

    // Missing vendor: camera is skipped, map stays empty.
    logs.clear();
    {
        const std::string yaml = "cam1:\n"
                                 "  type: camera\n";
        CHECK(vidre::test::writeFile(configPath, yaml));
        auto cameras = vidre::LoadSystemSetup(configPath.string(), {}, log);
        CHECK(cameras.empty());
    }

    // Unsupported vendor without plugin libraries: skipped.
    logs.clear();
    {
        const std::string yaml = "cam1:\n"
                                 "  type: camera\n"
                                 "  vendor: basler\n";
        CHECK(vidre::test::writeFile(configPath, yaml));
        auto cameras = vidre::LoadSystemSetup(configPath.string(), {}, log);
        CHECK(cameras.empty());
    }

    // Node without a "type" key is not a camera.
    logs.clear();
    {
        const std::string yaml = "cam1:\n"
                                 "  some_key: 1\n";
        CHECK(vidre::test::writeFile(configPath, yaml));
        auto cameras = vidre::LoadSystemSetup(configPath.string(), {}, log);
        CHECK(cameras.empty());
    }

    // Missing file: empty map.
    logs.clear();
    {
        auto cameras = vidre::LoadSystemSetup((dir.path() / "does_not_exist.yaml").string(), {}, log);
        CHECK(cameras.empty());
    }

    // Remote storage mode creates a remote frame sink (nothing sent yet).
    logs.clear();
    {
        const std::string yaml = "cam1:\n"
                                 "  type: camera\n"
                                 "  vendor: dummy\n"
                                 "  width: 32\n"
                                 "  height: 24\n";
        CHECK(vidre::test::writeFile(configPath, yaml));
        vidre::LoaderOptions options;
        options.transportMode = vidre::TransportMode::RemoteStorage;
        options.storageEndpoint = "127.0.0.1:50099";
        auto cameras = vidre::LoadSystemSetup(configPath.string(), options, log);
        CHECK(cameras.size() == 1);
        CHECK(cameras["cam1"]->getSentFrames() == 0);
    }

    if (failures != 0) {
        std::cerr << failures << " config_loader_test failure(s)\n";
        return 1;
    }
    std::cout << "config_loader_test: OK\n";
    return 0;
}
