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
    
    if (width == 0 || height == 0) {
        return;
    }
    
    auto bitsPerPixel = runtimeInstance_->getBitsPerPixel();
    displayFrame(imageData, width, height, bitsPerPixel);
    
    updateInfoDisplay();
}

inline int pack_bits_16_10(const uint8_t *data_in, size_t nbytes_in, uint8_t *data_out, size_t buffersize_out) {
    size_t  byte_offset_in, byte_offset_out, buffersize_out_in_samples, nsamples, remaining_samples;
    buffersize_out_in_samples = (size_t) floor(((double) buffersize_out) * 0.8);
    nsamples = nbytes_in / 2; //rounds down
    size_t n = std::min(nsamples, buffersize_out_in_samples);
    
    size_t nblocks = n / 4; //rounds down
    
    byte_offset_out = 0;
    byte_offset_in = 0;
    for (size_t jj = 0; jj < nblocks; jj++) {
        data_out[byte_offset_out    ] = data_in[byte_offset_in];
        data_out[byte_offset_out + 1] = data_in[byte_offset_in + 1] | (data_in[byte_offset_in + 2] << 2);
        data_out[byte_offset_out + 2] = (data_in[byte_offset_in + 2] >> 6) | (data_in[byte_offset_in + 3] << 2) | (data_in[byte_offset_in + 4] << 4);
        data_out[byte_offset_out + 3] = (data_in[byte_offset_in + 4] >> 4) | (data_in[byte_offset_in + 5] << 4) | (data_in[byte_offset_in + 6] << 6);
        data_out[byte_offset_out + 4] = (data_in[byte_offset_in + 6] >> 2) | (data_in[byte_offset_in + 7] << 6);
        byte_offset_out += 5;
        byte_offset_in += 8;
    }
    remaining_samples = n - 4 * nblocks;

    int nb_written = byte_offset_out;
    if (remaining_samples > 0) {
        data_out[byte_offset_out] = data_in[byte_offset_in];
        data_out[byte_offset_out + 1] = data_in[byte_offset_in + 1];
        nb_written += 2;
        if (remaining_samples > 1) {
            data_out[byte_offset_out + 1] |= (data_in[byte_offset_in + 2] << 2);
            data_out[byte_offset_out + 2] = (data_in[byte_offset_in + 2] >> 6) | (data_in[byte_offset_in + 3] << 2);
            nb_written++;
            if (remaining_samples > 2) {
                data_out[byte_offset_out + 2] |= (data_in[byte_offset_in + 4] << 4);
                data_out[byte_offset_out + 3] = (data_in[byte_offset_in + 4] >> 4) | (data_in[byte_offset_in + 5] << 4);
                nb_written++;
            }
        }
    }

    return nb_written;
}

void CameraWidget::displayFrame(const uint8_t* imageData, int width, int height, int bitsPerPixel)
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
        format = QImage::Format_Grayscale16;
        auto buffer_size =  (uint64_t) ceil((bitsPerPixel / 8.0) * width * height);
        uint8_t* packed_buffer = new uint8_t[buffer_size];
        pack_bits_16_10(imageData, buffer_size, packed_buffer, buffer_size);
        image = QImage(packed_buffer, width, height, format);
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
