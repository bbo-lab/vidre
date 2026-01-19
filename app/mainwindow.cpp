#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "devicelistdock.hpp"
#include "camerawidget.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QDateTime>
#include <QMdiSubWindow>
#include <QTextEdit>
#include <QToolButton>
#include <QIcon>

#include <iostream>
#include <boost/filesystem.hpp>
#include "yaml-cpp/yaml.h"

#include "utils/dynamic_loader/dynamic_library_loader.hpp"

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
    
    QAction *startAllAction = mainToolBar_->addAction("Start All");
    connect(startAllAction, &QAction::triggered, this, &MainWindow::onStartAllClicked);
    
    QAction *stopAllAction = mainToolBar_->addAction("Stop All");
    connect(stopAllAction, &QAction::triggered, this, &MainWindow::onStopAllClicked);
    
    mainToolBar_->addSeparator();
    
    QAction *refreshAction = mainToolBar_->addAction("Refresh");
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshDevicesClicked);
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
    
    logMessage("Loading configuration from: " + filename);
    
    // Close all MDI windows
    mdiArea_->closeAllSubWindows();
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
        QMdiSubWindow *subWindow = mdiArea_->currentSubWindow();
        for (auto *window : mdiArea_->subWindowList()) {
            if (window->widget() == cameraWidgets_[deviceName]) {
                mdiArea_->setActiveSubWindow(window);
                return;
            }
        }
    }
    
    // Create new camera widget window
    auto it = cameras_.find(deviceName.toStdString());
    if (it != cameras_.end()) {
        CameraWidget *cameraWidget = new CameraWidget(deviceName, it->second.get(), this);
        QMdiSubWindow *subWindow = mdiArea_->addSubWindow(cameraWidget);
        subWindow->setAttribute(Qt::WA_DeleteOnClose);
        
        cameraWidgets_[deviceName] = cameraWidget;
        
        connect(cameraWidget, &CameraWidget::windowClosed, 
                this, &MainWindow::onCameraWindowClosed);
        
        subWindow->show();
        logMessage(QString("Opened window for: %1").arg(deviceName));
    }
}

void MainWindow::onCameraWindowClosed(const QString& cameraName)
{
    cameraWidgets_.erase(cameraName);
    logMessage(QString("Closed window for: %1").arg(cameraName));
}

void MainWindow::updateCameraStatus()
{
    if (cameras_.empty()) {
        return;
    }
    
    // Update device list
    deviceListDock_->updateDeviceStatus(cameras_);
    
    // Update all open camera widgets
    for (auto& widget : cameraWidgets_) {
        widget.second->updateStatus();
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
    
    // TODO: cli/userinput/config
    std::string ap_base_str = "C:/Aptina Imaging/bin/apbase.dll"; 
    std::string pylon_base_str = "C:/Program Files/Basler/pylon 8/Runtime/x64/PylonBase_v9.dll";
    
// #ifdef _WIN32
    vidre::APTINA_LIBRARY_PATH = "vidre_aptina.dll";
    vidre::PYLON_LIBRARY_PATH = "vidre_pylon.dll";
// #elif __APPLE__
//     vidre::APTINA_LIBRARY_PATH = "libvidre_aptina.dylib";
//     vidre::PYLON_LIBRARY_PATH = "libvidre_pylon.dylib";
// #else
//     vidre::APTINA_LIBRARY_PATH = "libvidre_aptina.so";
//     vidre::PYLON_LIBRARY_PATH = "libvidre_pylon.so";
// #endif
    
    if (!cameraNode["vendor"]) {
        logMessage(QString("ERROR: No vendor specified for camera %1").arg(QString::fromStdString(cameraName)));
        return nullptr;
    }
    
    std::unique_ptr<vidre::ICamera> camera;
    std::string vendor = cameraNode["vendor"].as<std::string>();
    
    if (vendor == "pylon" && vidre::util::IsLibraryAvailable(pylon_base_str)) {
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
    else if (vendor == "aptina" && vidre::util::IsLibraryAvailable(ap_base_str)) {
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
    
    // Set trigger settings
    if (cameraNode["trigger_in"]) {
        camera->SetTriggerIn(cameraNode["trigger_in"].as<int>());
    }
    if (cameraNode["trigger_out"]) {
        camera->SetTriggerOut(cameraNode["trigger_out"].as<int>());
    }
    
    return camera;
}

std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> 
MainWindow::loadSystemSetup(const std::string& setupFilepath)
{
    std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>> cameras;
    
    if (!boost::filesystem::exists(setupFilepath)) {
        logMessage("ERROR: Configuration file does not exist");
        return cameras;
    }
    
    try {
        YAML::Node config = YAML::LoadFile(setupFilepath);
        
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
