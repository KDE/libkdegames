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
/**
 * KGameProgress -- a progress indicator widget for KDE.
 */

#include "kgameprogress.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QRegExp>
#include <QStyle>
#include <QFrame>
#include <QApplication>
#include <QtGui/QAbstractSlider>

#include <kglobalsettings.h>

class KGameProgress::KGameProgressPrivate
{
public:
	KGameProgressPrivate(KGameProgress *qq)
	    : q(qq)
	{
	}

	KGameProgress *q;

	void initialize();
	int recalcValue(int);
	void drawText(QPainter *);
	void adjustStyle();

	QPixmap *bar_pixmap;
	bool use_supplied_bar_color;
	QColor bar_color;
	QColor bar_text_color;
	QColor text_color;
	QRect fr;
	BarStyle bar_style;
	bool text_enabled;
	QString format_;
	QAbstractSlider *slider;
};


KGameProgress::KGameProgress(QWidget *parent)
	: QFrame(parent), d(new KGameProgressPrivate(this))
{
	d->initialize();
	d->slider->setOrientation(Qt::Horizontal);
}

KGameProgress::KGameProgress(Qt::Orientation orientation, QWidget *parent)
	: QFrame(parent), d(new KGameProgressPrivate(this))
{
	d->initialize();
	d->slider->setOrientation(orientation);
}

KGameProgress::~KGameProgress()
{
	delete d->bar_pixmap;
	delete d;
}

void KGameProgress::advance(int offset)
{
	setValue(value() + offset);
}

void KGameProgress::KGameProgressPrivate::initialize()
{
	slider = new QAbstractSlider(q);

	slider->setMinimum(0);
	slider->setMaximum(100);
	slider->setValue(0);

	format_ = QLatin1String( "%p%" );
	use_supplied_bar_color = false;
	bar_pixmap = 0;
	bar_style = Solid;
	text_enabled = true;
	connect(slider, SIGNAL(valueChanged(int)), q, SLOT(valueChange(int)));
	connect(KGlobalSettings::self(), SIGNAL(appearanceChanged()), q, SLOT(paletteChange()));
	q->paletteChange();
}

void KGameProgress::paletteChange()
{
	QPalette p = qApp->palette();
	if (!d->use_supplied_bar_color)
		d->bar_color = p.color( QPalette::Active, QPalette::Highlight );
	d->bar_text_color = p.color( QPalette::Active, QPalette::HighlightedText );
	d->text_color = p.color( QPalette::Active, QPalette::Text );
	setPalette(p);

	d->adjustStyle();
}


void KGameProgress::setBarPixmap(const QPixmap &pixmap)
{
	if (pixmap.isNull())
		return;
	if (d->bar_pixmap)
		delete d->bar_pixmap;

	d->bar_pixmap = new QPixmap(pixmap);
}

void KGameProgress::setBarColor(const QColor &color)
{
	d->bar_color = color;
	d->use_supplied_bar_color = true;
	if (d->bar_pixmap) {
		delete d->bar_pixmap;
		d->bar_pixmap = 0;
	}
}

void KGameProgress::setBarStyle(BarStyle style)
{
	if (d->bar_style != style) {
		d->bar_style = style;
		update();
	}
}

void KGameProgress::setOrientation(Qt::Orientation orientation)
{
	if (this->orientation() != orientation) {
		setOrientation(orientation);
		update();
	}
}

void KGameProgress::setValue(int value)
{
	d->slider->setValue( value );
}

void KGameProgress::setMinimum(int value)
{
	d->slider->setMinimum( value );
}

void KGameProgress::setMaximum(int value)
{
	d->slider->setMaximum( value );
}

void KGameProgress::setTextEnabled(bool enable)
{
	d->text_enabled = enable;
}

QColor KGameProgress::barColor() const
{
	return d->bar_color;
}

const QPixmap * KGameProgress::barPixmap() const
{
	return d->bar_pixmap;
}

int KGameProgress::value() const
{
	return d->slider->value();
}

int KGameProgress::minimum() const
{
	return d->slider->minimum();
}

int KGameProgress::maximum() const
{
	return d->slider->maximum();
}

bool KGameProgress::textEnabled() const
{
	return d->text_enabled;
}

QSize KGameProgress::sizeHint() const
{
	QSize s( size() );

	if(orientation() == Qt::Vertical) {
		s.setWidth(24);
	} else {
		s.setHeight(24);
	}

	return s;
}

QSize KGameProgress::minimumSizeHint() const
{
	return sizeHint();
}

QSizePolicy KGameProgress::sizePolicy() const
{
	if ( orientation()==Qt::Vertical )
		return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
	else
		return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
}

Qt::Orientation KGameProgress::orientation() const
{
	return d->slider->orientation();
}

KGameProgress::BarStyle KGameProgress::barStyle() const
{
	return d->bar_style;
}

int KGameProgress::KGameProgressPrivate::recalcValue(int range)
{
	int abs_value = q->value() - q->minimum();
	int abs_range = q->maximum() - q->minimum();
	return abs_range ? range * abs_value / abs_range : 0;
}

