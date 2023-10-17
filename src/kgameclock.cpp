/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
    Portions reused from KGameLCDClock
    SPDX-FileCopyrightText: 2001, 2002, 2003 Nicolas Hadacek <hadacek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgameclock.h"

// Qt
#include <QString>
#include <QTimer>

class KGameClockPrivate
{
public:
    KGameClockPrivate()
        : totalSeconds(0)
    {
    }

    QTimer *timerClock;
    uint totalSeconds;
    KGameClock::ClockType clocktype;
};

namespace
{
QString timeSectionString(uint x)
{
    return QString::number(x).rightJustified(2, QLatin1Char('0'), true);
}

QString timeStringHourMinSec(uint hour, uint min, uint sec)
{
    return timeSectionString(hour) + QLatin1Char(':') + timeSectionString(min) + QLatin1Char(':') + timeSectionString(sec);
}

QString timeStringMinSec(uint min, uint sec)
{
    return timeSectionString(min) + QLatin1Char(':') + timeSectionString(sec);
}

} // namespace

KGameClock::KGameClock(QObject *parent, KGameClock::ClockType clocktype)
    : QObject(parent)
    , d_ptr(new KGameClockPrivate)
{
    Q_D(KGameClock);

    d->clocktype = clocktype;
    d->timerClock = new QTimer(this);
    connect(d->timerClock, &QTimer::timeout, this, &KGameClock::timeoutClock);
}

KGameClock::~KGameClock() = default;

void KGameClock::timeoutClock()
{
    Q_D(KGameClock);

    d->totalSeconds++;
    showTime();
}

QString KGameClock::timeString() const
{
    Q_D(const KGameClock);

    uint sec = d->totalSeconds;
    if (d->clocktype == MinSecOnly) {
        return timeStringMinSec((sec / 60) % 60, sec % 60);
    }
    if (d->clocktype == FlexibleHourMinSec) {
        if (sec < 3600)
            return timeStringMinSec(sec / 60, sec % 60);
        return timeStringHourMinSec(sec / 3600, (sec / 60) % 60, sec % 60);
    }
    if (d->clocktype == LongMinSec) {
        return timeStringMinSec(sec / 60, sec % 60);
    }
    // default is HourMinSec
    return timeStringHourMinSec(sec / 3600, (sec / 60) % 60, sec % 60);
}

void KGameClock::showTime()
{
    Q_EMIT timeChanged(timeString());
}

void KGameClock::restart()
{
    Q_D(KGameClock);

    d->timerClock->stop();
    d->totalSeconds = 0;
    resume();
    showTime();
}

void KGameClock::resume()
{
    Q_D(KGameClock);

    d->timerClock->start(1000); // 1 second
}

void KGameClock::pause()
{
    Q_D(KGameClock);

    d->timerClock->stop();
}

uint KGameClock::seconds() const
{
    Q_D(const KGameClock);

    return d->totalSeconds;
}

void KGameClock::setTime(uint sec)
{
    Q_D(KGameClock);

    d->totalSeconds = sec;
    showTime();
}

void KGameClock::setTime(const QString &s)
{
    const QList<QStringView> sections = QStringView(s).split(QLatin1Char(':'));
    Q_ASSERT(sections.size() <= 3);
    uint sec = 0;
    for (const QStringView &x : sections) {
        sec = sec * 60 + x.toUInt();
    }
    setTime(sec);
}

#include "moc_kgameclock.cpp"
