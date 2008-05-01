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

#ifndef __KGAMECLOCK_H
#define __KGAMECLOCK_H

#include <QtCore/QObject>
#include <libkdegames_export.h>

class KGameClockPrivate;

/**
 * \class KGameClock kgameclock.h <KGameClock>
 * 
 * Class representing a game clock, wraps after 24 hours
 */
class KDEGAMES_EXPORT KGameClock : public QObject
{
    Q_OBJECT
public:
    enum ClockType { HourMinSec = 0, MinSecOnly };

    /**
     * @return Constructor
     */
    explicit KGameClock(QObject *parent = 0, ClockType clocktype = HourMinSec);

    virtual ~KGameClock();

    /**
     * @return the total number of seconds elapsed.
     */
    uint seconds() const;

    /**
     * @return the time as a string to be displayed: "mm:ss" or "hh:mm:ss" depending on clock type.
     */
    QString timeString() const;

    /**
     * Set the time.
     */
    void setTime(uint seconds);

    /**
     * Set the time (format should be "hh:mm:ss").
     */
    void setTime(const QString &s);
    
    /**
     * Refresh
     */
    void showTime();

Q_SIGNALS:
    void timeChanged(const QString &);

public Q_SLOTS:
    /**
     * Reset the clock and start again from zero
     */
    virtual void restart();

    /**
     * Pause the clock
     */
    virtual void pause();

    /**
     * Resume counting time from the current position
     */
    virtual void resume();

protected Q_SLOTS:
    virtual void timeoutClock();

private:
    friend class KGameClockPrivate;
    KGameClockPrivate *const d;

    Q_DISABLE_COPY(KGameClock)
};

#endif
