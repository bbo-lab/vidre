#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <CLI/CLI.hpp>

#include <core/transport/camera_backend.hpp>
#include <core/transport/control_server.hpp>
#include <core/transport/preview_server.hpp>
#include <utils/config_loader/config_loader.hpp>

namespace {

    std::atomic<bool> g_running{true};

#ifdef _WIN32
    BOOL WINAPI ConsoleCtrlHandler(DWORD /*type*/) {
        g_running = false;
        return TRUE;
    }
#else
    void HandleSignal(int /*signal*/) {
        g_running = false;
    }
#endif

}  // namespace

int main(int argc, char** argv)
{
    CLI::App app{"VidRe camera service: hosts cameras and exposes control + preview endpoints."};

    std::string config;
    std::string controlAddress = "0.0.0.0:50051";
    std::string previewAddress = "0.0.0.0:50052";
    std::string storageEndpoint;
    std::string aptinaLibrary;
    std::string pylonLibrary;
    std::string aptinaBaseLibrary;
    std::string pylonBaseLibrary;
    bool autostart = false;

    app.add_option("-c,--config", config, "System setup YAML file")->required();
    app.add_option("--control-address", controlAddress, "Control server listen address");
    app.add_option("--preview-address", previewAddress, "Preview server listen address");
    app.add_option("--storage-endpoint", storageEndpoint, "Storage service endpoint host:port");
    app.add_option("--aptina-library", aptinaLibrary, "Path to vidre Aptina camera plugin library");
    app.add_option("--pylon-library", pylonLibrary, "Path to vidre Pylon camera plugin library");
    app.add_option("--aptina-base-library", aptinaBaseLibrary, "Path to Aptina base runtime library");
    app.add_option("--pylon-base-library", pylonBaseLibrary, "Path to Pylon base runtime library");
    app.add_flag("--autostart", autostart, "Automatically start recording for all cameras with a storage config");
    CLI11_PARSE(app, argc, argv);

#ifdef _WIN32
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#else
    std::signal(SIGINT, HandleSignal);
    std::signal(SIGTERM, HandleSignal);
#endif

    vidre::LoaderOptions loaderOptions;
    loaderOptions.aptinaLibraryPath = aptinaLibrary;
    loaderOptions.pylonLibraryPath = pylonLibrary;
    loaderOptions.aptinaBaseLibraryPath = aptinaBaseLibrary;
    loaderOptions.pylonBaseLibraryPath = pylonBaseLibrary;
    loaderOptions.aptinaLibraryPathCliOverride = !aptinaLibrary.empty();
    loaderOptions.pylonLibraryPathCliOverride = !pylonLibrary.empty();
    loaderOptions.aptinaBaseLibraryPathCliOverride = !aptinaBaseLibrary.empty();
    loaderOptions.pylonBaseLibraryPathCliOverride = !pylonBaseLibrary.empty();
    loaderOptions.transportMode = vidre::TransportMode::RemoteStorage;
    loaderOptions.storageEndpoint = storageEndpoint;

    auto cameras = vidre::LoadSystemSetup(
        config,
        loaderOptions,
        [](const std::string& message) { std::cout << message << std::endl; });

    if (cameras.empty()) {
        std::cerr << "No cameras were loaded from " << config << std::endl;
        return 1;
    }

    vidre::CameraBackend backend;
    backend.setRuntimes(std::move(cameras));

    vidre::ControlServer controlServer(&backend, controlAddress,
        [](const std::string& message) { std::cout << message << std::endl; });
    controlServer.start();

    vidre::PreviewServer previewServer(&backend, previewAddress,
        [](const std::string& message) { std::cout << message << std::endl; });
    previewServer.start();

    if (autostart) {
        std::cout << "Autostarting all cameras..." << std::endl;
        for (const auto& info : backend.listCameras()) {
            const auto response = backend.start(info.cameraName);
            std::cout << (response.ok ? "Started: " : "Failed to start: ") << info.cameraName
                      << (response.ok ? "" : " - " + response.message) << std::endl;
        }
    }

    std::cout << "VidRe camera service running. Press Ctrl+C to stop." << std::endl;
    while (g_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Stopping camera service..." << std::endl;
    backend.stopAll();
    previewServer.stop();
    controlServer.stop();
    return 0;
}