void KGameProgress::valueChange(int newValue)
{
	Q_UNUSED(newValue);

	repaint(contentsRect());
	emit percentageChanged(d->recalcValue(100));
}

void KGameProgress::styleChange(QStyle&)
{
	d->adjustStyle();
}

void KGameProgress::KGameProgressPrivate::adjustStyle()
{
/// @todo Is the code below still necessary in KDE4 ???
/*	switch (style()->styleHint(QStyle::SH_GUIStyle)) {
		case Qt::WindowsStyle:
			setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
			break;
		case Qt::MotifStyle:
		default:
			setFrameStyle(QFrame::Panel | QFrame::Sunken);
			setLineWidth( 2 );
			break;
	}*/
	q->update();
}

void KGameProgress::paletteChange( const QPalette &p )
{
	// This never gets called for global color changes
	// because we call setPalette() ourselves.
	QFrame::paletteChange(p);
}

void KGameProgress::KGameProgressPrivate::drawText(QPainter *p)
{
	QRect r(q->contentsRect());
	//QColor c(bar_color.rgb() ^ backgroundColor().rgb());

	// Rik: Replace the tags '%p', '%v' and '%m' with the current percentage,
	// the current value and the maximum value respectively.
	QString s(format_);

	s.replace(QRegExp( QString::fromLatin1("%p" )), QString::number(recalcValue(100)));
	s.replace(QRegExp( QString::fromLatin1("%v" )), QString::number(q->value()));
	s.replace(QRegExp( QString::fromLatin1("%m" )), QString::number(q->maximum()));

	p->setPen(text_color);
	QFont font = p->font();
	font.setBold(true);
	p->setFont(font);
	//p->setRasterOp(XorROP);
	p->drawText(r, Qt::AlignCenter, s);
	p->setClipRegion( fr );
	p->setPen(bar_text_color);
	p->drawText(r, Qt::AlignCenter, s);
}

void KGameProgress::paintEvent( QPaintEvent *e )
{
	//paint the frame
	QFrame::paintEvent(e);

	//now for the inside of the Widget
	QPainter p(this);

	QRect cr = contentsRect(), er = cr;
	d->fr = cr;
        QBrush fb(d->bar_color), eb(palette().color(backgroundRole()));

	if (d->bar_pixmap)
		fb.setTexture(*d->bar_pixmap);

        QPixmap bkgnd_pix = palette().brush(backgroundRole()).texture();
	if (!bkgnd_pix.isNull())
		eb.setTexture(bkgnd_pix);

	switch (d->bar_style) {
		case Solid:
			if (orientation() == Qt::Horizontal) {
				d->fr.setWidth(d->recalcValue(cr.width()));
				er.setLeft(d->fr.right() + 1);
			} else {
				d->fr.setTop(cr.bottom() - d->recalcValue(cr.height()));
				er.setBottom(d->fr.top() - 1);
			}

			p.setBrushOrigin(cr.topLeft());
			p.fillRect(d->fr, fb);

			p.fillRect(er, eb);

			break;

		case Blocked:
			const int margin = 2;
			int max, num, dx, dy;
			if (orientation() == Qt::Horizontal) {
				d->fr.setHeight(cr.height() - 2 * margin);
				d->fr.setWidth((int)(0.67 * d->fr.height()));
				d->fr.moveTopLeft(QPoint(cr.left() + margin, cr.top() + margin));
				dx = d->fr.width() + margin;
				dy = 0;
				max = (cr.width() - margin) / (d->fr.width() + margin) + 1;
				num = d->recalcValue(max);
			} else {
				d->fr.setWidth(cr.width() - 2 * margin);
				d->fr.setHeight((int)(0.67 * d->fr.width()));
				d->fr.moveBottomLeft(QPoint(cr.left() + margin, cr.bottom() - margin));
				dx = 0;
				dy = - (d->fr.height() + margin);
				max = (cr.height() - margin) / (d->fr.height() + margin) + 1;
				num = d->recalcValue(max);
			}
			p.setClipRect(cr.x() + margin, cr.y() + margin,
			               cr.width() - margin, cr.height() - margin);
			for (int i = 0; i < num; i++) {
				p.setBrushOrigin(d->fr.topLeft());
				p.fillRect(d->fr, fb);
				d->fr.translate(dx, dy);
			}

			if (num != max) {
				if (orientation() == Qt::Horizontal)
					er.setLeft(d->fr.right() + 1);
				else
					er.setBottom(d->fr.bottom() + 1);
				if (!er.isNull()) {
					p.setBrushOrigin(cr.topLeft());
					p.fillRect(er, eb);
				}
			}

			break;
	}

	if (d->text_enabled && d->bar_style != Blocked)
		d->drawText(&p);
}

void KGameProgress::setFormat(const QString & format)
{
	d->format_ = format;
}

QString KGameProgress::format() const
{
	return d->format_;
}

#include "kgameprogress.moc"
