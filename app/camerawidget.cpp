#include "camerawidget.hpp"
#include "ui_camerawidget.h"
#include "status_mapping.hpp"
#include <QCloseEvent>
#include <QKeyEvent>
#include <QDateTime>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>

#include <core/runtime/shared_frame_header.hpp>

CameraWidget::CameraWidget(const QString& cameraName, 
                           vidre::IRuntimeInstance* runtimeInstance,
                           QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CameraWidget)
    , cameraName_(cameraName)
    , runtimeInstance_(runtimeInstance)
    , frameTimer_(new QTimer(this))
    , lastFrameID_(-1)
    , frameCount_(0)
{
    ui->setupUi(this);
    setupUI();
    
    connect(frameTimer_, &QTimer::timeout, this, &CameraWidget::updateFrame);
    frameTimer_->setInterval(33); // ~30 FPS
    updateStatus();
}

CameraWidget::~CameraWidget()
{
    frameTimer_->stop();
    delete ui;
}

void CameraWidget::setupUI()
{
    setWindowTitle(cameraName_);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(3, 3, 3, 3);
    
    // Control buttons group
    QGroupBox *controlGroup = new QGroupBox("Camera Control", this);
    controlGroup->setStyleSheet(
        "QGroupBox {"
        "    background-color: #2b2b2b;"
        "    color: #bbbbbb;"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "    margin-top: 5px;"
        "    font-weight: bold;"
        "    font-size: 9pt;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 2px 0 2px;"
        "}"
    );
    
    QGridLayout *controlLayout = new QGridLayout(controlGroup);
    controlLayout->setSpacing(2);
    controlLayout->setContentsMargins(3, 3, 3, 3);
    
    startButton_ = new QPushButton("Record", this);
    stopButton_ = new QPushButton("Stop", this);
    showButton_ = new QPushButton("Show", this);
    undockButton_ = new QPushButton("Undock", this);
    
    QString buttonStyle = 
        "QPushButton {"
        "    background-color: #3c3f41;"
        "    color: #bbbbbb;"
        "    border: 1px solid #555555;"
        "    border-radius: 2px;"
        "    padding: 3px 5px;"
        "    font-size: 9pt;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4b4b4b;"
        "    border: 1px solid #6b6b6b;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2b2b2b;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #2b2b2b;"
        "    color: #666666;"
        "}";
    
    startButton_->setStyleSheet(buttonStyle);
    stopButton_->setStyleSheet(buttonStyle);
    showButton_->setStyleSheet(buttonStyle);
    undockButton_->setStyleSheet(buttonStyle);
    startButton_->setMaximumHeight(24);
    stopButton_->setMaximumHeight(24);
    showButton_->setMaximumHeight(24);
    undockButton_->setMaximumHeight(24);
    
    controlLayout->addWidget(startButton_, 0, 0);
    controlLayout->addWidget(showButton_, 0, 1);
    controlLayout->addWidget(stopButton_, 0, 2);
    controlLayout->addWidget(undockButton_, 0, 3);
    
    // Status label
    statusLabel_ = new QLabel("Status: Stopped", this);
    statusLabel_->setMaximumHeight(20);
    statusLabel_->setStyleSheet(
        "QLabel {"
        "    background-color: #1e1e1e;"
        "    color: #ff8800;"
        "    border: 1px solid #555555;"
        "    border-radius: 2px;"
        "    padding: 2px 5px;"
        "    font-size: 9pt;"
        "    font-weight: bold;"
        "}"
    );
    
    // Image display area
    QGroupBox *imageGroup = new QGroupBox("Image Display", this);
    imageGroup->setStyleSheet(controlGroup->styleSheet());
    
    QVBoxLayout *imageLayout = new QVBoxLayout(imageGroup);
    imageLayout->setSpacing(1);
    imageLayout->setContentsMargins(2, 4, 2, 2);
    imageLabel_ = new QLabel("No image", this);
    imageLabel_->setMinimumSize(320, 240);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setStyleSheet(
        "QLabel {"
        "    background-color: #1e1e1e;"
        "    color: #666666;"
        "    border: 1px solid #555555;"
        "}"
    );
    imageLayout->addWidget(imageLabel_);
    
    // Info label
    infoLabel_ = new QLabel("Camera Information", this);
    infoLabel_->setMaximumHeight(90);
    infoLabel_->setStyleSheet(
        "QLabel {"
        "    background-color: #1e1e1e;"
        "    color: #888888;"
        "    border: 1px solid #555555;"
        "    border-radius: 2px;"
        "    padding: 3px 4px;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 8pt;"
        "}"
    );
    
    mainLayout->addWidget(controlGroup, 0);
    mainLayout->addWidget(statusLabel_, 0);
    mainLayout->addWidget(imageGroup, 1);
    mainLayout->addWidget(infoLabel_, 0);
    
    setLayout(mainLayout);
    
    // Connect signals
    connect(startButton_, &QPushButton::clicked, this, &CameraWidget::onStartClicked);
    connect(stopButton_, &QPushButton::clicked, this, &CameraWidget::onStopClicked);
    connect(showButton_, &QPushButton::clicked, this, &CameraWidget::onShowClicked);
    connect(undockButton_, &QPushButton::clicked, this, &CameraWidget::onToggleDockClicked);
    
    // Set dark background
    setStyleSheet("CameraWidget { background-color: #2b2b2b; }");
}

