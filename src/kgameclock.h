/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
    Portions reused from KGameLCDClock
    SPDX-FileCopyrightText: 2001, 2002, 2003 Nicolas Hadacek <hadacek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KGAMECLOCK_H
#define __KGAMECLOCK_H

// own
#include <libkdegames_export.h>
// Qt
#include <QObject>

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
    explicit KGameClock(QObject *parent = nullptr, ClockType clocktype = HourMinSec);

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
