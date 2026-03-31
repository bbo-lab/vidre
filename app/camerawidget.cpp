#include "camerawidget.hpp"
#include "ui_camerawidget.h"
#include <QCloseEvent>
#include <QDateTime>

#include "../src/core/runtime/shared_frame_header.hpp"

CameraWidget::CameraWidget(const QString& cameraName, 
                           vidre::RuntimeInstance* runtimeInstance,
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
    updateStatus();
    
    connect(frameTimer_, &QTimer::timeout, this, &CameraWidget::updateFrame);
    frameTimer_->setInterval(33); // ~30 FPS
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
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // Control buttons group
    QGroupBox *controlGroup = new QGroupBox("Camera Control", this);
    controlGroup->setStyleSheet(
        "QGroupBox {"
        "    background-color: #2b2b2b;"
        "    color: #bbbbbb;"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "    margin-top: 10px;"
        "    font-weight: bold;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 3px 0 3px;"
        "}"
    );
    
    QGridLayout *controlLayout = new QGridLayout(controlGroup);
    
    startButton_ = new QPushButton("Record", this);
    stopButton_ = new QPushButton("Stop", this);
    showButton_ = new QPushButton("Show", this);
    singleGrabButton_ = new QPushButton("Single Frame", this);
    
    QString buttonStyle = 
        "QPushButton {"
        "    background-color: #3c3f41;"
        "    color: #bbbbbb;"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "    padding: 8px;"
        "    font-size: 11pt;"
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
    // singleGrabButton_->setStyleSheet(buttonStyle);
    
    controlLayout->addWidget(startButton_, 0, 0);
    controlLayout->addWidget(showButton_, 0, 1);
    controlLayout->addWidget(stopButton_, 0, 2);
    // controlLayout->addWidget(singleGrabButton_, 1, 0, 1, 2);
    
    // Status label
    statusLabel_ = new QLabel("Status: Stopped", this);
    statusLabel_->setStyleSheet(
        "QLabel {"
        "    background-color: #1e1e1e;"
        "    color: #ff8800;"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "    padding: 8px;"
        "    font-size: 11pt;"
        "    font-weight: bold;"
        "}"
    );
    
    // Image display area
    QGroupBox *imageGroup = new QGroupBox("Image Display", this);
    imageGroup->setStyleSheet(controlGroup->styleSheet());
    
    QVBoxLayout *imageLayout = new QVBoxLayout(imageGroup);
    imageLabel_ = new QLabel("No image", this);
    imageLabel_->setMinimumSize(640, 480);
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
    infoLabel_->setStyleSheet(
        "QLabel {"
        "    background-color: #1e1e1e;"
        "    color: #bbbbbb;"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "    padding: 5px;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "}"
    );
    
    mainLayout->addWidget(controlGroup);
    mainLayout->addWidget(statusLabel_);
    mainLayout->addWidget(imageGroup, 1);
    mainLayout->addWidget(infoLabel_);
    
    setLayout(mainLayout);
    
    // Connect signals
    connect(startButton_, &QPushButton::clicked, this, &CameraWidget::onStartClicked);
    connect(stopButton_, &QPushButton::clicked, this, &CameraWidget::onStopClicked);
    connect(showButton_, &QPushButton::clicked, this, &CameraWidget::onShowClicked);
    // connect(singleGrabButton_, &QPushButton::clicked, this, &CameraWidget::onSingleGrabClicked);
    
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
            "    border-radius: 3px;"
            "    padding: 8px;"
            "    font-size: 11pt;"
            "    font-weight: bold;"
            "}"
        );
        startButton_->setEnabled(false);
        showButton_->setEnabled(false);
        stopButton_->setEnabled(true);
        // singleGrabButton_->setEnabled(false);
    } else {
        statusLabel_->setText("Status: Stopped");
        statusLabel_->setStyleSheet(
            "QLabel {"
            "    background-color: #1e1e1e;"
            "    color: #ff8800;"
            "    border: 1px solid #555555;"
            "    border-radius: 3px;"
            "    padding: 8px;"
            "    font-size: 11pt;"
            "    font-weight: bold;"
            "}"
        );
        startButton_->setEnabled(true);
        showButton_->setEnabled(true);
        stopButton_->setEnabled(false);
        // singleGrabButton_->setEnabled(true);
    }
    
    updateInfoDisplay();
}