void CameraWidget::updateStatus()
{
    if (!runtimeInstance_) {
        return;
    }
    
    bool isRunning = runtimeInstance_->isRunning();
    
    if (isRunning) {
        statusLabel_->setText("Status: Running");
        statusLabel_->setStyleSheet(
            "QLabel {"
            "    background-color: #1e1e1e;"
            "    color: #00cc00;"
            "    border: 1px solid #555555;"
            "    border-radius: 2px;"
            "    padding: 2px 5px;"
            "    font-size: 9pt;"
            "    font-weight: bold;"
            "}"
        );
        startButton_->setEnabled(false);
        showButton_->setEnabled(false);
        stopButton_->setEnabled(true);
        if (!frameTimer_->isActive()) {
            frameTimer_->start();
            lastFrameID_ = -1;
            frameCount_ = 0;
        }
    } else {
        statusLabel_->setText("Status: Stopped");
        statusLabel_->setStyleSheet(
            "QLabel {"
            "    background-color: #1e1e1e;"
            "    color: #ff8800;"
            "    border: 1px solid #555555;"
            "    border-radius: 2px;"
            "    padding: 2px 5px;"
            "    font-size: 9pt;"
            "    font-weight: bold;"
            "}"
        );
        startButton_->setEnabled(true);
        showButton_->setEnabled(true);
        stopButton_->setEnabled(false);
        if (frameTimer_->isActive()) {
            frameTimer_->stop();
        }
    }
    
    updateInfoDisplay();
}

