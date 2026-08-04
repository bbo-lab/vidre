#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QMdiArea>
#include <QTimer>
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QLabel>
#include <memory>
#include <unordered_map>
#include <string>
#include <streambuf>
#include <mutex>

#include <core/runtime/runtime.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class DeviceListDock;
class CameraWidget;
class CameraInfosDock;

class LogStreamBuffer : public std::streambuf {
public:
    explicit LogStreamBuffer(QTextEdit* textEdit);
    ~LogStreamBuffer();
protected:
    std::streambuf::int_type overflow(std::streambuf::int_type ch) override;
    int sync() override;
private:
    QTextEdit* textEdit_;
    std::string buffer_;
    std::mutex mutex_;
    std::streambuf* oldCout_;
    std::streambuf* oldCerr_;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /// Load a YAML configuration file (same as "Load Config" button).
    void loadConfigFile(const QString& filepath);
    void setLibraryPathsFromCommandLine(const QString& aptinaLibraryPath,
                                        const QString& pylonLibraryPath,
                                        const QString& aptinaBaseLibraryPath,
                                        const QString& pylonBaseLibraryPath);

private slots:
    void onLoadConfigClicked();
    void onShowAllClicked();
    void onStopShowAllClicked();
    void onStartAllClicked();
    void onStopAllClicked();
    void onRefreshDevicesClicked();
    void onDeviceDoubleClicked(const QString& deviceName);
    void onDeviceRecordRequested(const QString& deviceName);
    void onDeviceShowRequested(const QString& deviceName);
    void onDeviceStopRequested(const QString& deviceName);
    void onCameraWindowClosed(const QString& cameraName);
    void onCameraWindowActivated(const QString& cameraName);
    void updateCameraStatus();
    void onTileWindows();
    void onCascadeWindows();

private:
    Ui::MainWindow *ui;
    QMdiArea *mdiArea_;
    DeviceListDock *deviceListDock_;
    QToolBar *mainToolBar_;
    QDockWidget *logDock_;
    QTextEdit *logTextEdit_;
    QLabel *statusIndicator_;
    CameraInfosDock *cameraInfosDock_;
    LogStreamBuffer *logStreamBuffer_;

    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras_;
    std::unordered_map<QString, CameraWidget*> cameraWidgets_;
    QTimer *statusTimer_;

    std::string aptinaLibraryPath_;
    std::string pylonLibraryPath_;
    std::string aptinaBaseLibraryPath_;
    std::string pylonBaseLibraryPath_;
    bool aptinaLibraryPathCliOverride_ = false;
    bool pylonLibraryPathCliOverride_ = false;
    bool aptinaBaseLibraryPathCliOverride_ = false;
    bool pylonBaseLibraryPathCliOverride_ = false;
    
    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> loadSystemSetup(const std::string& setupFilepath);
    void setupUI();
    void createToolBar();
    void createDockWidgets();
    void logMessage(const QString& message);
};

#endif // MAINWINDOW_HPP
