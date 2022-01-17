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
// Std
#include <memory>

class KGameClockPrivate;

/**
 * \class KGameClock kgameclock.h <KGameClock>
 *
 * Class representing a game clock
 */
class KDEGAMES_EXPORT KGameClock : public QObject
{
    Q_OBJECT
public:
    /**
     * Controls the format of return value of @ref timeString()
     * Format              Example1   Example2
     * ---------------------------------------
     * HourMinSec          00:04:05   01:02:03
     * MinSecOnly             04:05      02:03
     * FlexibleHourMinSec     04:05   01:02:03
     * LongMinSec             04:05      62:03
     */
    enum ClockType
    {
        HourMinSec = 0,
        MinSecOnly = 1,
        FlexibleHourMinSec = 2, ///< @since 22.04
        LongMinSec = 3, ///< @since 22.04
    };

    /**
     * @return Constructor
     */
    explicit KGameClock(QObject *parent = nullptr, ClockType clocktype = HourMinSec);

    ~KGameClock() override;

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
     * Set the time. Format should be "hh:mm:ss" (or "mm:ss" @since 22.04).
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
    std::unique_ptr<KGameClockPrivate> const d;

    Q_DISABLE_COPY(KGameClock)
};

#endif
