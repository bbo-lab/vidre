#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "devicelistdock.hpp"
#include "camerawidget.hpp"
#include "camerainfosdock.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QDateTime>
#include <QMdiSubWindow>
#include <QTextEdit>
#include <QToolButton>
#include <QIcon>

#include <iostream>
#include <vector>
#include "yaml-cpp/yaml.h"

#include <core/runtime/remote_runtime.hpp>
#include <core/transport/control_client.hpp>
#include <core/transport/preview_client.hpp>
#include <utils/config_loader/config_loader.hpp>
#include <utils/dynamic_loader/dynamic_library_loader.hpp>
#include <core/transport/transport.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mdiArea_(new QMdiArea(this))
    , statusTimer_(new QTimer(this))
{
    ui->setupUi(this);
    setupUI();
    
    // Setup auto-refresh timer (every 2 seconds)
    connect(statusTimer_, &QTimer::timeout, this, &MainWindow::updateCameraStatus);
    statusTimer_->start(2000);
    
    logMessage("Application started. Load a configuration file to begin.");
}

MainWindow::~MainWindow()
{
    // Stop all cameras before exit
    for (auto& cam : cameras_) {
        if (cam.second && cam.second->isRunning()) {
            cam.second->stop();
        }
    }
    delete ui;
}

void MainWindow::setLibraryPathsFromCommandLine(const QString& aptinaLibraryPath,
                                                const QString& pylonLibraryPath,
                                                const QString& aptinaBaseLibraryPath,
                                                const QString& pylonBaseLibraryPath,
                                                const QString& storageMode,
                                                const QString& storageEndpoint)
{
    if (!aptinaLibraryPath.isEmpty()) {
        aptinaLibraryPath_ = aptinaLibraryPath.toStdString();
        aptinaLibraryPathCliOverride_ = true;
        logMessage(QString("CLI override: aptina library path = %1").arg(aptinaLibraryPath));
    }
    if (!pylonLibraryPath.isEmpty()) {
        pylonLibraryPath_ = pylonLibraryPath.toStdString();
        pylonLibraryPathCliOverride_ = true;
        logMessage(QString("CLI override: pylon library path = %1").arg(pylonLibraryPath));
    }
    if (!aptinaBaseLibraryPath.isEmpty()) {
        aptinaBaseLibraryPath_ = aptinaBaseLibraryPath.toStdString();
        aptinaBaseLibraryPathCliOverride_ = true;
        logMessage(QString("CLI override: aptina base library path = %1").arg(aptinaBaseLibraryPath));
    }
    if (!pylonBaseLibraryPath.isEmpty()) {
        pylonBaseLibraryPath_ = pylonBaseLibraryPath.toStdString();
        pylonBaseLibraryPathCliOverride_ = true;
        logMessage(QString("CLI override: pylon base library path = %1").arg(pylonBaseLibraryPath));
    }

    if (!storageMode.isEmpty()) {
        const auto mode = storageMode.toLower();
        transportMode_ = (mode == "remote") ? vidre::TransportMode::RemoteStorage : vidre::TransportMode::LocalStorage;
        logMessage(QString("CLI override: storage mode = %1").arg(storageMode));
    }

    if (!storageEndpoint.isEmpty()) {
        storageEndpoint_ = storageEndpoint.toStdString();
        logMessage(QString("CLI override: storage endpoint = %1").arg(storageEndpoint));
    }
}

void MainWindow::setConnectEndpoint(const QString& endpoint)
{
    if (endpoint.isEmpty()) {
        return;
    }
    remoteMode_ = true;
    connectEndpoint_ = endpoint.toStdString();

    const auto separator = connectEndpoint_.rfind(':');
    if (separator != std::string::npos && separator + 1 < connectEndpoint_.size()) {
        cameraHost_ = connectEndpoint_.substr(0, separator);
        cameraControlPort_ = static_cast<uint16_t>(std::stoi(connectEndpoint_.substr(separator + 1)));
        cameraPreviewPort_ = cameraControlPort_ + 1;
    } else {
        cameraHost_ = connectEndpoint_;
    }

    logMessage(QString("Remote mode: camera service at %1 (preview port %2)")
        .arg(QString::fromStdString(connectEndpoint_))
        .arg(cameraPreviewPort_));
}

