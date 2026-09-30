#include "camerawidget.hpp"
#include "ui_camerawidget.h"
#include <QCloseEvent>
#include <QDateTime>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QScrollBar>

#include <core/runtime/shared_frame_header.hpp>

namespace {
    QString statusStyle(const char* color)
    {
        return QString(
            "QLabel {"
            "    background-color: #1e1e1e;"
            "    color: %1;"
            "    border: 1px solid #555555;"
            "    border-radius: 2px;"
            "    padding: 1px 5px;"
            "    font-size: 8pt;"
            "    font-weight: bold;"
            "}").arg(color);
    }
}

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
    mainLayout->setContentsMargins(2, 2, 2, 2);
    
    startButton_ = new QPushButton("Record", this);
    stopButton_ = new QPushButton("Stop", this);
    showButton_ = new QPushButton("Show", this);
    singleGrabButton_ = new QPushButton("Single Frame", this);
    singleGrabButton_->hide();
    undockButton_ = new QPushButton("Undock", this);
    
    const QString buttonStyle = 
        "QPushButton {"
        "    background-color: #3c3f41;"
        "    color: #bbbbbb;"
        "    border: 1px solid #555555;"
        "    border-radius: 2px;"
        "    padding: 1px 6px;"
        "    font-size: 8pt;"
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
    
    for (QPushButton *button : {startButton_, showButton_, stopButton_, undockButton_}) {
        button->setStyleSheet(buttonStyle);
        button->setFixedHeight(20);
    }
    
    statusLabel_ = new QLabel("Status: Stopped", this);
    statusLabel_->setFixedHeight(20);
    statusLabel_->setStyleSheet(statusStyle("#ff8800"));
    
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(2);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(startButton_);
    controlLayout->addWidget(showButton_);
    controlLayout->addWidget(stopButton_);
    controlLayout->addWidget(undockButton_);
    controlLayout->addStretch(1);
    vidre::ICamera *camera = runtimeInstance_ ? runtimeInstance_->getCamera() : nullptr;
    if (camera && (camera->triggerInSet || camera->triggerOutSet)) {
        QString triggerText = "TRIGGER IN";
        QString triggerColor = "#33aaff";
        if (camera->triggerOutSet) {
            triggerText = camera->triggerInSet ? "TRIGGER IN + OUT" : "TRIGGER OUT";
            triggerColor = "#cc66ff";
        }
        QLabel *triggerLabel = new QLabel(triggerText, this);
        triggerLabel->setFixedHeight(20);
        triggerLabel->setStyleSheet(QString(
            "QLabel {"
            "    background-color: %1;"
            "    color: #1e1e1e;"
            "    border-radius: 2px;"
            "    padding: 1px 5px;"
            "    font-size: 8pt;"
            "    font-weight: bold;"
            "}").arg(triggerColor));
        controlLayout->addWidget(triggerLabel);
    }
    controlLayout->addWidget(statusLabel_);
    
    imageLabel_ = new QLabel("No image", this);
    imageLabel_->setMinimumSize(320, 240);
    imageLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setStyleSheet(
        "QLabel {"
        "    background-color: #1e1e1e;"
        "    color: #666666;"
        "    border: 1px solid #555555;"
        "}"
    );
    imageLabel_->installEventFilter(this);
    
    infoText_ = new QPlainTextEdit(this);
    infoText_->setReadOnly(true);
    infoText_->setMinimumHeight(24);
    infoText_->setStyleSheet(
        "QPlainTextEdit {"
        "    background-color: #1e1e1e;"
        "    color: #888888;"
        "    border: 1px solid #555555;"
        "    border-radius: 2px;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 8pt;"
        "}"
    );
    
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->setHandleWidth(4);
    splitter->setStyleSheet("QSplitter::handle { background-color: #3c3f41; }");
    splitter->addWidget(imageLabel_);
    splitter->addWidget(infoText_);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, true);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setSizes({10000, 48});
    
    mainLayout->addLayout(controlLayout, 0);
    mainLayout->addWidget(splitter, 1);
    
    setLayout(mainLayout);
    
    connect(startButton_, &QPushButton::clicked, this, &CameraWidget::onStartClicked);
    connect(stopButton_, &QPushButton::clicked, this, &CameraWidget::onStopClicked);
    connect(showButton_, &QPushButton::clicked, this, &CameraWidget::onShowClicked);
    connect(undockButton_, &QPushButton::clicked, this, &CameraWidget::onToggleDockClicked);
    
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
        statusLabel_->setStyleSheet(statusStyle("#00cc00"));
        startButton_->setEnabled(false);
        showButton_->setEnabled(false);
        stopButton_->setEnabled(true);
        if (!frameTimer_->isActive()) {
            frameTimer_->start();
            lastFrameID_ = -1;
            frameCount_ = 0;
        }
        // singleGrabButton_->setEnabled(false);
    } else {
        statusLabel_->setText("Status: Stopped");
        statusLabel_->setStyleSheet(statusStyle("#ff8800"));
        startButton_->setEnabled(true);
        showButton_->setEnabled(true);
        stopButton_->setEnabled(false);
        if (frameTimer_->isActive()) {
            frameTimer_->stop();
        }
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
    
    vidre::ICamera *camera = runtimeInstance_->getCamera();
    if (camera && camera->triggerInSet) {
        info += "Trigger: IN\n";
    }
    if (camera && camera->triggerOutSet) {
        info += "Trigger: OUT\n";
    }
    
    bool isActive = (status != vidre::RecordingStatus::Stopped);
    if (isActive) {
        info += QString("Frames Displayed: %1\n").arg(frameCount_);
        info += QString("Last Frame ID: %1\n").arg(lastFrameID_);
        
        unsigned int width = runtimeInstance_->getFrameWidth();
        unsigned int height = runtimeInstance_->getFrameHeight();
        info += QString("Resolution: %1 x %2\n").arg(width).arg(height);
    }
    
    info += QString("Last Update: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
    
    QScrollBar *scrollBar = infoText_->verticalScrollBar();
    const int scrollPosition = scrollBar->value();
    infoText_->setPlainText(info);
    scrollBar->setValue(scrollPosition);
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
    emit recordingStateChanged();
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
    emit recordingStateChanged();
}

void CameraWidget::onStopClicked()
{
    if (runtimeInstance_ && runtimeInstance_->isRunning()) {
        runtimeInstance_->stop();
        frameTimer_->stop();
        updateStatus();
    }
    emit recordingStateChanged();
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
        image = QImage(imageData, width, height, QImage::Format_RGB32);
    } else {
        return; // Unsupported format
    }
    
    lastPixmap_ = QPixmap::fromImage(image);
    showScaledFrame();
    
    if (unpacked_buffer) {
        delete[] unpacked_buffer;
    }
    
    frameCount_++;
}

void CameraWidget::showScaledFrame()
{
    if (lastPixmap_.isNull()) {
        return;
    }
    imageLabel_->setPixmap(lastPixmap_.scaled(imageLabel_->contentsRect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

bool CameraWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == imageLabel_ && event->type() == QEvent::Resize) {
        showScaledFrame();
    }
    return QWidget::eventFilter(watched, event);
}

void CameraWidget::onToggleDockClicked()
{
    QMdiSubWindow *subWindow = qobject_cast<QMdiSubWindow*>(parentWidget());
    
    if (subWindow) {
        subWindow->setWidget(nullptr);
        subWindow->deleteLater();

        setParent(nullptr);
        setWindowTitle(cameraName_);
        setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
        setAttribute(Qt::WA_DeleteOnClose, true);
        
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
            subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

            mdiArea->addSubWindow(subWindow);

            hide();
            setAttribute(Qt::WA_DeleteOnClose, false);
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
    frameTimer_->stop();
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
