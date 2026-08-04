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

    void addCamera(const QString& cameraName, vidre::IRuntimeInstance* runtimeInstance);
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
        vidre::IRuntimeInstance* runtimeInstance;
        QString lastInfo;
        QGroupBox* groupBox;
        QLabel* infoLabel;
        QDoubleSpinBox* exposureSpin;
        QDoubleSpinBox* gainSpin;
        QPushButton* applyButton;
        bool controlsInitialized;
    };
    
    std::unordered_map<std::string, CameraData> cameras_;
    
    void refreshDisplay();
    QString generateCameraInfo(const QString& cameraName, vidre::IRuntimeInstance* runtimeInstance);
    QGroupBox* createCameraGroup(const QString& cameraName, CameraData& data);
    void applyCameraSettings(const QString& cameraName);
};

#endif // CAMERAINFOSDOCK_HPP
