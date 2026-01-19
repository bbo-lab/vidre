#include "mainwindow.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("Vidre Camera Control");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MPINB");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
