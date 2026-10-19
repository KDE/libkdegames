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
#include "kdegames_export.h"
// Qt
#include <QObject>
// Std
#include <memory>

class KGameClockPrivate;

/*!
 * \class KGameClock
 * \inmodule LibKDEGames
 * \brief Class representing a game clock.
 *
 * This class represents a game clock.
 */
class KDEGAMES_EXPORT KGameClock : public QObject
{
    Q_OBJECT

public:
    /*!
     * \enum KGameClock::ClockType
     * Controls the format of the return value of \l timeString().
     * \table
     * \header
     *     \li Format
     *     \li Example1
     *     \li Example2
     * \row
     *     \li HourMinSec
     *     \li 00:04:05
     *     \li 01:02:03
     * \row
     *     \li MinSecOnly
     *     \li    04:05
     *     \li    02:03
     * \row
     *     \li FlexibleHourMinSec
     *     \li    04:05
     *     \li 01:02:03
     * \row
     *     \li LongMinSec
     *     \li    04:05
     *     \li    62:03
     * \endtable
     * \value HourMinSec
     *        Time in hours, minutes, and seconds.
     * \value MinSecOnly
     *        Time in minutes and seconds, minus any hours.
     * \value [since 22.04] FlexibleHourMinSec
     *        Time in minutes and seconds, with hours if necessary.
     * \value [since 22.04] LongMinSec
     *        Time in total minutes and seconds.
     */
    enum ClockType {
        HourMinSec = 0,
        MinSecOnly = 1,
        FlexibleHourMinSec = 2,
        LongMinSec = 3,
    };

    /*!
     * Constructs a KGameClock with \a parent, \a clocktype.
     */
    explicit KGameClock(QObject *parent = nullptr, ClockType clocktype = HourMinSec);

    ~KGameClock() override;

    /*!
     * Returns the total number of seconds elapsed.
     */
    uint seconds() const;

    /*!
     * Returns the time as a string to be displayed: "mm:ss" or "hh:mm:ss" depending on clock type.
     */
    QString timeString() const;

    /*!
     * Sets the time to \a seconds.
     */
    void setTime(uint seconds);

    /*!
     * Sets the time to \a s, format should be "hh:mm:ss" (or "mm:ss" since 22.04).
     */
    void setTime(const QString &s);

    /*!
     * Refreshes the time by emitting timeChanged.
     */
    void showTime();

Q_SIGNALS:
    /*!
     * When showTime() is called, emits the value of timeString().
     */
    void timeChanged(const QString &);

public Q_SLOTS:
    /*!
     * Resets the clock and starts again from zero.
     */
    virtual void restart();

    /*!
     * Pauses the clock.
     */
    virtual void pause();

    /*!
     * Resumes counting time from the current position.
     */
    virtual void resume();

protected Q_SLOTS:
    virtual void timeoutClock();

private:
    friend class KGameClockPrivate;
    std::unique_ptr<KGameClockPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KGameClock)

    Q_DISABLE_COPY(KGameClock)
};

#endif
