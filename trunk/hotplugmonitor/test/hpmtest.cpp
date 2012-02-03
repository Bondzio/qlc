#include <QListWidget>
#include <QLayout>

#include "hotplugmonitor.h"
#include "hpmtest.h"

HPMTest::HPMTest(HotPlugMonitor* mon)
    : QWidget(0)
{
    new QHBoxLayout(this);
    m_list = new QListWidget(this);
    layout()->addWidget(m_list);

    connect(mon, SIGNAL(deviceAdded(uint,uint)), this, SLOT(slotDeviceAdded(uint,uint)));
    connect(mon, SIGNAL(deviceRemoved(uint,uint)), this, SLOT(slotDeviceRemoved(uint,uint)));
}

HPMTest::~HPMTest()
{
}

void HPMTest::slotDeviceAdded(uint vid, uint pid)
{
    m_list->addItem(QString("%1: VID %2, PID %3").arg("Added")
                                                 .arg(QString::number(vid, 16))
                                                 .arg(QString::number(pid, 16)));
}

void HPMTest::slotDeviceRemoved(uint vid, uint pid)
{
    m_list->addItem(QString("%1: VID %2, PID %3").arg("Removed")
                                                 .arg(QString::number(vid, 16))
                                                 .arg(QString::number(pid, 16)));
}