/*
    This file is part of the KDE games library
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

#ifndef __KGAMELCD_H
#define __KGAMELCD_H

#include <qlcdnumber.h>
#include <QVector>
//Added by qt3to4:
#include <QLabel>
#include <kdemacros.h>

class QLabel;
class QTimer;

//-----------------------------------------------------------------------------
/**
 * This class is a visually enhanced @ref QLCDNumber:
 * <ul>
 * <li> It can show an additional string before the integer being
 * displayed.</li>
 * <li> Its foreground and background colors can easily be modified. </li>
 * <li> It can be highlighted for a short time. </li>
 * </ul>
 *
 * @since 3.2
 */
class KDE_EXPORT KGameLCD : public QLCDNumber
{
    Q_OBJECT
public:
    KGameLCD(uint nbDigits, QWidget *parent = 0, const char *name = 0);

    ~KGameLCD();

    /**
     * Set the default background color.
     */
    void setDefaultBackgroundColor(const QColor &color);

    /**
     * Set the default foreground color.
     */
    void setDefaultColor(const QColor &color);

    /**
     * Set highlight color.
     */
    void setHighlightColor(const QColor &color);

    /**
     * Set the string that will be displayed before the integer number to be
     * displayed. By default this string is null.
     */
    void setLeadingString(const QString &s);

    /**
     * Set the highlight duration in milliseconds. The default value is
     * 800 milliseconds.
     */
    void setHighlightTime(uint time);

    /**
     * Reset the foreground color to the default one.
     */
    void resetColor();

    /**
     * Set the foreground color.
     */
    void setColor(const QColor &color);

public slots:
    /**
     * Highlight the LCD with the QColorGourp::HighlightedText color
     * for a small time (setHighlightTime).
     */
    void highlight();

    /**
     * Display the given integer with the (optionnal) leading string.
     *
     * Note: we cannot use display(int) since QLCDNumber::display is
     * not virtual... And you cannot use QLCDNumber::intValue() to retrieve
     * the given value.
     */
    void displayInt(int value);

private slots:
    void timeout() { highlight(false); }

private:
    QColor   _fgColor, _hlColor;
    QString  _lead;
    uint     _htime;
    QTimer  *_timer;

    class KGameLCDPrivate;
    KGameLCDPrivate *d;

    void highlight(bool light);

};

//-----------------------------------------------------------------------------
/**
 * This class is a digital clock widget. It has a maximum duration of
 * 3599 seconds (one hour) and it gets updated every second.
 *
 * @since 3.2
 */
class KDE_EXPORT KGameLCDClock : public KGameLCD
{
    Q_OBJECT
public:
    KGameLCDClock(QWidget *parent = 0, const char *name = 0);

    ~KGameLCDClock();

    /**
     * @return the total number of seconds elapsed.
     */
    uint seconds() const;

    /**
     * @return the time as a string to be displayed: "mm:ss".
     */
    QString pretty() const;

    /**
     * Set the time.
     */
    void setTime(uint seconds);

    /**
     * Set the time (format should be "mm:ss").
     */
    void setTime(const QString &s);

public slots:
    /**
     * Stop the clock and reset it to zero.
     */
    virtual void reset();

    /**
     * Stop the clock but do not reset it to zero.
     */
	virtual void stop();

    /**
     * Start the clock from the current time.
     */
	virtual void start();

protected slots:
    virtual void timeoutClock();

private:
    QTimer *_timerClock;
	uint    _sec, _min;

    class KGameLCDClockPrivate;
    KGameLCDClockPrivate *d;

	void showTime();
};

//-----------------------------------------------------------------------------
/**
 * This widget holds a list of @ref QLCDNumber arranged in a vertical layout.
 * It also shows a label at the top of the list.
 *
 * @since 3.2
 */
class KDE_EXPORT KGameLCDList : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * @param title is the content of the top label.
     * @param parent passed to the QWidget constructor
     * @param name passed to the QWidget constructor
     */
    KGameLCDList(const QString &title,
                 QWidget *parent = 0, const char *name = 0);
    KGameLCDList(QWidget *parent = 0, const char *name = 0);

    ~KGameLCDList();

    /**
     * Append a QLCDNumber at the bottom of the list.
     * The QLCDNumber should have the KGameLCDList as parent.
     */
    void append(QLCDNumber *lcd);
    
    /**
     * Append a QLCDNumber at the bottom of the list.
     * The QLCDNumber should have the KGameLCDList as parent.
     */
    void append(const QString &leading, QLCDNumber *lcd);

    /**
     * Delete all @ref QLCDNumber and clear the list.
     */
    void clear();

    /**
     * @return the title label.
     */
    QLabel *title() const { return _title; }

    /**
     * @return the QLCDNumber at index @param i
     */
    QLCDNumber *lcd(uint i) const { return _lcds[i]; }
    
    /**
     * @return the number of QLCDNumber in the list.
     */
    uint size() const { return _lcds.size(); }

private:
    QLabel *_title;
    QVector<QLCDNumber *> _lcds;

    class KGameLCDListPrivate;
    KGameLCDListPrivate *d;

    void init(const QString &title);
};

#endif
