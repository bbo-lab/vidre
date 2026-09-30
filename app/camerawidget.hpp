#ifndef CAMERAWIDGET_HPP
#define CAMERAWIDGET_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QShowEvent>
#include <QFocusEvent>
#include <memory>

#include <core/runtime/runtime.hpp>

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
    void windowActivated(const QString& cameraName);
    void recordingStateChanged();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onStartClicked();
    void onStopClicked();
    void onShowClicked();
    void onSingleGrabClicked();
    void onToggleDockClicked();
    void updateFrame();

private:
    Ui::CameraWidget *ui;
    QString cameraName_;
    vidre::RuntimeInstance* runtimeInstance_;
    
    QPushButton *startButton_;
    QPushButton *stopButton_;
    QPushButton *showButton_;
    QPushButton *singleGrabButton_;
    QPushButton *undockButton_;
    QLabel *statusLabel_;
    QLabel *imageLabel_;
    QPlainTextEdit *infoText_;
    QTimer *frameTimer_;
    QPixmap lastPixmap_;

    int64_t lastFrameID_;
    int frameCount_;

    void setupUI();
    void showScaledFrame();
    void updateInfoDisplay();
    void displayFrame(const uint8_t* imageData, uint64_t width, uint64_t height, uint64_t bitsPerPixel, uint64_t frameBytes);
};

#endif // CAMERAWIDGET_HPP
