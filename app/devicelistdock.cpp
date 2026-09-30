#include "devicelistdock.hpp"
#include <QHeaderView>
#include <QIcon>

DeviceListDock::DeviceListDock(QWidget *parent)
    : QWidget(parent)
    , treeWidget_(new QTreeWidget(this))
{
    setupUI();
}

DeviceListDock::~DeviceListDock()
{
}

void DeviceListDock::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    treeWidget_->setHeaderLabels(QStringList() << "Device" << "Status" << "Vendor" << "Trigger");
    treeWidget_->setAlternatingRowColors(true);
    treeWidget_->setRootIsDecorated(false);
    treeWidget_->header()->setStretchLastSection(false);
    treeWidget_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    treeWidget_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    treeWidget_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    treeWidget_->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    
    // Dark theme styling
    treeWidget_->setStyleSheet(
        "QTreeWidget {"
        "    background-color: #2b2b2b;"
        "    color: #bbbbbb;"
        "    border: 1px solid #555555;"
        "    alternate-background-color: #323232;"
        "}"
        "QTreeWidget::item:selected {"
        "    background-color: #0d5d9f;"
        "}"
        "QTreeWidget::item:hover {"
        "    background-color: #3c3f41;"
        "}"
        "QHeaderView::section {"
        "    background-color: #3c3f41;"
        "    color: #bbbbbb;"
        "    padding: 4px;"
        "    border: 1px solid #555555;"
        "    font-weight: bold;"
        "}"
    );
    
    connect(treeWidget_, &QTreeWidget::itemDoubleClicked, 
            this, &DeviceListDock::onItemDoubleClicked);
    
    layout->addWidget(treeWidget_);
    setLayout(layout);
}

void DeviceListDock::updateDeviceList(const std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>>& cameras)
{
    treeWidget_->clear();
    
    for (const auto& cam : cameras) {
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget_);
        item->setText(0, QString::fromStdString(cam.first));
        item->setText(1, cam.second->isRunning() ? "Running" : "Stopped");
        
        QString vendor = "Unknown"; //
        QString camVendorName = QString::fromStdString(cam.second->getCamera()->GetVendorName()).toLower();
        if (camVendorName.contains("pylon") || camVendorName.contains("basler")) {
            vendor = "Pylon";
        } else if (camVendorName.contains("aptina")) {
            vendor = "Aptina";
        }
        item->setText(2, vendor);
        
        vidre::ICamera *camera = cam.second->getCamera();
        if (camera && camera->triggerOutSet) {
            item->setText(3, camera->triggerInSet ? "IN + OUT" : "OUT");
            item->setForeground(3, QBrush(QColor(204, 102, 255)));
        } else if (camera && camera->triggerInSet) {
            item->setText(3, "IN");
            item->setForeground(3, QBrush(QColor(51, 170, 255)));
        }
        
        // Color code status
        if (cam.second->isRunning()) {
            item->setForeground(1, QBrush(QColor(0, 200, 0)));
        } else {
            item->setForeground(1, QBrush(QColor(200, 100, 0)));
        }
        
        treeWidget_->addTopLevelItem(item);
    }
}

void DeviceListDock::updateDeviceStatus(const std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>>& cameras)
{
    for (int i = 0; i < treeWidget_->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = treeWidget_->topLevelItem(i);
        QString deviceName = item->text(0);
        
        auto it = cameras.find(deviceName.toStdString());
        if (it != cameras.end()) {
            item->setText(1, it->second->isRunning() ? "Running" : "Stopped");
            
            if (it->second->isRunning()) {
                item->setForeground(1, QBrush(QColor(0, 200, 0)));
            } else {
                item->setForeground(1, QBrush(QColor(200, 100, 0)));
            }
        }
    }
}

void DeviceListDock::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (item) {
        emit deviceDoubleClicked(item->text(0));
    }
}
