#include "camerainfosdock.hpp"
#include <QVBoxLayout>
#include <QDateTime>
#include <QString>
#include <QFormLayout>

namespace {
    QString groupStyleSheet() {
        return
            "QGroupBox {"
            "    background-color: #252526;"
            "    color: #d4d4d4;"
            "    border: 1px solid #555555;"
            "    border-radius: 4px;"
            "    margin-top: 8px;"
            "    font-weight: bold;"
            "}"
            "QGroupBox::title {"
            "    subcontrol-origin: margin;"
            "    left: 8px;"
            "    padding: 0 4px;"
            "}";
    }
}

CameraInfosDock::CameraInfosDock(QWidget *parent)
    : QDockWidget("Active Camera Info", parent)
    , contentWidget_(new QWidget(this))
    , scrollArea_(new QScrollArea(this))
    , camerasLayout_(new QVBoxLayout())
    , emptyLabel_(new QLabel("No active cameras", this))
    , updateTimer_(new QTimer(this))
{
    setObjectName("CameraInfosDock");

    camerasLayout_->setContentsMargins(8, 8, 8, 8);
    camerasLayout_->setSpacing(8);
    camerasLayout_->setAlignment(Qt::AlignTop);

    emptyLabel_->setStyleSheet("color: #999999; padding: 8px;");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    camerasLayout_->addWidget(emptyLabel_);
    camerasLayout_->addStretch();

    contentWidget_->setLayout(camerasLayout_);
    contentWidget_->setStyleSheet("background-color: #1e1e1e;");

    scrollArea_->setWidgetResizable(true);
    scrollArea_->setWidget(contentWidget_);
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setMinimumWidth(360);
    setWidget(scrollArea_);
    
    // Set up timer for periodic updates
    connect(updateTimer_, &QTimer::timeout, this, &CameraInfosDock::updateAllCamerasInfo);
    updateTimer_->setInterval(500); // Update every 500ms
    updateTimer_->start();
}

CameraInfosDock::~CameraInfosDock()
{
    updateTimer_->stop();
}

void CameraInfosDock::addCamera(const QString& cameraName, vidre::RuntimeInstance* runtimeInstance)
{
    if (!runtimeInstance) {
        return;
    }

    auto existing = cameras_.find(cameraName.toStdString());
    if (existing != cameras_.end()) {
        existing->second.runtimeInstance = runtimeInstance;
        return;
    }

    CameraData data;
    data.runtimeInstance = runtimeInstance;
    data.lastInfo = "";
    data.groupBox = nullptr;
    data.infoLabel = nullptr;
    data.exposureSpin = nullptr;
    data.gainSpin = nullptr;
    data.applyButton = nullptr;
    data.controlsInitialized = false;

    cameras_[cameraName.toStdString()] = data;
    CameraData& storedData = cameras_[cameraName.toStdString()];
    storedData.groupBox = createCameraGroup(cameraName, storedData);

    camerasLayout_->insertWidget(camerasLayout_->count() - 1, storedData.groupBox);

    updateCameraInfo(cameraName);
    refreshDisplay();
}

void CameraInfosDock::removeCamera(const QString& cameraName)
{
    auto it = cameras_.find(cameraName.toStdString());
    if (it != cameras_.end()) {
        if (it->second.groupBox) {
            it->second.groupBox->deleteLater();
        }
        cameras_.erase(it);
    }
    refreshDisplay();
}

void CameraInfosDock::updateCameraInfo(const QString& cameraName)
{
    auto it = cameras_.find(cameraName.toStdString());
    if (it != cameras_.end()) {
        it->second.lastInfo = generateCameraInfo(cameraName, it->second.runtimeInstance);
        if (it->second.infoLabel) {
            it->second.infoLabel->setText(it->second.lastInfo);
        }
    }
}

void CameraInfosDock::clearAllCameras()
{
    for (auto& [_, cameraData] : cameras_) {
        if (cameraData.groupBox) {
            cameraData.groupBox->deleteLater();
        }
    }
    cameras_.clear();
    refreshDisplay();
}

void CameraInfosDock::updateAllCamerasInfo()
{
    bool changed = false;
    
    for (auto& [cameraNameStr, cameraData] : cameras_) {
        QString cameraName = QString::fromStdString(cameraNameStr);
        QString newInfo = generateCameraInfo(cameraName, cameraData.runtimeInstance);
        
        if (newInfo != cameraData.lastInfo) {
            cameraData.lastInfo = newInfo;
            if (cameraData.infoLabel) {
                cameraData.infoLabel->setText(newInfo);
            }
            changed = true;
        }

        vidre::ICamera* camera = cameraData.runtimeInstance ? cameraData.runtimeInstance->getCamera() : nullptr;
        if (camera && !cameraData.controlsInitialized) {
            cameraData.exposureSpin->setValue(camera->GetExposure());
            cameraData.gainSpin->setValue(camera->GetGain());
            cameraData.controlsInitialized = true;
        }
    }
    
    if (changed) {
        refreshDisplay();
    }
}

void CameraInfosDock::refreshDisplay()
{
    emptyLabel_->setVisible(cameras_.empty());
}