void CameraWidget::updateInfoDisplay()
{
    QString info = QString("Camera: %1\n").arg(cameraName_);
    
    vidre::RecordingStatus status = runtimeInstance_->getRecordingStatus();
    QString statusStr;
    switch (status) {
        case vidre::RecordingStatus::Stopped:
            statusStr = "Stopped";
            break;
        case vidre::RecordingStatus::Streaming:
            statusStr = "Streaming";
            break;
        case vidre::RecordingStatus::RecordingFine:
            statusStr = "Recording";
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
    
    info += QString("Status: %1\n").arg(statusStr);
    
    bool isActive = (status != vidre::RecordingStatus::Stopped);
    if (isActive) {
        info += QString("Frames Displayed: %1\n").arg(frameCount_);
        info += QString("Last Frame ID: %1\n").arg(lastFrameID_);
        
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

void CameraWidget::onSingleGrabClicked()
{
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
    
    const auto [imageData, frameID] = runtimeInstance_->getLatestFrame(); 
    
    if (frameID == lastFrameID_) {
        return; 
    }
    
    lastFrameID_ = frameID;
    
    // Get frame dimensions
    unsigned int width = runtimeInstance_->getFrameWidth();
    unsigned int height = runtimeInstance_->getFrameHeight();
    unsigned int frameBytes = runtimeInstance_->getFrameBytes();
    
    if (width == 0 || height == 0 || frameBytes == 0) {
        return;
    }
    
    auto bitsPerPixel = runtimeInstance_->getBitsPerPixel();
    displayFrame(imageData, width, height, bitsPerPixel, frameBytes);
    
    updateInfoDisplay();
}

void unpack_10bit_to_8bit(const uint8_t *data_in, size_t nbytes_in, uint8_t *data_out) {
    int nsamples_in = floor(nbytes_in * 8.0 / 10.0);
    size_t nblocks = nsamples_in / 4;
    
    for (size_t i = 0; i < nblocks; ++i) {
        size_t in_idx = i * 5;
        size_t out_idx = i * 4;

        const uint8_t* src = &data_in[in_idx];
        uint8_t* dst = &data_out[out_idx];

        uint16_t s0 = src[0] | ((src[1] & 0x03) << 8);
        dst[0] = s0 >> 2;

        uint16_t s1 = (src[1] >> 2) | ((src[2] & 0x0F) << 6);
        dst[1] = s1 >> 2;

        uint16_t s2 = (src[2] >> 4) | ((src[3] & 0x3F) << 4);
        dst[2] = s2 >> 2;

        uint16_t s3 = (src[3] >> 6) | (src[4] << 2);
        dst[3] = s3 >> 2;
    }

    size_t remaining_samples = nsamples_in - (nblocks * 4);
    if (remaining_samples > 0) {
        size_t in_idx = nblocks * 5;
        size_t out_idx = nblocks * 4;

        const uint8_t* src = &data_in[in_idx];
        uint8_t* dst = &data_out[out_idx];

        uint16_t s0 = src[0] | ((src[1] & 0x03) << 8);
        dst[0] = s0 >> 2;

        if (remaining_samples > 1) {
            uint16_t s1 = (src[1] >> 2) | ((src[2] & 0x0F) << 6);
            dst[1] = s1 >> 2;

            if (remaining_samples > 2) {
                uint16_t s2 = (src[2] >> 4) | ((src[3] & 0x3F) << 4);
                dst[2] = s2 >> 2;
            }
        }
    }
}

void CameraWidget::displayFrame(const uint8_t* imageData, uint64_t width, uint64_t height, uint64_t bitsPerPixel, uint64_t frameBytes)
{
    if (!imageData || width <= 0 || height <= 0) {
        return;
    }
    
    QImage::Format format;
    QImage image;
    uint8_t* unpacked_buffer = nullptr;

    if (bitsPerPixel == 8) {
        format = QImage::Format_Grayscale8;
        image = QImage(imageData, width, height, format);
    } else if (bitsPerPixel == 10) {
        format = QImage::Format_Grayscale8;
        int nsamples_in = floor(frameBytes * 8.0 / 10.0);
        unpacked_buffer = new uint8_t[nsamples_in];
        
        unpack_10bit_to_8bit(imageData, frameBytes, unpacked_buffer);
        
        image = QImage(unpacked_buffer, width, height, format);
    } else {
        return; // Unsupported format
    }
    
    // Scale to fit the label while maintaining aspect ratio
    QPixmap pixmap = QPixmap::fromImage(image);
    QPixmap scaled = pixmap.scaled(imageLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    imageLabel_->setPixmap(scaled);
    
    if (unpacked_buffer) {
        delete[] unpacked_buffer;
    }
    
    frameCount_++;
}

void CameraWidget::closeEvent(QCloseEvent *event)
{
    emit windowClosed(cameraName_);
    event->accept();
}
