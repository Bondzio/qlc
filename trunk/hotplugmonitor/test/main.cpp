#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include "hotplugmonitor.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    HotPlugMonitor mon(NULL);
    mon.start();

    //QTimer::singleShot(1000, &mon, SLOT(stop()));

    return app.exec();
}
