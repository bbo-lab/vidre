#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <CLI/CLI.hpp>

#include <core/transport/storage_server.hpp>

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
    CLI::App app{"VidRe storage service: receives frames over the network and writes CCV files."};

    std::string listenAddress = "0.0.0.0:50053";
    std::string outputDir = ".";

    app.add_option("--listen-address", listenAddress, "TCP listen address for frame + session commands");
    app.add_option("-o,--output-dir", outputDir, "Directory for CCV output files");
    CLI11_PARSE(app, argc, argv);

#ifdef _WIN32
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#else
    std::signal(SIGINT, HandleSignal);
    std::signal(SIGTERM, HandleSignal);
#endif

    vidre::StorageServer storageServer(listenAddress, outputDir,
        [](const std::string& message) { std::cout << message << std::endl; });
    storageServer.start();

    std::cout << "VidRe storage service running (output dir: " << outputDir << "). Press Ctrl+C to stop." << std::endl;
    while (g_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    storageServer.stop();
    return 0;
}