void CameraWidget::updateInfoDisplay()
{
    QString info = QString("Camera: %1\n").arg(cameraName_);
    
    vidre::RecordingStatus status = runtimeInstance_->getRecordingStatus();
    info += QString("Status: %1\n").arg(status_mapping::toString(status));
    
    bool isActive = (status != vidre::RecordingStatus::Stopped);
    if (isActive) {
        info += QString("Frames Displayed: %1\n").arg(frameCount_);
        info += QString("Last Frame ID: %1\n").arg(lastFrameID_);
        
        int saved = runtimeInstance_->getSavedFramesNumber();
        int dropped = runtimeInstance_->getDroppedFrames();
        
        info += QString("Saved: %1 | Dropped: %2\n").arg(saved).arg(dropped);
        
        unsigned int width = runtimeInstance_->getFrameWidth();
        unsigned int height = runtimeInstance_->getFrameHeight();
        info += QString("Resolution: %1 x %2\n").arg(width).arg(height);
    }
    
    info += QString("Last Update: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
    
    infoLabel_->setText(info);
}

void CameraWidget::onStartClicked()
{
    if (runtimeInstance_ && !runtimeInstance_->isRunning()) {
        runtimeInstance_->run();
    } 
    if (!frameTimer_->isActive()) {
        frameTimer_->start();
        lastFrameID_ = -1;
        frameCount_ = 0;
        updateStatus();
    }
}

void CameraWidget::onShowClicked()
{
    if (runtimeInstance_ && !runtimeInstance_->isRunning()) {
        runtimeInstance_->show();
    } 
    if (!frameTimer_->isActive()) {
        frameTimer_->start();
        lastFrameID_ = -1;
        frameCount_ = 0;
        updateStatus();
    }
}

void CameraWidget::onStopClicked()
{
    if (runtimeInstance_ && runtimeInstance_->isRunning()) {
        runtimeInstance_->stop();
        frameTimer_->stop();
        updateStatus();
    }
}

void CameraWidget::updateFrame()
{
    if (!runtimeInstance_) {
        return;
    }
    
    // Check if camera is running or streaming
    vidre::RecordingStatus status = runtimeInstance_->getRecordingStatus();
    bool isActive = (status == vidre::RecordingStatus::RecordingFine || 
                     status == vidre::RecordingStatus::RecordingDroppedFrames ||
                     status == vidre::RecordingStatus::RecordingError ||
                     status == vidre::RecordingStatus::Streaming);
    
    if (!isActive) {
        return;
    }
    

    
    const uint8_t* imageData;
    unsigned int width; 
    unsigned int height;
    unsigned int frameID;
    std::tie(imageData, width, height, frameID) = runtimeInstance_->getLatestFrame(); 
    auto frameBytes = runtimeInstance_->getFrameBytes();
    if (frameID == lastFrameID_) {
        return; 
    }
    
    lastFrameID_ = frameID;
    
    if (width == 0 || height == 0 || frameBytes == 0) {
        return;
    }
    
    auto bitsPerPixel = runtimeInstance_->getBitsPerPixel();
    displayFrame(imageData, width, height, bitsPerPixel, frameBytes);
    
    updateInfoDisplay();
}

void CameraWidget::displayFrame(const uint8_t* imageData, uint64_t width, uint64_t height, uint64_t bitsPerPixel, uint64_t frameBytes)
{
    if (!imageData || width == 0 || height == 0) {
        return;
    }
    
    QImage image;

    if (bitsPerPixel == 8) {
        image = QImage(imageData, width, height, width, QImage::Format_Grayscale8);
    } else if (bitsPerPixel == 10) {
        // 10-bit packed: 4 pixels per 5 bytes. Unpack to 16-bit grayscale for display.
        // Each pixel is 10 bits; 4 pixels fit in 5 bytes (32 bits / 4 px).
        // We unpack to Format_Grayscale16, scaling 10-bit to 16-bit.
        const size_t numPixels = width * height;
        QImage unpacked(numPixels > 0 ? static_cast<int>(width) : 0,
                        static_cast<int>(height),
                        QImage::Format_Grayscale16);
        if (unpacked.isNull()) {
            return;
        }
        // 10-bit packed format: 4 pixels in 5 bytes (little-endian).
        // Byte layout for 4 pixels: [b0 b1 b2 b3 b4]
        // p0 = b0 | ((b1 & 0x03) << 8)   (10 bits)
        // p1 = (b1 >> 2) | ((b2 & 0x0F) << 6)
        // p2 = (b2 >> 4) | ((b3 & 0x3F) << 4)
        // p3 = (b3 >> 6) | (b4 << 2)
        const uint8_t* src = imageData;
        uint16_t* dst = reinterpret_cast<uint16_t*>(unpacked.bits());
        const size_t numGroups = numPixels / 4;
        const uint16_t scale10to16 = 65535 / 1023; // scale 10-bit (0-1023) to 16-bit (0-65535)
        for (size_t i = 0; i < numGroups; ++i) {
            uint8_t b0 = src[0], b1 = src[1], b2 = src[2], b3 = src[3], b4 = src[4];
            dst[0] = static_cast<uint16_t>((b0 | ((b1 & 0x03) << 8)) * scale10to16);
            dst[1] = static_cast<uint16_t>(((b1 >> 2) | ((b2 & 0x0F) << 6)) * scale10to16);
            dst[2] = static_cast<uint16_t>(((b2 >> 4) | ((b3 & 0x3F) << 4)) * scale10to16);
            dst[3] = static_cast<uint16_t>(((b3 >> 6) | (b4 << 2)) * scale10to16);
            src += 5;
            dst += 4;
        }
        // Handle remaining pixels (if numPixels % 4 != 0)
        size_t remaining = numPixels % 4;
        if (remaining > 0 && src + 4 < imageData + frameBytes) {
            uint8_t b0 = src[0], b1 = src[1], b2 = src[2], b3 = src[3], b4 = (src + 4 < imageData + frameBytes) ? src[4] : 0;
            if (remaining > 0) dst[0] = static_cast<uint16_t>((b0 | ((b1 & 0x03) << 8)) * scale10to16);
            if (remaining > 1) dst[1] = static_cast<uint16_t>(((b1 >> 2) | ((b2 & 0x0F) << 6)) * scale10to16);
            if (remaining > 2) dst[2] = static_cast<uint16_t>(((b2 >> 4) | ((b3 & 0x3F) << 4)) * scale10to16);
            if (remaining > 3) dst[3] = static_cast<uint16_t>(((b3 >> 6) | (b4 << 2)) * scale10to16);
        }
        image = unpacked;
    } else {
        return;
    }
    
    QPixmap pixmap = QPixmap::fromImage(image);
    QPixmap scaled = pixmap.scaled(imageLabel_->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
    
    imageLabel_->setPixmap(scaled);
    
    frameCount_++;
}

void CameraWidget::onToggleDockClicked()
{
    QMdiSubWindow *subWindow = qobject_cast<QMdiSubWindow*>(parentWidget());
    
    if (subWindow) {
        subWindow->setWidget(nullptr);
        subWindow->hide();
        
        setParent(nullptr);
        setWindowTitle(cameraName_);
        setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
        
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->geometry();
            int x = screenGeometry.x() + (screenGeometry.width() - width()) / 2;
            int y = screenGeometry.y() + (screenGeometry.height() - height()) / 2;
            move(x, y);
        }
        
        show();
        raise();
        activateWindow();
        setFocus();
        
        undockButton_->setText("Dock");
    } else {
        QMdiArea *mdiArea = nullptr;
        
        QWidget *widget = qobject_cast<QWidget*>(sender());
        while (widget) {
            QMainWindow *mainWin = qobject_cast<QMainWindow*>(widget);
            if (mainWin) {
                mdiArea = qobject_cast<QMdiArea*>(mainWin->centralWidget());
                break;
            }
            widget = widget->parentWidget();
        }
        
        if (!mdiArea) {
            for (QWidget *w : QApplication::topLevelWidgets()) {
                QMainWindow *mainWin = qobject_cast<QMainWindow*>(w);
                if (mainWin) {
                    mdiArea = qobject_cast<QMdiArea*>(mainWin->centralWidget());
                    if (mdiArea) break;
                }
            }
        }
        
        if (mdiArea) {
            subWindow = new QMdiSubWindow();
            subWindow->setWidget(this);
            subWindow->setWindowTitle(cameraName_);
            subWindow->setAttribute(Qt::WA_DeleteOnClose);
            subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            
            mdiArea->addSubWindow(subWindow);
            
            hide();
            setParent(subWindow);
            setWindowFlags(Qt::Widget);
            
            subWindow->show();
            mdiArea->setActiveSubWindow(subWindow);
            show();
            
            undockButton_->setText("Undock");
        }
    }
}

void CameraWidget::closeEvent(QCloseEvent *event)
{
    emit windowClosed(cameraName_);
    event->accept();
}

void CameraWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    emit windowActivated(cameraName_);
}

void CameraWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    emit windowActivated(cameraName_);
}

void CameraWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Space:
            if (runtimeInstance_) {
                if (runtimeInstance_->isRunning()) {
                    onStopClicked();
                } else {
                    onStartClicked();
                }
            }
            event->accept();
            break;
        case Qt::Key_Escape:
            close();
            event->accept();
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}
