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
    
    parser.process(app);
    
    MainWindow window;
    window.show();
    
    if (parser.isSet(configOption)) {
        window.loadConfigFile(parser.value(configOption));
    }
    
    return app.exec();
}
