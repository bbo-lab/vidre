#include "mainwindow.hpp"

#include <QApplication>
#include <QCommandLineParser>

#include <iostream>

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

        for (auto& camera : cameras) {
            if (camera.second && !camera.second->isRunning()) {
                camera.second->run();
                std::cout << "Started camera backend: " << camera.first << std::endl;
            }
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
    window.show();

    if (parser.isSet(configOption)) {
        window.loadConfigFile(parser.value(configOption));
    }

    return app.exec();
}