void MainWindow::loadRemoteSetup()
{
    logMessage(QString("Connecting to camera service: %1").arg(QString::fromStdString(connectEndpoint_)));

    auto control = std::make_shared<vidre::ControlClient>(connectEndpoint_);
    std::vector<vidre::transport::CameraInfo> cameraInfos;
    if (!control->connect() || !control->listCameras(cameraInfos)) {
        logMessage("ERROR: Could not connect to the camera service.");
        QMessageBox::warning(this, "Connection Error",
            "Could not connect to the camera service at " + QString::fromStdString(connectEndpoint_));
        return;
    }

    mdiArea_->closeAllSubWindows();
    cameraWidgets_.clear();

    for (auto& cam : cameras_) {
        if (cam.second && cam.second->isRunning()) {
            cam.second->stop();
        }
    }
    cameras_.clear();

    for (const auto& info : cameraInfos) {
        auto preview = std::make_shared<vidre::PreviewClient>(cameraHost_, cameraPreviewPort_, info.cameraName);
        preview->start();
        cameras_[info.cameraName] = std::make_unique<vidre::RemoteRuntimeInstance>(
            info.cameraName, control, preview, info);
        logMessage(QString("Connected camera: %1 (%2)")
            .arg(QString::fromStdString(info.cameraName))
            .arg(QString::fromStdString(info.vendorName)));
    }

    logMessage(QString("Connected %1 remote camera(s)").arg(cameras_.size()));
    deviceListDock_->updateDeviceList(cameras_);
    updateCameraStatus();
    statusBar()->showMessage(QString("%1 remote camera(s)").arg(cameras_.size()), 3000);
}

void MainWindow::setupUI()
{
    // Set MDI area as central widget
    setCentralWidget(mdiArea_);
    mdiArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea_->setViewMode(QMdiArea::SubWindowView);
    
    // Create toolbar and dock widgets
    createToolBar();
    createDockWidgets();
    
    // Connect menu actions
    connect(ui->actionLoadConfig, &QAction::triggered, this, &MainWindow::onLoadConfigClicked);
    connect(ui->actionShowAll, &QAction::triggered, this, &MainWindow::onShowAllClicked);
    connect(ui->actionStopShowAll, &QAction::triggered, this, &MainWindow::onStopShowAllClicked);
    connect(ui->actionStartAll, &QAction::triggered, this, &MainWindow::onStartAllClicked);
    connect(ui->actionStopAll, &QAction::triggered, this, &MainWindow::onStopAllClicked);
    connect(ui->actionRefreshDevices, &QAction::triggered, this, &MainWindow::onRefreshDevicesClicked);
    connect(ui->actionTileWindows, &QAction::triggered, this, &MainWindow::onTileWindows);
    connect(ui->actionCascadeWindows, &QAction::triggered, this, &MainWindow::onCascadeWindows);
    
    // Set window properties
    resize(1200, 800);
    setWindowTitle("Vidre Camera Control");
}

void MainWindow::createToolBar()
{
    mainToolBar_ = addToolBar("Main Toolbar");
    mainToolBar_->setMovable(false);
    mainToolBar_->setIconSize(QSize(32, 32));
    
    QAction *loadAction = mainToolBar_->addAction("Load Config");
    connect(loadAction, &QAction::triggered, this, &MainWindow::onLoadConfigClicked);
    
    mainToolBar_->addSeparator();

    QAction *showAllAction = mainToolBar_->addAction("Show All");
    connect(showAllAction, &QAction::triggered, this, &MainWindow::onShowAllClicked);
    
    QAction *stopShowAllAction = mainToolBar_->addAction("Stop Showing All");
    connect(stopShowAllAction, &QAction::triggered, this, &MainWindow::onStopShowAllClicked);
    
    QAction *startAllAction = mainToolBar_->addAction("Start All");
    connect(startAllAction, &QAction::triggered, this, &MainWindow::onStartAllClicked);
    
    QAction *stopAllAction = mainToolBar_->addAction("Stop All");
    connect(stopAllAction, &QAction::triggered, this, &MainWindow::onStopAllClicked);
    
    mainToolBar_->addSeparator();
    
    QAction *refreshAction = mainToolBar_->addAction("Refresh");
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshDevicesClicked);

    // Add spacer to push indicator to the right
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainToolBar_->addWidget(spacer);

    // Add status indicator
    statusIndicator_ = new QLabel(this);
    statusIndicator_->setFixedSize(20, 20);
    statusIndicator_->setStyleSheet("background-color: grey; border-radius: 10px;");
    statusIndicator_->setToolTip("System Status");
    mainToolBar_->addWidget(statusIndicator_);
}

