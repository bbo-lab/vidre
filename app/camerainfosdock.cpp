#include "camerainfosdock.hpp"
#include <QVBoxLayout>
#include <QDateTime>
#include <QString>

CameraInfosDock::CameraInfosDock(QWidget *parent)
    : QDockWidget("Active Camera Info", parent)
    , updateTimer_(new QTimer(this))
{
    setObjectName("CameraInfosDock");
    
    infoTextEdit_ = new QTextEdit(this);
    infoTextEdit_->setReadOnly(true);
    infoTextEdit_->setFont(QFont("Consolas", 9));
    infoTextEdit_->setStyleSheet(
        "QTextEdit {"
        "    background-color: #1e1e1e;"
        "    color: #d4d4d4;"
        "    border: 1px solid #555555;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 9pt;"
        "    padding: 5px;"
        "}"
    );
    
    setWidget(infoTextEdit_);
    
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
    
    CameraData data;
    data.runtimeInstance = runtimeInstance;
    data.lastInfo = "";
    
    cameras_[cameraName.toStdString()] = data;
    refreshDisplay();
}

void CameraInfosDock::removeCamera(const QString& cameraName)
{
    cameras_.erase(cameraName.toStdString());
    refreshDisplay();
}

void CameraInfosDock::updateCameraInfo(const QString& cameraName)
{
    auto it = cameras_.find(cameraName.toStdString());
    if (it != cameras_.end()) {
        it->second.lastInfo = generateCameraInfo(cameraName, it->second.runtimeInstance);
    }
}

void CameraInfosDock::clearAllCameras()
{
    cameras_.clear();
    infoTextEdit_->clear();
}

void CameraInfosDock::updateAllCamerasInfo()
{
    bool changed = false;
    
    for (auto& [cameraNameStr, cameraData] : cameras_) {
        QString cameraName = QString::fromStdString(cameraNameStr);
        QString newInfo = generateCameraInfo(cameraName, cameraData.runtimeInstance);
        
        if (newInfo != cameraData.lastInfo) {
            cameraData.lastInfo = newInfo;
            changed = true;
        }
    }
    
    if (changed) {
        refreshDisplay();
    }
}

void CameraInfosDock::refreshDisplay()
{
    QString displayText;
    
    if (cameras_.empty()) {
        displayText = "No active cameras";
        infoTextEdit_->setText(displayText);
        return;
    }
    
    for (const auto& [cameraNameStr, cameraData] : cameras_) {
        displayText += cameraData.lastInfo;
        displayText += "\n" + QString(60, '=') + "\n\n";
    }
    
    infoTextEdit_->setText(displayText);
}

QString CameraInfosDock::generateCameraInfo(const QString& cameraName, vidre::RuntimeInstance* runtimeInstance)
{
    if (!runtimeInstance) {
        return QString("Error: Invalid camera instance");
    }
    
    QString info;
    
    
    info += QString("<b>Camera: %1</b>\n").arg(cameraName);
    
    
    vidre::ICamera* camera = runtimeInstance->getCamera();
    
    if (camera) {
        
        std::string serialNumber = camera->GetSerialNumber();
        std::string vendorName = camera->GetVendorName();
        
        info += QString("Serial: %1\n").arg(QString::fromStdString(serialNumber));
        info += QString("Vendor: %1\n").arg(QString::fromStdString(vendorName));
        
        
        info += "\n";
        
        info += "<u>Frame Configuration:</u>\n";
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
        
        info += "<u>Acquisition Settings:</u>\n";
        info += QString("  Frame Rate: %1 fps\n")
            .arg(camera->GetFrameRate(), 0, 'f', 2);
        
        info += "\n";
        
        info += "<u>Recording Status:</u>\n";
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
