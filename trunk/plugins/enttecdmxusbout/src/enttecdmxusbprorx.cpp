/*
  Q Light Controller
  enttecdmxusbprorx.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

#include <QDebug>
#include "enttecdmxusbprorx.h"

// FTD2XX will not produce full messages but a continuous flow of bytes,
// so it's best to use the same approach for libftdi-based implementation
// as well. Undef this to use the alternative approach (message at once).
#define RX_BYTE_BY_BYTE

EnttecDMXUSBProRX::EnttecDMXUSBProRX(const QString& serial, const QString& name,
                                     quint32 input, quint32 id)
    : QThread(NULL)
    , EnttecDMXUSBPro(serial, name, id)
    , m_input(input)
    , m_running(false)
    , m_universe(QByteArray(512, char(0)))
{
    qDebug() << Q_FUNC_INFO;
}

EnttecDMXUSBProRX::~EnttecDMXUSBProRX()
{
    qDebug() << Q_FUNC_INFO;
    stop();
}

EnttecDMXUSBWidget::Type EnttecDMXUSBProRX::type() const
{
    return EnttecDMXUSBWidget::ProRX;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBProRX::open()
{
    qDebug() << Q_FUNC_INFO;

    if (EnttecDMXUSBPro::open() == false)
        return close();

    start();
    return true;
}

bool EnttecDMXUSBProRX::close()
{
    qDebug() << Q_FUNC_INFO;
    stop();
    return EnttecDMXUSBWidget::close();
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBProRX::additionalInfo() const
{
    QString info;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2 (%3)").arg(QObject::tr("Protocol"))
                                         .arg("Enttec DMX USB Pro")
                                         .arg(QObject::tr("Input"));
    info += QString("</P>");

    return info;
}

void EnttecDMXUSBProRX::stop()
{
    qDebug() << Q_FUNC_INFO;

    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

#ifdef RX_BYTE_BY_BYTE // BYTE BY BYTE method

void EnttecDMXUSBProRX::run()
{
    qDebug() << Q_FUNC_INFO << "begin";

    uchar byte = 0;
    ushort dataLength = 0;

    m_running = true;
    while (m_running == true) {
        while ( (byte = ftdi()->readByte()) != ENTTEC_PRO_START_OF_MSG) {
            // Fast-forward until we find the start of the next message
        }

        byte = ftdi()->readByte();
        if (byte != ENTTEC_PRO_RECV_DMX_PKT) {
            qWarning() << Q_FUNC_INFO << "Got label:" << (uchar) byte
                       << "but expected:" << (uchar) ENTTEC_PRO_RECV_DMX_PKT;
            continue;
        }

        dataLength = (ushort) ftdi()->readByte() | ((ushort) ftdi()->readByte() << 8);

        byte = ftdi()->readByte();
        if (byte & char(0x01)) {
            qWarning() << Q_FUNC_INFO << "Widget receive queue overflowed";
        } else if (byte & char(0x02)) {
            qWarning() << Q_FUNC_INFO << "Widget receive overrun occurred";
        }

        byte = ftdi()->readByte();
        if (byte != char(0)) {
            qWarning() << Q_FUNC_INFO << "Non-standard DMX startcode received:"
                       << (uchar) byte;
        }

        ushort i = 0;
        for (i = 0; i < dataLength; i++) {
            byte = ftdi()->readByte();
            if (byte == (uchar) ENTTEC_PRO_END_OF_MSG) {
                break;
            } else if (byte != (uchar) m_universe[i]) {
                m_universe[i] = byte;
                emit valueChanged(m_input, i, byte);
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "end";
}

#else // MESSAGE AT ONCE method

void EnttecDMXUSBProRX::run()
{
    qDebug() << Q_FUNC_INFO << "begin";

    QByteArray ba(513, char(0));
    ushort dataLength = 0;

    m_running = true;
    while (m_running == true)
    {
        QByteArray tmp = ftdi()->read(ba.size(), (uchar*) ba.data());
        if (tmp.size() == 0)
            continue;

        if (ba[0] != ENTTEC_PRO_START_OF_MSG) {
            qWarning() << Q_FUNC_INFO << "Got start byte:" << (uchar) ba[0]
                       << "but expected:" << (uchar) ENTTEC_PRO_START_OF_MSG;
            continue;
        }

        if (ba[1] != ENTTEC_PRO_RECV_DMX_PKT) {
            qWarning() << Q_FUNC_INFO << "Got label:" << (uchar) ba[1]
                       << "but expected:" << (uchar) ENTTEC_PRO_RECV_DMX_PKT;
            continue;
        }

        dataLength = (ushort) ba[2] | ((ushort) ba[3] << 8);

        if (ba[4] & char(0x01)) {
            qWarning() << Q_FUNC_INFO << "Widget receive queue overflowed";
        } else if (ba[1] & char(0x02)) {
            qWarning() << Q_FUNC_INFO << "Widget receive overrun occurred";
        }

        if (ba[5] != char(0)) {
            qWarning() << Q_FUNC_INFO << "Non-standard DMX startcode received:"
                       << (uchar) ba[5];
        }

        if (ba[tmp.size() - 1] != ENTTEC_PRO_END_OF_MSG) {
            qWarning() << Q_FUNC_INFO << "Got end byte:" << (uchar) ba[tmp.size() - 1]
                       << "but expected:" << (uchar) ENTTEC_PRO_END_OF_MSG;
        }

        // Skip status & startcode from data, thus -2
        for (int i = 0; i < dataLength - 2; i++) {
            uchar byte = (uchar) ba[6 + i];
            if (byte != m_universe[i]) {
                m_universe[i] = byte;
                emit valueChanged(m_input, i, byte);
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "end";
}
#endif
