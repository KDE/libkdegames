/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "kgamerendereditem.h"
#include "kgamerenderer.h"

#include <QtGui/QGraphicsView>

class KGameRenderedItemPrivate : public QObject, public QGraphicsPixmapItem
{
	public:
		KGameRenderedItemPrivate(KGameRenderedItem* parent);
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
	public:
		KGameRenderedItem* m_parent;
		QGraphicsView* m_primaryView;
};

KGameRenderedItemPrivate::KGameRenderedItemPrivate(KGameRenderedItem* parent)
	: QGraphicsPixmapItem(parent)
	, m_parent(parent)
	, m_primaryView(0)
{
}

KGameRenderedItem::KGameRenderedItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, KGameRendererClient(renderer, spriteKey)
	, d(new KGameRenderedItemPrivate(this))
{
	setPrimaryView(renderer->defaultPrimaryView());
}

KGameRenderedItem::~KGameRenderedItem()
{
	delete d;
}

QGraphicsView* KGameRenderedItem::primaryView() const
{
	return d->m_primaryView;
}

void KGameRenderedItem::setPrimaryView(QGraphicsView* view)
{
	if (d->m_primaryView != view)
	{
		d->m_primaryView = view;
		if (view)
		{
			//use KGameRenderedItemPrivate::paint to determine render size and coordinate system
			d->paint(0, 0, view);
		}
		else
		{
			//reset transform to make coordinate systems of this item and the private item equal
			d->setTransform(QTransform());
		}
	}
}

void KGameRenderedItem::receivePixmap(const QPixmap& pixmap)
{
	d->setPixmap(pixmap);
}

//BEGIN QGraphicsItem reimplementation
//NOTE: The purpose of this reimplementation is to make sure that all interactional events are sent to this item, not to the contained QGraphicsPixmapItem which provides the visual representation (and the metrics calculations).

QRectF KGameRenderedItem::boundingRect() const
{
	if (d->m_primaryView)
	{
		return QRectF(0, 0, 1, 1);
	}
	else
	{
		return d->mapRectToParent(d->boundingRect());
	}
}

bool KGameRenderedItem::contains(const QPointF& point) const
{
	return d->contains(d->mapFromParent(point));
}

bool KGameRenderedItem::isObscuredBy(const QGraphicsItem* item) const
{
	return d->isObscuredBy(item);
}

QPainterPath KGameRenderedItem::opaqueArea() const
{
	return d->mapToParent(d->opaqueArea());
}

void KGameRenderedItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	//the actual painting is done by KGameRenderedItemPrivate::paint (see below)
	Q_UNUSED(painter) Q_UNUSED(option) Q_UNUSED(widget)
}

QPainterPath KGameRenderedItem::shape() const
{
	return d->mapToParent(d->shape());
}

//END QGraphicsItem reimplementation

void KGameRenderedItemPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	//There is no way to get informed when the viewport's coordinate system
	//(relative to this item's coordinate system) has changed, so we're checking
	//the renderSize in each paintEvent coming from the primary view.
	//WARNING: Calls with painter == 0 && option == 0 are used by KGameRenderedItem::setPrimaryView.
	if (m_primaryView)
	{
		if (m_primaryView == widget || m_primaryView->isAncestorOf(widget))
		{
			//determine rectangle which is covered by this item on the view
			const QRectF viewRect = m_primaryView->mapFromScene(m_parent->sceneBoundingRect()).boundingRect();
			//check resulting render size
			const QSize rSize = viewRect.size().toSize();
			if (rSize != m_parent->renderSize())
			{
				m_parent->setRenderSize(rSize);
				//update transform of private item to fit it into (0,0 1x1) in parent coordinates
				QTransform t;
				t.scale(qreal(1.0) / rSize.width(), qreal(1.0) / rSize.height());
				setTransform(t);
				//We're *not* calling the paint() method now. We wait until the pixmap has been delivered.
				return;
			}
		}
	}
	QGraphicsPixmapItem::paint(painter, option, widget);
}

#include "kgamerendereditem.moc"
