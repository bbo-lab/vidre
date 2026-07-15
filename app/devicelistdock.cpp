#include "devicelistdock.hpp"
#include <QHeaderView>
#include <QIcon>

DeviceListDock::DeviceListDock(QWidget *parent)
    : QWidget(parent)
    , treeWidget_(new QTreeWidget(this))
    , contextMenu_(new QMenu(this))
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
    
    treeWidget_->setHeaderLabels(QStringList() << "Device" << "Status" << "Vendor");
    treeWidget_->setAlternatingRowColors(true);
    treeWidget_->setRootIsDecorated(false);
    treeWidget_->header()->setStretchLastSection(false);
    treeWidget_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    treeWidget_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    treeWidget_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    treeWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    
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
    connect(treeWidget_, &QWidget::customContextMenuRequested,
            this, &DeviceListDock::onContextMenu);
    
    // Context menu actions
    recordAction_ = new QAction("Record", this);
    showAction_ = new QAction("Show", this);
    stopAction_ = new QAction("Stop", this);
    contextMenu_->addAction(recordAction_);
    contextMenu_->addAction(showAction_);
    contextMenu_->addAction(stopAction_);
    
    connect(recordAction_, &QAction::triggered, this, &DeviceListDock::onRecordAction);
    connect(showAction_, &QAction::triggered, this, &DeviceListDock::onShowAction);
    connect(stopAction_, &QAction::triggered, this, &DeviceListDock::onStopAction);
    
    layout->addWidget(treeWidget_);
    setLayout(layout);
}

QString DeviceListDock::statusToString(vidre::RecordingStatus status)
{
    switch (status) {
        case vidre::RecordingStatus::RecordingFine:           return "Recording";
        case vidre::RecordingStatus::RecordingDroppedFrames:  return "Rec (Dropped)";
        case vidre::RecordingStatus::RecordingError:          return "Rec (Error)";
        case vidre::RecordingStatus::Streaming:               return "Streaming";
        case vidre::RecordingStatus::Stopped:                 return "Stopped";
        default:                                              return "Unknown";
    }
}

QColor DeviceListDock::statusToColor(vidre::RecordingStatus status)
{
    switch (status) {
        case vidre::RecordingStatus::RecordingFine:           return QColor(0, 200, 0);
        case vidre::RecordingStatus::RecordingDroppedFrames:  return QColor(220, 180, 0);
        case vidre::RecordingStatus::RecordingError:          return QColor(220, 40, 40);
        case vidre::RecordingStatus::Streaming:               return QColor(60, 140, 220);
        case vidre::RecordingStatus::Stopped:                 return QColor(200, 100, 0);
        default:                                              return QColor(150, 150, 150);
    }
}

void DeviceListDock::updateDeviceList(const std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>>& cameras)
{
    treeWidget_->clear();
    
    for (const auto& cam : cameras) {
        if (!cam.second || !cam.second->getCamera()) {
            continue;
        }
        
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget_);
        item->setText(0, QString::fromStdString(cam.first));
        
        auto status = cam.second->getRecordingStatus();
        item->setText(1, statusToString(status));
        item->setForeground(1, QBrush(statusToColor(status)));
        
        QString vendor = "Unknown";
        QString camVendorName = QString::fromStdString(cam.second->getCamera()->GetVendorName()).toLower();
        if (camVendorName.contains("pylon") || camVendorName.contains("basler")) {
            vendor = "Pylon";
        } else if (camVendorName.contains("aptina")) {
            vendor = "Aptina";
        } else if (camVendorName.contains("dummy")) {
            vendor = "Dummy";
        }
        item->setText(2, vendor);
        
        treeWidget_->addTopLevelItem(item);
    }
}

void DeviceListDock::updateDeviceStatus(const std::unordered_map<std::string, std::unique_ptr<vidre::RuntimeInstance>>& cameras)
{
    for (int i = 0; i < treeWidget_->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = treeWidget_->topLevelItem(i);
        QString deviceName = item->text(0);
        
        auto it = cameras.find(deviceName.toStdString());
        if (it != cameras.end() && it->second) {
            auto status = it->second->getRecordingStatus();
            item->setText(1, statusToString(status));
            item->setForeground(1, QBrush(statusToColor(status)));
        }
    }
}

void DeviceListDock::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (item) {
        emit deviceDoubleClicked(item->text(0));
    }
}

void DeviceListDock::onContextMenu(const QPoint& pos)
{
    QTreeWidgetItem *item = treeWidget_->itemAt(pos);
    if (!item) {
        return;
    }
    
    contextMenuItemName_ = item->text(0);
    contextMenu_->exec(treeWidget_->viewport()->mapToGlobal(pos));
}

void DeviceListDock::onRecordAction()
{
    emit deviceRecordRequested(contextMenuItemName_);
}

void DeviceListDock::onShowAction()
{
    emit deviceShowRequested(contextMenuItemName_);
}

void DeviceListDock::onStopAction()
{
    emit deviceStopRequested(contextMenuItemName_);
}
