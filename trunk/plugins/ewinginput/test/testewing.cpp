#include "ewing.h"
#include "testewing.h"

EWingStub::EWingStub(QObject* parent, const QHostAddress& host, const QByteArray& ba)
    : EWing(parent, host, ba)
{
    Q_UNUSED(parent);
    Q_UNUSED(host);
    Q_UNUSED(ba);
}

EWingStub::~EWingStub()
{
}

QString EWingStub::name() const
{
    return QString("EWingStub");
}

void EWingStub::parseData(const QByteArray& ba)
{
    Q_UNUSED(ba);
}

void EWing_Test::resolveType()
{
    QByteArray ba;
    EWingStub es(this, QHostAddress(), ba);
}
