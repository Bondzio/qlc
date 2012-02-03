#include <QApplication>
#include <QDebug>
#include <QTimer>

#include "hotplugmonitor.h"
#include "hpmtest.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    HotPlugMonitor* mon = new HotPlugMonitor;
    HPMTest* test = new HPMTest(mon);
    test->show();

    mon->start();

    int r = app.exec();

    delete test;
    delete mon;

    return r;
}
