#ifndef CAMERAWIDGET_HPP
#define CAMERAWIDGET_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <memory>

#include "../src/core/runtime/runtime.hpp"

namespace Ui {
class CameraWidget;
}

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(const QString& cameraName, 
                         vidre::RuntimeInstance* runtimeInstance,
                         QWidget *parent = nullptr);
    ~CameraWidget();

    QString getCameraName() const { return cameraName_; }
    void updateStatus();

signals:
    void windowClosed(const QString& cameraName);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onStartClicked();
    void onStopClicked();
    void onSingleGrabClicked();
    void updateFrame();

private:
    Ui::CameraWidget *ui;
    QString cameraName_;
    vidre::RuntimeInstance* runtimeInstance_;
    
    QPushButton *startButton_;
    QPushButton *stopButton_;
    QPushButton *singleGrabButton_;
    QLabel *statusLabel_;
    QLabel *imageLabel_;
    QLabel *infoLabel_;
    QTimer *frameTimer_;
    
    int64_t lastFrameID_;
    int frameCount_;
    
    void setupUI();
    void updateInfoDisplay();
    void displayFrame(const uint8_t* imageData, int width, int height, int bytesPerPixel);
};

#endif // CAMERAWIDGET_HPP