QString CameraInfosDock::generateCameraInfo(const QString& cameraName, vidre::RuntimeInstance* runtimeInstance)
{
    if (!runtimeInstance) {
        return QString("Error: Invalid camera instance");
    }
    
    QString info;
    
    
    info += QString("Camera: %1\n").arg(cameraName);
    
    
    vidre::ICamera* camera = runtimeInstance->getCamera();
    
    if (camera) {
        
        std::string serialNumber = camera->GetSerialNumber();
        std::string vendorName = camera->GetVendorName();
        
        info += QString("Serial: %1\n").arg(QString::fromStdString(serialNumber));
        info += QString("Vendor: %1\n").arg(QString::fromStdString(vendorName));
        
        
        info += "\n";
        
        info += "Frame Configuration:\n";
        info += QString("  Resolution: %1 x %2\n")
            .arg(camera->GetFrameWidth())
            .arg(camera->GetFrameHeight());
        
        info += QString("  Offset: X=%1, Y=%2\n")
            .arg(camera->GetFrameOffsetX())
            .arg(camera->GetFrameOffsetY());
        
        info += QString("  Frame Bytes: %1\n").arg(camera->GetFrameBytes());
        
        info += QString("  Buffer Size: %1 MB\n")
            .arg(camera->GetBufferSize() / (1024.0 * 1024.0), 0, 'f', 2);
        
        info += QString("  Max Buffers: %1\n").arg(camera->GetMaxNumBuffer());
        
        info += "\n";
        
        info += "Acquisition Settings:\n";
        info += QString("  Frame Rate: %1 fps\n")
            .arg(camera->GetFrameRate(), 0, 'f', 2);
        info += QString("  Exposure: %1\n").arg(camera->GetExposure(), 0, 'f', 2);
        info += QString("  Gain: %1\n").arg(camera->GetGain(), 0, 'f', 2);
        
        info += "\n";
        
        info += "Recording Status:\n";
        vidre::RecordingStatus status = runtimeInstance->getRecordingStatus();
        QString statusStr;
        switch (status) {
            case vidre::RecordingStatus::Stopped:
                statusStr = "Stopped";
                break;
            case vidre::RecordingStatus::Streaming:
                statusStr = "Streaming";
                break;
            case vidre::RecordingStatus::RecordingFine:
                statusStr = "Recording (OK)";
                break;
            case vidre::RecordingStatus::RecordingDroppedFrames:
                statusStr = "Recording (Dropped Frames)";
                break;
            case vidre::RecordingStatus::RecordingError:
                statusStr = "Recording (Error)";
                break;
            default:
                statusStr = "Unknown";
        }
        
        info += QString("  Status: %1\n").arg(statusStr);
        
        info += QString("  Armed: %1\n").arg(camera->IsArmed() ? "Yes" : "No");
        info += QString("  Running: %1\n").arg(camera->IsRunning() ? "Yes" : "No");
    }
    
    info += QString("  Last Update: %1\n")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
    
    return info;
}

QGroupBox* CameraInfosDock::createCameraGroup(const QString& cameraName, CameraData& data)
{
    QGroupBox* group = new QGroupBox(QString("%1 Controls").arg(cameraName), contentWidget_);
    group->setStyleSheet(groupStyleSheet());

    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->setContentsMargins(8, 8, 8, 8);
    groupLayout->setSpacing(6);

    data.infoLabel = new QLabel(group);
    data.infoLabel->setWordWrap(true);
    data.infoLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #1e1e1e;"
        "    color: #d4d4d4;"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "    padding: 6px;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 9pt;"
        "}"
    );

    QWidget* controlsWidget = new QWidget(group);
    QFormLayout* controlsLayout = new QFormLayout(controlsWidget);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->setSpacing(6);

    data.exposureSpin = new QDoubleSpinBox(controlsWidget);
    data.exposureSpin->setDecimals(2);
    data.exposureSpin->setRange(0.0, 1000000.0);
    data.exposureSpin->setSingleStep(10.0);
    data.exposureSpin->setValue(0.0);
    data.exposureSpin->setSuffix(" us");

    data.gainSpin = new QDoubleSpinBox(controlsWidget);
    data.gainSpin->setDecimals(2);
    data.gainSpin->setRange(0.0, 1024.0);
    data.gainSpin->setSingleStep(0.25);
    data.gainSpin->setValue(0.0);

    controlsLayout->addRow("Exposure", data.exposureSpin);
    controlsLayout->addRow("Gain", data.gainSpin);

    data.applyButton = new QPushButton("Apply", controlsWidget);
    controlsLayout->addRow("", data.applyButton);

    connect(data.applyButton, &QPushButton::clicked, this, [this, cameraName]() {
        applyCameraSettings(cameraName);
    });
    connect(data.exposureSpin, &QDoubleSpinBox::editingFinished, this, [this, cameraName]() {
        applyCameraSettings(cameraName);
    });
    connect(data.gainSpin, &QDoubleSpinBox::editingFinished, this, [this, cameraName]() {
        applyCameraSettings(cameraName);
    });

    groupLayout->addWidget(data.infoLabel);
    groupLayout->addWidget(controlsWidget);

    return group;
}

void CameraInfosDock::applyCameraSettings(const QString& cameraName)
{
    auto it = cameras_.find(cameraName.toStdString());
    if (it == cameras_.end()) {
        return;
    }

    CameraData& cameraData = it->second;
    if (!cameraData.runtimeInstance || !cameraData.exposureSpin || !cameraData.gainSpin) {
        return;
    }

    vidre::ICamera* camera = cameraData.runtimeInstance->getCamera();
    if (!camera) {
        return;
    }

    camera->SetExposure(cameraData.exposureSpin->value());
    camera->SetGain(cameraData.gainSpin->value());

    cameraData.lastInfo = generateCameraInfo(cameraName, cameraData.runtimeInstance);
    if (cameraData.infoLabel) {
        cameraData.infoLabel->setText(cameraData.lastInfo);
    }
}
