#include "mainwindow.hpp"

#include <QApplication>
#include <QCommandLineParser>

#include <iostream>

#include <core/transport/camera_backend.hpp>
#include <core/transport/control_server.hpp>
#include <core/transport/preview_server.hpp>
#include <utils/config_loader/config_loader.hpp>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("Vidre Camera Control");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MPINB");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Vidre Camera Control");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption configOption(
        QStringList() << "config",
        "File path to system configuration file (YAML).",
        "file");
    parser.addOption(configOption);

    QCommandLineOption aptinaLibraryOption(
        QStringList() << "aptina-library",
        "Path to vidre Aptina camera plugin library.",
        "file");
    parser.addOption(aptinaLibraryOption);

    QCommandLineOption pylonLibraryOption(
        QStringList() << "pylon-library",
        "Path to vidre Pylon camera plugin library.",
        "file");
    parser.addOption(pylonLibraryOption);

    QCommandLineOption aptinaBaseLibraryOption(
        QStringList() << "aptina-base-library",
        "Path to Aptina base runtime library (for vendor availability checks).",
        "file");
    parser.addOption(aptinaBaseLibraryOption);

    QCommandLineOption pylonBaseLibraryOption(
        QStringList() << "pylon-base-library",
        "Path to Pylon base runtime library (for vendor availability checks).",
        "file");
    parser.addOption(pylonBaseLibraryOption);

    QCommandLineOption storageModeOption(
        QStringList() << "storage-mode",
        "Storage transport mode for camera microservices (local or remote).",
        "mode");
    parser.addOption(storageModeOption);

    QCommandLineOption storageEndpointOption(
        QStringList() << "storage-endpoint",
        "Endpoint for the storage microservice in host:port form.",
        "endpoint");
    parser.addOption(storageEndpointOption);

    QCommandLineOption headlessOption(
        QStringList() << "headless",
        "Run cameras without creating the Qt GUI window.");
    parser.addOption(headlessOption);

    QCommandLineOption connectOption(
        QStringList() << "connect",
        "Camera service endpoint in host:port form (remote mode).",
        "endpoint");
    parser.addOption(connectOption);

    QCommandLineOption controlPortOption(
        QStringList() << "control-port",
        "Port for the control server in headless mode (default 50051).",
        "port");
    parser.addOption(controlPortOption);

    QCommandLineOption previewPortOption(
        QStringList() << "preview-port",
        "Port for the preview server in headless mode (default 50052).",
        "port");
    parser.addOption(previewPortOption);
    
    parser.process(app);

    const QString storageModeValue = parser.isSet(storageModeOption) ? parser.value(storageModeOption) : QString();
    const QString storageEndpointValue = parser.isSet(storageEndpointOption) ? parser.value(storageEndpointOption) : QString();
    
    if (parser.isSet(headlessOption)) {
        if (!parser.isSet(configOption)) {
            std::cerr << "--headless requires --config" << std::endl;
            return 1;
        }

        vidre::LoaderOptions loaderOptions;
        loaderOptions.aptinaLibraryPath = parser.isSet(aptinaLibraryOption) ? parser.value(aptinaLibraryOption).toStdString() : std::string();
        loaderOptions.pylonLibraryPath = parser.isSet(pylonLibraryOption) ? parser.value(pylonLibraryOption).toStdString() : std::string();
        loaderOptions.aptinaBaseLibraryPath = parser.isSet(aptinaBaseLibraryOption) ? parser.value(aptinaBaseLibraryOption).toStdString() : std::string();
        loaderOptions.pylonBaseLibraryPath = parser.isSet(pylonBaseLibraryOption) ? parser.value(pylonBaseLibraryOption).toStdString() : std::string();
        loaderOptions.aptinaLibraryPathCliOverride = parser.isSet(aptinaLibraryOption);
        loaderOptions.pylonLibraryPathCliOverride = parser.isSet(pylonLibraryOption);
        loaderOptions.aptinaBaseLibraryPathCliOverride = parser.isSet(aptinaBaseLibraryOption);
        loaderOptions.pylonBaseLibraryPathCliOverride = parser.isSet(pylonBaseLibraryOption);

        if (!storageModeValue.isEmpty()) {
            const auto storageMode = storageModeValue.toLower();
            if (storageMode == "remote") {
                loaderOptions.transportMode = vidre::TransportMode::RemoteStorage;
            }
        }
        if (!storageEndpointValue.isEmpty()) {
            loaderOptions.storageEndpoint = storageEndpointValue.toStdString();
        }

        auto cameras = vidre::LoadSystemSetup(
            parser.value(configOption).toStdString(),
            loaderOptions,
            [](const std::string& message) { std::cout << message << std::endl; });

        if (cameras.empty()) {
            std::cerr << "No cameras were loaded." << std::endl;
            return 1;
        }

        vidre::CameraBackend backend;
        backend.setRuntimes(std::move(cameras));

        const std::string controlAddress = "0.0.0.0:" + (parser.isSet(controlPortOption) ? parser.value(controlPortOption).toStdString() : std::string("50051"));
        const std::string previewAddress = "0.0.0.0:" + (parser.isSet(previewPortOption) ? parser.value(previewPortOption).toStdString() : std::string("50052"));

        vidre::ControlServer controlServer(&backend, controlAddress,
            [](const std::string& message) { std::cout << message << std::endl; });
        controlServer.start();

        vidre::PreviewServer previewServer(&backend, previewAddress,
            [](const std::string& message) { std::cout << message << std::endl; });
        previewServer.start();

        std::cout << "Headless camera service: control on " << controlAddress
                  << ", preview on " << previewAddress << std::endl;

        for (const auto& info : backend.listCameras()) {
            const auto response = backend.start(info.cameraName);
            std::cout << (response.ok ? "Started camera backend: " : "Failed to start: ")
                      << info.cameraName << (response.ok ? "" : " - " + response.message) << std::endl;
        }

        return app.exec();
    }

    MainWindow window;
    window.setLibraryPathsFromCommandLine(
        parser.isSet(aptinaLibraryOption) ? parser.value(aptinaLibraryOption) : QString(),
        parser.isSet(pylonLibraryOption) ? parser.value(pylonLibraryOption) : QString(),
        parser.isSet(aptinaBaseLibraryOption) ? parser.value(aptinaBaseLibraryOption) : QString(),
        parser.isSet(pylonBaseLibraryOption) ? parser.value(pylonBaseLibraryOption) : QString(),
        storageModeValue,
        storageEndpointValue);
    if (parser.isSet(connectOption)) {
        window.setConnectEndpoint(parser.value(connectOption));
    }
    window.show();

    if (parser.isSet(connectOption)) {
        window.loadRemoteSetup();
    } else if (parser.isSet(configOption)) {
        window.loadConfigFile(parser.value(configOption));
    }

    return app.exec();
}