void MainWindow::createDockWidgets()
{
    // Device list dock
    deviceListDock_ = new DeviceListDock(this);
    QDockWidget *deviceDock = new QDockWidget("Device List", this);
    deviceDock->setWidget(deviceListDock_);
    deviceDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, deviceDock);
    
    connect(deviceListDock_, &DeviceListDock::deviceDoubleClicked, 
            this, &MainWindow::onDeviceDoubleClicked);
    connect(ui->actionDeviceList, &QAction::toggled, deviceDock, &QDockWidget::setVisible);
    connect(deviceDock, &QDockWidget::visibilityChanged, ui->actionDeviceList, &QAction::setChecked);
    
    // Camera info dock (right side)
    cameraInfosDock_ = new CameraInfosDock(this);
    cameraInfosDock_->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, cameraInfosDock_);
    connect(ui->actionCameraInfo, &QAction::toggled, cameraInfosDock_, &QDockWidget::setVisible);
    connect(cameraInfosDock_, &QDockWidget::visibilityChanged, ui->actionCameraInfo, &QAction::setChecked);
    
    // Log dock
    logDock_ = new QDockWidget("Log", this);
    logTextEdit_ = new QTextEdit(this);
    logTextEdit_->setReadOnly(true);
    logDock_->setWidget(logTextEdit_);
    logDock_->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);
    
    connect(ui->actionLog, &QAction::toggled, logDock_, &QDockWidget::setVisible);
    connect(logDock_, &QDockWidget::visibilityChanged, ui->actionLog, &QAction::setChecked);
}

void MainWindow::onLoadConfigClicked()
{
    if (remoteMode_) {
        loadRemoteSetup();
        return;
    }

    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open Configuration File"), "",
        tr("YAML Files (*.yaml *.yml);;All Files (*)"));
    
    if (filename.isEmpty()) {
        return;
    }
    
    loadConfigFile(filename);
}

void MainWindow::loadConfigFile(const QString& filename)
{
    if (remoteMode_) {
        loadRemoteSetup();
        return;
    }

    logMessage("Loading configuration from: " + filename);
    
    // Close all MDI windows and dock widgets
    mdiArea_->closeAllSubWindows();
    for (auto& widget : cameraWidgets_) {
        QDockWidget *dock = qobject_cast<QDockWidget*>(widget.second->parentWidget());
        if (dock) {
            dock->close();
        }
    }
    cameraWidgets_.clear();
    
    // Stop all running cameras first
    for (auto& cam : cameras_) {
        if (cam.second && cam.second->isRunning()) {
            cam.second->stop();
        }
    }
    
    // Load new configuration
    vidre::LoaderOptions loaderOptions;
    loaderOptions.aptinaLibraryPath = aptinaLibraryPath_;
    loaderOptions.pylonLibraryPath = pylonLibraryPath_;
    loaderOptions.aptinaBaseLibraryPath = aptinaBaseLibraryPath_;
    loaderOptions.pylonBaseLibraryPath = pylonBaseLibraryPath_;
    loaderOptions.aptinaLibraryPathCliOverride = aptinaLibraryPathCliOverride_;
    loaderOptions.pylonLibraryPathCliOverride = pylonLibraryPathCliOverride_;
    loaderOptions.aptinaBaseLibraryPathCliOverride = aptinaBaseLibraryPathCliOverride_;
    loaderOptions.pylonBaseLibraryPathCliOverride = pylonBaseLibraryPathCliOverride_;
    loaderOptions.transportMode = transportMode_;
    loaderOptions.storageEndpoint = storageEndpoint_;

    cameras_ = vidre::LoadSystemSetup(
        filename.toStdString(),
        loaderOptions,
        [this](const std::string& message) { logMessage(QString::fromStdString(message)); });
    
    if (cameras_.empty()) {
        logMessage("ERROR: Failed to load configuration or no cameras found.");
        QMessageBox::warning(this, "Configuration Error", 
            "Failed to load configuration file or no cameras found.");
        return;
    }
    
    logMessage(QString("Successfully loaded %1 camera(s)").arg(cameras_.size()));
    
    // Update device list
    deviceListDock_->updateDeviceList(cameras_);
    
    statusBar()->showMessage(QString("%1 camera(s) loaded").arg(cameras_.size()), 3000);
}

