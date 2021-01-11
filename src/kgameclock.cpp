/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
    Portions reused from KGameLCDClock
    SPDX-FileCopyrightText: 2001, 2002, 2003 Nicolas Hadacek <hadacek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgameclock.h"

// Qt
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
    connect(d->timerClock, &QTimer::timeout, this, &KGameClock::timeoutClock);
}

KGameClock::~KGameClock() = default;

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
    QString sec = QString::number(d->sec).rightJustified(2, QLatin1Char( '0' ), true);
    QString min = QString::number(d->min).rightJustified(2, QLatin1Char( '0' ), true);
    if (d->clocktype==MinSecOnly) return min + QLatin1Char( ':' ) + sec;
    //else return hour as well
    QString hour = QString::number(d->hour).rightJustified(2, QLatin1Char( '0' ), true);
    return hour + QLatin1Char( ':' ) + min + QLatin1Char( ':' ) + sec;
}

void KGameClock::showTime()
{
    Q_EMIT timeChanged(timeString());
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
    Q_ASSERT( s.length()==8 && s[2]==QLatin1Char( ':' ) && s[5]==QLatin1Char( ':' ) );
    uint hour = qMin(s.section(QLatin1Char( ':' ), 0, 0).toUInt(), uint(23));
    uint min = qMin(s.section(QLatin1Char( ':' ), 1, 1).toUInt(), uint(59));
    uint sec = qMin(s.section(QLatin1Char( ':' ), 2, 2).toUInt(), uint(59));
    setTime(sec + min*60 + hour*3600);
}
