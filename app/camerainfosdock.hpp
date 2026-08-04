#ifndef CAMERAINFOSDOCK_HPP
#define CAMERAINFOSDOCK_HPP

#include <QDockWidget>
#include <QTimer>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGroupBox>
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
    QWidget *contentWidget_;
    QScrollArea *scrollArea_;
    QVBoxLayout *camerasLayout_;
    QLabel *emptyLabel_;
    QTimer *updateTimer_;
    
    struct CameraData {
        vidre::RuntimeInstance* runtimeInstance = nullptr;
        QString lastInfo;
        QGroupBox* groupBox = nullptr;
        QLabel* infoLabel = nullptr;
        QDoubleSpinBox* exposureSpin = nullptr;
        QDoubleSpinBox* gainSpin = nullptr;
        QPushButton* applyButton = nullptr;
        bool controlsInitialized = false;

        QString serial;
        QString vendor;
        unsigned int width = 0;
        unsigned int height = 0;
        unsigned int offsetX = 0;
        unsigned int offsetY = 0;
        unsigned int frameBytes = 0;
        unsigned int bufferSize = 0;
        unsigned int maxNumBuffers = 0;
        double frameRate = 0.0;
        double exposure = 0.0;
        double gain = 0.0;
    };
    
    std::unordered_map<std::string, CameraData> cameras_;
    
    void refreshDisplay();
    void snapshotCameraParams(const QString& cameraName, CameraData& data);
    QString generateCameraInfo(const QString& cameraName, const CameraData& data);
    QGroupBox* createCameraGroup(const QString& cameraName, CameraData& data);
    void applyCameraSettings(const QString& cameraName);
};

#endif // CAMERAINFOSDOCK_HPP
