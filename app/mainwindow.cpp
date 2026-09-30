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

#include <utils/dynamic_loader/dynamic_library_loader.hpp>

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
                                                const QString& pylonBaseLibraryPath)
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
    cameras_ = loadSystemSetup(filename.toStdString());
    
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
        CameraWidget *cameraWidget = new CameraWidget(deviceName, it->second.get(), this);
        cameraWidgets_[deviceName] = cameraWidget;
        
        QMdiSubWindow *subWindow = new QMdiSubWindow();
        subWindow->setWidget(cameraWidget);
        subWindow->setWindowTitle(deviceName);
        subWindow->setAttribute(Qt::WA_DeleteOnClose);
        subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        
        mdiArea_->addSubWindow(subWindow);
        
        cameraWidget->setProperty("mdiSubWindow", QVariant::fromValue((void*)subWindow));
        
        connect(cameraWidget, &CameraWidget::windowClosed, 
                this, &MainWindow::onCameraWindowClosed);
        connect(cameraWidget, &CameraWidget::windowActivated,
                this, &MainWindow::onCameraWindowActivated);
        
        subWindow->show();
        mdiArea_->setActiveSubWindow(subWindow);
        logMessage(QString("Opened window for: %1").arg(deviceName));
    }
}

void MainWindow::onCameraWindowClosed(const QString& cameraName)
{
    cameraWidgets_.erase(cameraName);
    cameraInfosDock_->removeCamera(cameraName);
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
        if (cam.second) { //TODO fix logic
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

std::unique_ptr<vidre::ICamera> MainWindow::loadCamera(const std::string& cameraName, void* nodePtr)
{
    YAML::Node cameraNode = *static_cast<YAML::Node*>(nodePtr);

    const std::string ap_base_str = aptinaBaseLibraryPath_;
    const std::string pylon_base_str = pylonBaseLibraryPath_;
    vidre::APTINA_LIBRARY_PATH = aptinaLibraryPath_;
    vidre::PYLON_LIBRARY_PATH = pylonLibraryPath_;
    
    if (!cameraNode["vendor"]) {
        logMessage(QString("ERROR: No vendor specified for camera %1").arg(QString::fromStdString(cameraName)));
        return nullptr;
    }
    
    std::unique_ptr<vidre::ICamera> camera;
    std::string vendor = cameraNode["vendor"].as<std::string>();
    
    if (vendor == "pylon" && (pylon_base_str.empty() || vidre::util::IsLibraryAvailable(pylon_base_str))) {
        if (vidre::PYLON_LIBRARY_PATH.empty()) {
            logMessage("ERROR: Pylon plugin library path is empty. Set it in config or via --pylon-library.");
            return nullptr;
        }
        using camera_ref_t = vidre::ICamera*();
        using destroy_ref_t = void(vidre::ICamera*);
        auto create = vidre::util::GetFunction<camera_ref_t*>(vidre::PYLON_LIBRARY_PATH, "create_pylon_camera");
        auto destroy = vidre::util::GetFunction<destroy_ref_t*>(vidre::PYLON_LIBRARY_PATH, "destroy_pylon_camera");
        
        if (create == nullptr || destroy == nullptr) {
            logMessage("ERROR: Failed to load Pylon camera functions");
            return nullptr;
        }
        camera = std::unique_ptr<vidre::ICamera>(create());
        logMessage(QString("Created Pylon camera: %1").arg(QString::fromStdString(cameraName)));
    }
    else if (vendor == "aptina" && (ap_base_str.empty() || vidre::util::IsLibraryAvailable(ap_base_str))) {
        if (vidre::APTINA_LIBRARY_PATH.empty()) {
            logMessage("ERROR: Aptina plugin library path is empty. Set it in config or via --aptina-library.");
            return nullptr;
        }
        using camera_ref_t = vidre::ICamera*();
        using destroy_ref_t = void(vidre::ICamera*);
        auto create = vidre::util::GetFunction<camera_ref_t*>(vidre::APTINA_LIBRARY_PATH, "create_aptina_camera");
        auto destroy = vidre::util::GetFunction<destroy_ref_t*>(vidre::APTINA_LIBRARY_PATH, "destroy_aptina_camera");
        
        if (create == nullptr || destroy == nullptr) {
            logMessage("ERROR: Failed to load Aptina camera functions");
            return nullptr;
        }
        camera = std::unique_ptr<vidre::ICamera>(create());
        logMessage(QString("Created Aptina camera: %1").arg(QString::fromStdString(cameraName)));
    }
    else {
        logMessage(QString("ERROR: Unsupported or unavailable vendor: %1").arg(QString::fromStdString(vendor)));
        return nullptr;
    }
    
    // Initialize camera with serial number
    if (cameraNode["serial"] && !camera->Init(cameraNode["serial"].as<std::string>())) {
        logMessage(QString("ERROR: Could not initialize camera with serial: %1")
            .arg(QString::fromStdString(cameraNode["serial"].as<std::string>())));
        return nullptr;
    }
    
    // Load configuration file
    if (cameraNode["config_file"]) {
        if (!camera->LoadConfiguration(cameraNode["config_file"].as<std::string>().c_str())) {
            logMessage("ERROR: Could not load config file (check path)");
            return nullptr;
        }
    }
    
    // Set trigger settings; a failed trigger is logged and the camera is still used
    const QString name = QString::fromStdString(cameraName);
    if (cameraNode["trigger_in"]) {
        try {
            camera->SetTriggerIn(cameraNode["trigger_in"].as<int>());
            camera->triggerInSet = true;
            logMessage(QString("Trigger in set for %1").arg(name));
        } catch (const std::exception& e) {
            logMessage(QString("WARNING: Trigger in could not be set for %1: %2").arg(name, e.what()));
        } catch (...) {
            logMessage(QString("WARNING: Trigger in could not be set for %1").arg(name));
        }
    }
    if (cameraNode["trigger_out"]) {
        try {
            camera->SetTriggerOut(cameraNode["trigger_out"].as<int>());
            camera->triggerOutSet = true;
            logMessage(QString("Trigger out set for %1").arg(name));
        } catch (const std::exception& e) {
            logMessage(QString("WARNING: Trigger out could not be set for %1: %2").arg(name, e.what()));
        } catch (...) {
            logMessage(QString("WARNING: Trigger out could not be set for %1").arg(name));
        }
    }
    
    return camera;
}

std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> 
MainWindow::loadSystemSetup(const std::string& setupFilepath)
{
    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras;
    
    if (!std::filesystem::exists(setupFilepath)) {
        logMessage(QString("ERROR: Configuration file %1 does not exist")
            .arg(QString::fromStdString(setupFilepath)));
        return cameras;
    }
    
    try {
        YAML::Node config = YAML::LoadFile(setupFilepath);

        auto readStringByKeys = [](const YAML::Node& node,
                                   const std::vector<std::string>& keys) -> std::string {
            if (!node || !node.IsMap()) {
                return "";
            }
            for (const auto& key : keys) {
                if (node[key] && node[key].IsScalar()) {
                    return node[key].as<std::string>();
                }
            }
            return "";
        };

        auto applyPath = [this](const std::string& value,
                                std::string& destination,
                                bool cliOverride,
                                const char* label) {
            if (!value.empty() && !cliOverride) {
                destination = value;
                logMessage(QString("Config library path (%1): %2")
                    .arg(label)
                    .arg(QString::fromStdString(value)));
            }
        };

        YAML::Node libraryConfig;
        if (config["libraries"] && config["libraries"].IsMap()) {
            libraryConfig = config["libraries"];
        } else if (config["library_paths"] && config["library_paths"].IsMap()) {
            libraryConfig = config["library_paths"];
        }

        applyPath(
            readStringByKeys(libraryConfig, {"aptina", "aptina_library", "aptina_library_path"}),
            aptinaLibraryPath_,
            aptinaLibraryPathCliOverride_,
            "aptina plugin");
        applyPath(
            readStringByKeys(libraryConfig, {"pylon", "pylon_library", "pylon_library_path"}),
            pylonLibraryPath_,
            pylonLibraryPathCliOverride_,
            "pylon plugin");
        applyPath(
            readStringByKeys(libraryConfig, {"aptina_base", "aptina_base_library", "aptina_base_library_path"}),
            aptinaBaseLibraryPath_,
            aptinaBaseLibraryPathCliOverride_,
            "aptina base");
        applyPath(
            readStringByKeys(libraryConfig, {"pylon_base", "pylon_base_library", "pylon_base_library_path"}),
            pylonBaseLibraryPath_,
            pylonBaseLibraryPathCliOverride_,
            "pylon base");

        applyPath(
            readStringByKeys(config, {"aptina_library_path", "aptina_library"}),
            aptinaLibraryPath_,
            aptinaLibraryPathCliOverride_,
            "aptina plugin");
        applyPath(
            readStringByKeys(config, {"pylon_library_path", "pylon_library"}),
            pylonLibraryPath_,
            pylonLibraryPathCliOverride_,
            "pylon plugin");
        applyPath(
            readStringByKeys(config, {"aptina_base_library_path", "aptina_base_library"}),
            aptinaBaseLibraryPath_,
            aptinaBaseLibraryPathCliOverride_,
            "aptina base");
        applyPath(
            readStringByKeys(config, {"pylon_base_library_path", "pylon_base_library"}),
            pylonBaseLibraryPath_,
            pylonBaseLibraryPathCliOverride_,
            "pylon base");
        
        for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
            if (!it->second.IsMap())
                continue;
            if (!it->second["type"])
                continue;
            
            // Create camera from file-defined details if the node is a camera node
            if (it->second["type"].as<std::string>() == "camera") {
                std::string cameraName = it->first.as<std::string>();
                cameras[cameraName] = std::make_unique<vidre::RuntimeInstance>();
                
                YAML::Node cameraNode = it->second;
                cameras[cameraName]->setCamera(loadCamera(cameraName, &cameraNode));
                
                if (!cameras[cameraName]->checkCamera()) {
                    logMessage(QString("WARNING: Camera %1 was not constructed properly")
                        .arg(QString::fromStdString(cameraName)));
                    continue;
                }
                
                // Setup storage
                if (it->second["storage"]) {
                    auto storage_config = it->second["storage"];
                    auto output_file = storage_config["output_file"].as<std::string>();
                    auto header_format = storage_config["header"].as<std::string>();
                    auto bits_per_pixel = storage_config["bits_per_pixel"].as<uint64_t>();
                    auto packed = storage_config["packed"].as<uint8_t>();
                    
                    cameras[cameraName]->createStorage(output_file, header_format, bits_per_pixel, packed);
                    logMessage(QString("Configured storage for %1: %2")
                        .arg(QString::fromStdString(cameraName))
                        .arg(QString::fromStdString(output_file)));
                }
            }
        }
    }
    catch (const YAML::Exception& e) {
        logMessage(QString("YAML Error: %1").arg(e.what()));
    }
    catch (const std::exception& e) {
        logMessage(QString("Error loading configuration: %1").arg(e.what()));
    }
    
    return cameras;
}
