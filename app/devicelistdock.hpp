#ifndef DEVICELISTDOCK_HPP
#define DEVICELISTDOCK_HPP

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
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

private slots:
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    QTreeWidget *treeWidget_;
    void setupUI();
};

#endif // DEVICELISTDOCK_HPP
