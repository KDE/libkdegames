/*
    This file is part of the KDE games library
    Copyright (C) 2007 Mauricio Piacentini (mauricio@tabuleiro.com)
    Portions reused from KGameLCDClock
    Copyright (C) 2001,2002,2003 Nicolas Hadacek (hadacek@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgameclock.h"
#include "kgameclock.moc"

#include <QTimer>

class KGameClockPrivate
{
public:
    KGameClockPrivate()
        : sec(0), min(0), hour(0)
    {
    }

    QTimer *timerClock;
    uint    sec, min, hour;
    KGameClock::ClockType clocktype;
};

KGameClock::KGameClock(QObject *parent, KGameClock::ClockType clocktype)
: QObject(parent) , d(new KGameClockPrivate)
{
    d->clocktype = clocktype;
    d->timerClock = new QTimer(this);
    connect(d->timerClock, SIGNAL(timeout()), SLOT(timeoutClock()));
}

KGameClock::~KGameClock()
{
    delete d;
}

void KGameClock::timeoutClock()
{
    if ( d->hour==23 && d->min==59 && d->sec==59 ) return;
    d->sec++;
    if (d->sec==60) {
        d->min++;
        d->sec = 0;
    }
    if (d->min==60) {
        d->hour++;
        d->min = 0;
    }
    showTime();
}

QString KGameClock::timeString() const
{
    QString sec = QString::number(d->sec).rightJustified(2, '0', true);
    QString min = QString::number(d->min).rightJustified(2, '0', true);
    if (d->clocktype==MinSecOnly) return min + ':' + sec;
    //else return hour as well
    QString hour = QString::number(d->hour).rightJustified(2, '0', true);
    return hour + ':' + min + ':' + sec;
}

void KGameClock::showTime()
{
    emit timeChanged(timeString());
}

void KGameClock::restart()
{
    d->timerClock->stop();
    d->sec = 0;
    d->min = 0;
    d->hour = 0;
    resume();
    showTime();
}

void KGameClock::resume()
{
    d->timerClock->start(1000); // 1 second
}

void KGameClock::pause()
{
    d->timerClock->stop();
}

uint KGameClock::seconds() const
{
    return d->hour*3600 + d->min*60 + d->sec;
}

void KGameClock::setTime(uint sec)
{
    Q_ASSERT( sec<(3600*24) );
    d->sec = sec % 60;
    d->min = (sec / 60) % 60;
    d->hour = sec / 1440 ;
    showTime();
}

void KGameClock::setTime(const QString &s)
{
    Q_ASSERT( s.length()==8 && s[2]==':' && s[5]==':' );
    uint hour = qMin(s.section(':', 0, 0).toUInt(), uint(23));
    uint min = qMin(s.section(':', 1, 1).toUInt(), uint(59));
    uint sec = qMin(s.section(':', 2, 2).toUInt(), uint(59));
    setTime(sec + min*60 + hour*3600);
}
