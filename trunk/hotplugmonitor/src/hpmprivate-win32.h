#ifndef HPMPRIVATE_WIN32_H
#define HPMPRIVATE_WIN32_H

#include <Windows.h>
#include <QWidget>

class HotPlugMonitor;

/****************************************************************************
 * HPMPrivate declaration
 ****************************************************************************/

class HPMPrivate : public QWidget
{
    Q_OBJECT

public:
    HPMPrivate(HotPlugMonitor* parent = 0);
    ~HPMPrivate();

    void registerNotification();
    void unregisterNotification();

    static bool extractVidPid(const QString& deviceId, uint* vid, uint* pid);

protected:
    bool winEvent(MSG* message, long* result);

private:
    HotPlugMonitor* hpm;
    HDEVNOTIFY hDeviceNotify;
    static const GUID USBClassGUID;
};

#endif