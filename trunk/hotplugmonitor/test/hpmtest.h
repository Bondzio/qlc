#ifndef HPMTEST_H
#define HPMTEST_H

#include <QWidget>

class HotPlugMonitor;
class QListWidget;

class HPMTest : public QWidget
{
    Q_OBJECT

public:
    HPMTest(HotPlugMonitor* mon);
    ~HPMTest();

private slots:
    void slotDeviceAdded(uint vid, uint pid);
    void slotDeviceRemoved(uint vid, uint pid);

private:
    QListWidget* m_list;
};

#endif