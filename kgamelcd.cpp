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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgamelcd.h"
#include "kgamelcd.moc"

#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QGridLayout>

#include <kglobal.h>


//-----------------------------------------------------------------------------
KGameLCD::KGameLCD(uint nbDigits, QWidget *parent, const char *name)
    : QLCDNumber(nbDigits, parent, name), _htime(800)
{
    const QPalette &p = palette();
    _fgColor = p.color(QPalette::Active, QColorGroup::Foreground);
    _hlColor = p.color(QPalette::Active, QColorGroup::HighlightedText);

    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), SLOT(timeout()));

    setFrameStyle(Panel | Plain);
	setSegmentStyle(Flat);

    displayInt(0);
}

KGameLCD::~KGameLCD()
{}

void KGameLCD::setDefaultBackgroundColor(const QColor &color)
{
    QPalette p = palette();
    p.setColor(QColorGroup::Background, color);
    setPalette(p);
}

void KGameLCD::setDefaultColor(const QColor &color)
{
    _fgColor = color;
    QPalette p = palette();
    p.setColor(QColorGroup::Foreground, color);
    setPalette(p);
}

void KGameLCD::setHighlightColor(const QColor &color)
{
    _hlColor = color;
}

void KGameLCD::setLeadingString(const QString &s)
{
    _lead = s;
    displayInt(0);
}

void KGameLCD::setHighlightTime(uint time)
{
    _htime = time;
}

void KGameLCD::resetColor()
{
    setColor(QColor());
}

void KGameLCD::setColor(const QColor &color)
{
    const QColor &c = (color.isValid() ? color : _fgColor);
    QPalette p = palette();
    p.setColor(QColorGroup::Foreground, c);
    setPalette(p);
}

void KGameLCD::displayInt(int v)
{
    int n = numDigits() - _lead.length();
    display(_lead + QString::number(v).rightJustified(n));
}

void KGameLCD::highlight()
{
    highlight(true);
    _timer->start(_htime, true);
}

void KGameLCD::highlight(bool light)
{
    if (light) setColor(_hlColor);
    else resetColor();
}

//-----------------------------------------------------------------------------
KGameLCDClock::KGameLCDClock(QWidget *parent, const char *name)
: KGameLCD(5, parent, name)
{
    _timerClock = new QTimer(this);
    connect(_timerClock, SIGNAL(timeout()), SLOT(timeoutClock()));
}

KGameLCDClock::~KGameLCDClock()
{}

void KGameLCDClock::timeoutClock()
{
    // waiting an hour does not restart timer
    if ( _min==59 && _sec==59 ) return;
    _sec++;
    if (_sec==60) {
        _min++;
        _sec = 0;
    }
    showTime();
}

QString KGameLCDClock::pretty() const
{
    QString sec = QString::number(_sec).rightJustified(2, '0', true);
    QString min = QString::number(_min).rightJustified(2, '0', true);
    return min + ':' + sec;
}

void KGameLCDClock::showTime()
{
    display(pretty());
}

void KGameLCDClock::reset()
{
    _timerClock->stop();
	_sec = 0;
    _min = 0;
	showTime();
}

void KGameLCDClock::start()
{
    _timerClock->start(1000); // 1 second
}

void KGameLCDClock::stop()
{
    _timerClock->stop();
}

uint KGameLCDClock::seconds() const
{
    return _min*60 + _sec;
}

void KGameLCDClock::setTime(uint sec)
{
    Q_ASSERT( sec<3600 );
    _sec = sec % 60;
    _min = sec / 60;
    showTime();
}

void KGameLCDClock::setTime(const QString &s)
{
    Q_ASSERT( s.length()==5 && s[2]==':' );
    uint min = kMin(s.section(':', 0, 0).toUInt(), uint(59));
    uint sec = kMin(s.section(':', 1, 1).toUInt(), uint(59));
    setTime(sec + min*60);
}


//-----------------------------------------------------------------------------
class KGameLCDList::KGameLCDListPrivate
{
public:
  Q3ValueVector<QLabel *> _leadings;
};

KGameLCDList::KGameLCDList(const QString &title, QWidget *parent,
                           const char *name)
    : QWidget(parent, name)
{
    init(title);
}

KGameLCDList::KGameLCDList(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    init(QString::null);
}

KGameLCDList::~KGameLCDList()
{
  delete d;
}

void KGameLCDList::init(const QString &title)
{
    d = new KGameLCDListPrivate;

    QGridLayout *top = new QGridLayout(this, 1, 2, 5);
    top->setColStretch(1, 1);

    _title = new QLabel(title, this);
    _title->setAlignment(Qt::AlignCenter);
    top->addMultiCellWidget(_title, 0, 0, 0, 1, Qt::AlignCenter);
}

void KGameLCDList::append(QLCDNumber *lcd)
{
    append(QString::null, lcd);
}

void KGameLCDList::append(const QString &leading, QLCDNumber *lcd)
{
    uint i = size();
    QLabel *label = 0;
    if ( !leading.isEmpty() ) {
      label = new QLabel(leading, this);
      static_cast<QGridLayout *>(layout())->addWidget(label, i+1, 0);
    }
    d->_leadings.push_back(label);
    _lcds.push_back(lcd);
    static_cast<QGridLayout *>(layout())->addWidget(lcd, i+1, 1);
}

void KGameLCDList::clear()
{
    for (int i=0; i<_lcds.size(); i++) {
        delete d->_leadings[i];
        delete _lcds[i];
    }
    d->_leadings.clear();
    _lcds.clear();
}
