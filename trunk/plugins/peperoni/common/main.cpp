#include <QCoreApplication>
#include "unixioenumerator.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);
    UnixIOEnumerator e;
    e.rescan();
    return 0;
}
