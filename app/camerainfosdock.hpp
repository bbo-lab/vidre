#ifndef CAMERAINFOSDOCK_HPP
#define CAMERAINFOSDOCK_HPP

#include <QDockWidget>
#include <QTextEdit>
#include <QTimer>
#include <QLabel>
#include <memory>
#include <unordered_map>
#include <string>

#include <core/runtime/runtime.hpp>

class CameraInfosDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit CameraInfosDock(QWidget *parent = nullptr);
    ~CameraInfosDock();

    void addCamera(const QString& cameraName, vidre::RuntimeInstance* runtimeInstance);
    void removeCamera(const QString& cameraName);
    void updateCameraInfo(const QString& cameraName);
    void clearAllCameras();

private slots:
    void updateAllCamerasInfo();

private:
    QTextEdit *infoTextEdit_;
    QTimer *updateTimer_;
    
    struct CameraData {
        vidre::RuntimeInstance* runtimeInstance;
        QString lastInfo;
    };
    
    std::unordered_map<std::string, CameraData> cameras_;
    
    void refreshDisplay();
    QString generateCameraInfo(const QString& cameraName, vidre::RuntimeInstance* runtimeInstance);
};

#endif // CAMERAINFOSDOCK_HPP
