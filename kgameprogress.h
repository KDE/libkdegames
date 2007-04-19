/* This file is part of the KDE libraries
   Copyright (C) 1996 Martynas Kunigelis martynas.kunigelis@vm.ktu.lt

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
/*****************************************************************************
*                                                                            *
*  KGameProgress -- progress indicator widget for KDE by Martynas Kunigelis  *
*                                                                            *
*****************************************************************************/

#ifndef _KPROGRES_H
#define _KPROGRES_H "$Id$"

#include <QtGui/QFrame>
#include <QtGui/QAbstractSlider>
#include <libkdegames_export.h>
/**
 * @short A progress indicator widget.
 *
 * KGameProgress is derived from QFrame and QRangeControl, so
 * you can use all the methods from those classes. The only difference
 * is that setValue() is now made a slot, so you can connect
 * stuff to it.
 *
 * None of the constructors take line step and page step as arguments,
 * so by default they're set to 1 and 10 respectively.
 *
 * The Blocked style ignores the textEnabled() setting and displays
 * no text, since it looks truly ugly (and for other reasons). Signal
 * percentageChanged() is emitted whenever the value changes so you
 * can set up a different widget to display the current percentage complete
 * and connect the signal to it.
 *
 * @author Martynas Kunigelis
 * @version $Id$
 */
class KDEGAMES_EXPORT KGameProgress : public QFrame
{
  Q_OBJECT
  Q_ENUMS( BarStyle )
  Q_PROPERTY( int value READ value WRITE setValue)
  Q_PROPERTY( int mininum READ minimum WRITE setMinimum)
  Q_PROPERTY( int maximum READ maximum WRITE setMaximum)
  Q_PROPERTY( BarStyle barStyle READ barStyle WRITE setBarStyle )
  Q_PROPERTY( QColor barColor READ barColor WRITE setBarColor )
  Q_PROPERTY( QPixmap barPixmap READ barPixmap WRITE setBarPixmap )
  Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )
  Q_PROPERTY( bool textEnabled READ textEnabled WRITE setTextEnabled )

public:
  /**
   * Possible values for bar style.
   *
   * @p Solid means one continuous progress bar, @p Blocked means a
   * progress bar made up of several blocks.
   */
  enum BarStyle { Solid, Blocked };

  /**
   * Construct a horizontal progress bar.
   */
  explicit KGameProgress(QWidget *parent=0);

  /**
   * Construct a progress bar with orientation @p orient.
   */
  explicit KGameProgress(Qt::Orientation orient, QWidget *parent=0);

  /**
   * Destruct the progress bar.
   */
  ~KGameProgress();

  /**
   * Set the progress bar style.
   *
   * Allowed values are @p Solid and @p Blocked.
   */
  void setBarStyle(BarStyle style);

  /**
   * Set the color of the progress bar.
   */
  void setBarColor(const QColor &);

  /**
   * Set a pixmap to be shown in the progress bar.
   */
  void setBarPixmap(const QPixmap &);

  /**
   * Set the orientation of the progress bar.
   *
   * Allowed values are @p Horizontal and @p Vertical.
   */
  void setOrientation(Qt::Orientation);

  /**
   * If this is set to @p true, the progress text will be displayed.
   *
   */
  void setTextEnabled(bool);

  /**
   * Retrieve the bar style.
   *
   * @see setBarStyle()
   */
  BarStyle barStyle() const;

  /**
   * Retrieve the bar color.
   * @see setBarColor()
   */
  QColor barColor() const;

  /**
   * Retrieve the bar pixmap.
   *
   * @see setBarPixmap()
   */
  const QPixmap *barPixmap() const;

  /**
   * Retrive the current status
   *
   * @see setValue()
   */
  int value() const { return slider->value(); }

  /**
   * Retrieve the minimum value
   *
   * @see setMinimum()
   */
  int minimum() const { return slider->minimum(); }

  /**
   * Retrieve the maximum value
   *
   * @see setMaximum()
   */
  int maximum() const { return slider->maximum(); }

  /**
   * Retrive the orientation of the progress bar.
   *
   * @see setOrientation()
   */
  Qt::Orientation orientation() const;

  /**
   * Returns @p true if progress text will be displayed,
   * @p false otherwise.
   *
   * @see setFormat()
   */
  bool textEnabled() const;

  /**
   */
  virtual QSize sizeHint() const;

  /**
   */
  virtual QSize minimumSizeHint() const;

  /**
   */
  virtual QSizePolicy sizePolicy() const;

  /**
   * Retrieve the current format for printing status text.
   * @see setFormat()
   */
  QString format() const;

public Q_SLOTS:

  /**
   * Set the format of the text to use to display status.
   *
   * The default format is "%p%" (which looks like "42%".)
   *
   * @param format %p is replaced by percentage done, %v is replaced by actual
   * value, %m is replaced by the maximum value.
   */
  void setFormat(const QString & format);

  /**
   * Set the current value of the progress bar to @p value.
   *
   * This must be a number in the range 0..100.
   */
  void setValue(int value);

  /**
   * Set the minimum value of the progress bar to @p value.
   *
   * The maximum value may be changed to keep the range valid.
   */
  void setMinimum(int value);

  /**
   * Set the maximum value of the progress bar to @p value.
   *
   * The minimum value may be changed to keep the range valid.
   */
  void setMaximum(int value);

  /**
   * Advance the progress bar by @p prog.
   *
   * This method is
   * provided for convenience and is equivalent with
   * setValue(value()+prog).
   */
  void advance(int prog);

  /**
   */
  void valueChange(int newValue);

Q_SIGNALS:
  /**
   * Emitted when the state of the progress bar changes.
   */
  void percentageChanged(int);

protected:
  /**
   */
  void styleChange( QStyle& );
  /**
   */
  void paletteChange( const QPalette & );
  /**
   */
  void paintEvent( QPaintEvent *e );

private Q_SLOTS:
  void paletteChange();

private:
  QPixmap  *bar_pixmap;
  bool      use_supplied_bar_color;
  QColor    bar_color;
  QColor    bar_text_color;
  QColor    text_color;
  QRect     fr;
  BarStyle  bar_style;
  bool      text_enabled;
  QString   format_;
  QAbstractSlider *slider;
  void      initialize();
  int       recalcValue(int);
  void      drawText(QPainter *);
  void      adjustStyle();

  class KGameProgressPrivate;
  KGameProgressPrivate *d;
};


#endif