void MainWindow::onShowAllClicked()
{
    logMessage("Showing all cameras...");
    
    int started = 0;
    for (auto& cam : cameras_) {
        if (!cam.second->isRunning()) {
            cam.second->show();
            logMessage(QString("Showing: %1").arg(QString::fromStdString(cam.first)));
            started++;
        }
    }
    
    updateCameraStatus();
    statusBar()->showMessage(QString("Showing %1 camera(s)").arg(started), 3000);
}
void MainWindow::onStopShowAllClicked()
{
    logMessage("Stop Showing all cameras...");
    
    int started = 0;
    for (auto& cam : cameras_) {
        if (cam.second->isRunning()) {
            cam.second->stop();
            logMessage(QString("Stop Showing: %1").arg(QString::fromStdString(cam.first)));
            started++;
        }
    }
    
    updateCameraStatus();
    statusBar()->showMessage(QString("Stop Showing %1 camera(s)").arg(started), 3000);
}

void MainWindow::onStartAllClicked()
{
    logMessage("Starting all cameras...");
    
    int started = 0;
    for (auto& cam : cameras_) {
        if (!cam.second->isRunning()) {
            cam.second->run();
            logMessage(QString("Started: %1").arg(QString::fromStdString(cam.first)));
            started++;
        }
    }
    
    updateCameraStatus();
    statusBar()->showMessage(QString("Started %1 camera(s)").arg(started), 3000);
}

void MainWindow::onStopAllClicked()
{
    logMessage("Stopping all cameras...");
    
    int stopped = 0;
    for (auto& cam : cameras_) {
        if (cam.second->isRunning()) {
            cam.second->stop();
            logMessage(QString("Stopped: %1").arg(QString::fromStdString(cam.first)));
            stopped++;
        }
    }
    
    updateCameraStatus();
    statusBar()->showMessage(QString("Stopped %1 camera(s)").arg(stopped), 3000);
}

void MainWindow::onRefreshDevicesClicked()
{
    deviceListDock_->updateDeviceStatus(cameras_);
    updateCameraStatus();
    logMessage("Device list refreshed.");
    statusBar()->showMessage("Refreshed", 2000);
}

void MainWindow::onDeviceDoubleClicked(const QString& deviceName)
{
    // Check if window already exists
    if (cameraWidgets_.find(deviceName) != cameraWidgets_.end()) {
        // Activate existing window
        CameraWidget *widget = cameraWidgets_[deviceName];
        
        QMdiSubWindow *subWindow = qobject_cast<QMdiSubWindow*>(widget->parentWidget());
        if (subWindow) {
            subWindow->show();
            subWindow->raise();
            subWindow->setFocus();
            mdiArea_->setActiveSubWindow(subWindow);
            return;
        }
        
        // Check if floating (standalone window)
        QWidget *window = widget->window();
        if (window && window != this) {
            window->show();
            window->raise();
            window->activateWindow();
            window->setFocus();
            return;
        }
    }
    
    // Create new camera widget window
    auto it = cameras_.find(deviceName.toStdString());
    if (it != cameras_.end()) {
        it->second->setPreviewRequested(true);
        CameraWidget *cameraWidget = new CameraWidget(deviceName, it->second.get(), this);
        cameraWidgets_[deviceName] = cameraWidget;
        
        QMdiSubWindow *subWindow = new QMdiSubWindow();
        subWindow->setWidget(cameraWidget);
        subWindow->setWindowTitle(deviceName);
        subWindow->setAttribute(Qt::WA_DeleteOnClose);
        subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        
        mdiArea_->addSubWindow(subWindow);
        
        cameraWidget->setProperty("mdiSubWindow", QVariant::fromValue((void*)subWindow));
        
        connect(cameraWidget, &CameraWidget::windowClosed, 
                this, &MainWindow::onCameraWindowClosed);
        connect(cameraWidget, &CameraWidget::windowActivated,
                this, &MainWindow::onCameraWindowActivated);
        connect(subWindow, &QMdiSubWindow::destroyed, [this, deviceName]() {
            onCameraWindowClosed(deviceName);
        });
        
        subWindow->show();
        mdiArea_->setActiveSubWindow(subWindow);
        logMessage(QString("Opened window for: %1").arg(deviceName));
    }
}

