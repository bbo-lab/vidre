#include "mainwindow.hpp"

#include <QApplication>
#include <QCommandLineParser>

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
    
    parser.process(app);
    
    MainWindow window;
    window.setLibraryPathsFromCommandLine(
        parser.isSet(aptinaLibraryOption) ? parser.value(aptinaLibraryOption) : QString(),
        parser.isSet(pylonLibraryOption) ? parser.value(pylonLibraryOption) : QString(),
        parser.isSet(aptinaBaseLibraryOption) ? parser.value(aptinaBaseLibraryOption) : QString(),
        parser.isSet(pylonBaseLibraryOption) ? parser.value(pylonBaseLibraryOption) : QString());
    window.show();
    
    if (parser.isSet(configOption)) {
        window.loadConfigFile(parser.value(configOption));
    }
    
    return app.exec();
}
