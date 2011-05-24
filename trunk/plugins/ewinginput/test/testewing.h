#ifndef TESTEWING_H
#define TESTEWING_H

#include <QObject>
#include "ewing.h"

class EWingStub : public EWing
{
    Q_OBJECT
public:
    EWingStub(QObject* parent, const QHostAddress& host, const QByteArray& ba);
    ~EWingStub();

    QString name() const;
    void parseData(const QByteArray& ba);
};

class EWing_Test : public QObject
{
    Q_OBJECT

private slots:
    void resolveType();
    void resolveFirmware();
    void isOutputData();
    void initial();
    void page();
    void bcd();
    void cache();
};

#endif