void MainWindow::onCameraWindowClosed(const QString& cameraName)
{
    cameraWidgets_.erase(cameraName);
    cameraInfosDock_->removeCamera(cameraName);
    auto it = cameras_.find(cameraName.toStdString());
    if (it != cameras_.end()) {
        it->second->setPreviewRequested(false);
    }
    logMessage(QString("Closed window for: %1").arg(cameraName));
}

void MainWindow::onCameraWindowActivated(const QString& cameraName)
{
    auto it = cameras_.find(cameraName.toStdString());
    if (it != cameras_.end()) {
        cameraInfosDock_->addCamera(cameraName, it->second.get());
    }
}

void MainWindow::updateCameraStatus()
{
    if (cameras_.empty()) {
        statusIndicator_->setStyleSheet("background-color: grey; border-radius: 10px;");
        statusIndicator_->setToolTip("No cameras loaded");
        return;
    }
    
    // Update device list
    deviceListDock_->updateDeviceStatus(cameras_);
    
    // Update all open camera widgets
    for (auto& widget : cameraWidgets_) {
        widget.second->updateStatus();
        // widget.second->setRunning();
    }

    // Determine overall system status
    bool anyRunning = false;
    bool anyRecording = false;
    bool anyDroppedFrames = false;
    bool anyError = false;

    for (const auto& cam : cameras_) {
        if (cam.second) {
            cam.second->refresh();
            vidre::RecordingStatus status = cam.second->getRecordingStatus();
            if (status != vidre::RecordingStatus::Stopped) {
                anyRunning = true;
            }
            if (status == vidre::RecordingStatus::RecordingFine || 
                status == vidre::RecordingStatus::RecordingDroppedFrames ||
                status == vidre::RecordingStatus::RecordingError) {
                anyRecording = true;
            }
            if (status == vidre::RecordingStatus::RecordingDroppedFrames) {
                anyDroppedFrames = true;
            }
            if (status == vidre::RecordingStatus::RecordingError) {
                anyError = true;
            }
        }
    }

    if (anyError) {
        statusIndicator_->setStyleSheet("background-color: red; border-radius: 10px;");
        statusIndicator_->setToolTip("Recording Error");
    } else if (anyDroppedFrames) {
        statusIndicator_->setStyleSheet("background-color: yellow; border-radius: 10px;");
        statusIndicator_->setToolTip("Recording (Dropped Frames)");
    } else if (anyRecording) {
        statusIndicator_->setStyleSheet("background-color: green; border-radius: 10px;");
        statusIndicator_->setToolTip("Recording Fine");
    } else if (anyRunning) {
        statusIndicator_->setStyleSheet("background-color: blue; border-radius: 10px;");
        statusIndicator_->setToolTip("Streaming (Not Recording)");
    } else {
        statusIndicator_->setStyleSheet("background-color: grey; border-radius: 10px;");
        statusIndicator_->setToolTip("Stopped");
    }
}

void MainWindow::onTileWindows()
{
    mdiArea_->tileSubWindows();
}

void MainWindow::onCascadeWindows()
{
    mdiArea_->cascadeSubWindows();
}

void MainWindow::logMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);
    logTextEdit_->append(logEntry);
}

