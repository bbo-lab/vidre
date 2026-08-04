#ifndef DEVICELISTDOCK_HPP
#define DEVICELISTDOCK_HPP

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <unordered_map>
#include <memory>

#include <core/runtime/runtime.hpp>

class DeviceListDock : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceListDock(QWidget *parent = nullptr);
    ~DeviceListDock();

    void updateDeviceList(const std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>>& cameras);
    void updateDeviceStatus(const std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>>& cameras);

signals:
    void deviceDoubleClicked(const QString& deviceName);
    void deviceRecordRequested(const QString& deviceName);
    void deviceShowRequested(const QString& deviceName);
    void deviceStopRequested(const QString& deviceName);

private slots:
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onContextMenu(const QPoint& pos);
    void onRecordAction();
    void onShowAction();
    void onStopAction();

private:
    QTreeWidget *treeWidget_;
    QMenu *contextMenu_;
    QAction *recordAction_;
    QAction *showAction_;
    QAction *stopAction_;
    QString contextMenuItemName_;
    void setupUI();
};

#endif // DEVICELISTDOCK_HPP
