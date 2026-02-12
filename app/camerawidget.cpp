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
    // TODO show only cam/system 
    // TODO record system / stop system
    stopButton_ = new QPushButton("Stop", this);
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
    // singleGrabButton_->setStyleSheet(buttonStyle);
    
    controlLayout->addWidget(startButton_, 0, 0);
    controlLayout->addWidget(stopButton_, 0, 1);
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
        stopButton_->setEnabled(false);
        // singleGrabButton_->setEnabled(true);
    }
    
    updateInfoDisplay();
}

void CameraWidget::updateInfoDisplay()
{
    QString info = QString("Camera: %1\n").arg(cameraName_);
    info += QString("Status: %1\n").arg(runtimeInstance_->isRunning() ? "Running" : "Stopped");
    
    if (runtimeInstance_->isRunning()) {
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
    if (!runtimeInstance_ || !runtimeInstance_->isRunning()) {
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

void pack_bits_16_10(const uint8_t *data_in, size_t nbytes_in, uint8_t *data_out, size_t buffersize_out) {
    int nsamples_in = floor(nbytes_in * 8.0 / 10.0);

    size_t nblocks = nsamples_in / 4;
    
    for (size_t i = 0; i < nblocks; ++i) {
        size_t in_idx = i * 5;
        size_t out_idx = i * 8; // 4 samples * 2 bytes

        const uint8_t* src = &data_in[in_idx];
        uint8_t* dst = &data_out[out_idx];

        dst[0] = src[0];
        dst[1] = src[1] & 0x03;

        dst[2] = (src[1] >> 2) | ((src[2] & 0x03) << 6);
        dst[3] = (src[2] >> 2) & 0x03;

        dst[4] = (src[2] >> 4) | (src[3] << 4);
        dst[5] = (src[3] >> 4) & 0x03;

        dst[6] = (src[3] >> 6) | (src[4] << 2);
        dst[7] = src[4] >> 6;
    }

    size_t remaining_samples = nsamples_in - (nblocks * 4);

    if (remaining_samples > 0) {
        size_t in_idx = nblocks * 5;
        size_t out_idx = nblocks * 8;

        const uint8_t* src = &data_in[in_idx];
        uint8_t* dst = &data_out[out_idx];

        // Sample 0 logic
        dst[0] = src[0];
        dst[1] = src[1] & 0x03;

        if (remaining_samples > 1) {
            // Sample 1 logic
            dst[2] = (src[1] >> 2) | ((src[2] & 0x03) << 6);
            dst[3] = (src[2] >> 2) & 0x03;

            if (remaining_samples > 2) {
                // Sample 2 logic
                dst[4] = (src[2] >> 4) | (src[3] << 4);
                dst[5] = (src[3] >> 4) & 0x03;
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
    if (bitsPerPixel == 8) {
        format = QImage::Format_Grayscale8;
        image = QImage(imageData, width, height, format);
    } else if (bitsPerPixel == 10) {
        format = QImage::Format_Grayscale8;
        int nsamples_in = floor(frameBytes * 8.0 / 10.0);
        int bytes_per_sample_unpacked = 2;
        int nbytes_out = bytes_per_sample_unpacked * nsamples_in;
        // // std::cout << "QT: " << frameBytes << " " << buffer_size << std::endl;
        uint8_t* packed_buffer = new uint8_t[nbytes_out];
        for (int i = 0; i < nbytes_out; ++i) {
            packed_buffer[i] =  ((uint64_t)imageData[i]) * 255 / 1024.0;
        }
        // pack_bits_16_10(imageData, frameBytes, packed_buffer, nbytes_out);
        image = QImage(imageData, width, height, format);
    } else {
        return; // Unsupported format
    }
    
    
    // Scale to fit the label while maintaining aspect ratio
    QPixmap pixmap = QPixmap::fromImage(image);
    QPixmap scaled = pixmap.scaled(imageLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    imageLabel_->setPixmap(scaled);
    
    frameCount_++;
}

void CameraWidget::closeEvent(QCloseEvent *event)
{
    emit windowClosed(cameraName_);
    event->accept();
}
