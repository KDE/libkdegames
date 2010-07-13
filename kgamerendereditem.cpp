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

class KGameRenderedItemPrivate : public QGraphicsPixmapItem
{
	public:
		KGameRenderedItemPrivate(KGameRenderedItem* parent);
		bool adjustRenderSize(); //returns whether an adjustment was made; WARNING: only call when m_primaryView != 0

		//QGraphicsItem reimplementations (see comment below for why we need all of this)
		virtual bool contains(const QPointF& point) const;
		virtual bool isObscuredBy(const QGraphicsItem* item) const;
		virtual QPainterPath opaqueArea() const;
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
		virtual QPainterPath shape() const;
	public:
		KGameRenderedItem* m_parent;
		QGraphicsView* m_primaryView;
		QSize m_correctRenderSize;
};

KGameRenderedItemPrivate::KGameRenderedItemPrivate(KGameRenderedItem* parent)
	: QGraphicsPixmapItem(parent)
	, m_parent(parent)
	, m_primaryView(0)
{
}

bool KGameRenderedItemPrivate::adjustRenderSize()
{
	Q_ASSERT(m_primaryView);
	//determine rectangle which is covered by this item on the view
	const QRectF viewRect = m_primaryView->mapFromScene(m_parent->sceneBoundingRect()).boundingRect();
	//check resulting render size
	m_correctRenderSize = viewRect.size().toSize();
	const QSize diff = m_parent->renderSize() - m_correctRenderSize;
	//ignore fluctuations in the render size which result from rounding errors
	if (qAbs(diff.width()) <= 1 && qAbs(diff.height()) <= 1)
	{
		return false;
	}
	m_parent->setRenderSize(m_correctRenderSize);
	//calculate new transform for this item
	QTransform t;
	t.scale(qreal(1.0) / m_correctRenderSize.width(), qreal(1.0) / m_correctRenderSize.height());
	//render item
	m_parent->prepareGeometryChange();
	setTransform(t);
	m_parent->update();
	return true;
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

QPointF KGameRenderedItem::offset() const
{
	return d->pos();
}

void KGameRenderedItem::setOffset(const QPointF& offset)
{
	if (d->pos() != offset)
	{
		prepareGeometryChange();
		d->setPos(offset);
		update();
	}
}

void KGameRenderedItem::setOffset(qreal x, qreal y)
{
	setOffset(QPointF(x, y));
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
			//determine render size and adjust coordinate system
			d->m_correctRenderSize = QSize(-10, -10); //force adjustment to be made
			d->adjustRenderSize();
		}
		else
		{
			//reset transform to make coordinate systems of this item and the private item equal
			prepareGeometryChange();
			d->setTransform(QTransform());
			update();
		}
	}
}

void KGameRenderedItem::receivePixmap(const QPixmap& pixmap)
{
	prepareGeometryChange();
	d->setPixmap(pixmap);
	update();
}

//We want to make sure that all interactional events are sent ot this item, and
//not to the contained QGraphicsPixmapItem which provides the visual
//representation (and the metrics calculations).
//At the same time, we do not want the contained QGraphicsPixmapItem to slow
//down operations like QGraphicsScene::collidingItems().
//So the strategy is to use the QGraphicsPixmapItem implementation from
//KGameRenderedItemPrivate for KGameRenderedItem.
//Then the relevant methods in KGameRenderedItemPrivate are reimplemented empty
//to effectively clear the item and hide it from any collision detection. This
//strategy allows us to use the nifty QGraphicsPixmapItem logic without exposing
//a QGraphicsPixmapItem subclass (which would conflict with QGraphicsObject).

//BEGIN QGraphicsItem reimplementation of KGameRenderedItem

QRectF KGameRenderedItem::boundingRect() const
{
	return d->mapRectToParent(d->QGraphicsPixmapItem::boundingRect());
}

bool KGameRenderedItem::contains(const QPointF& point) const
{
	return d->QGraphicsPixmapItem::contains(d->mapFromParent(point));
}

bool KGameRenderedItem::isObscuredBy(const QGraphicsItem* item) const
{
	return d->QGraphicsPixmapItem::isObscuredBy(item);
}

QPainterPath KGameRenderedItem::opaqueArea() const
{
	return d->mapToParent(d->QGraphicsPixmapItem::opaqueArea());
}

void KGameRenderedItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(painter) Q_UNUSED(option) Q_UNUSED(widget)
}

QPainterPath KGameRenderedItem::shape() const
{
	return d->mapToParent(d->QGraphicsPixmapItem::shape());
}

//END QGraphicsItem reimplementation of KGameRenderedItem
//BEGIN QGraphicsItem reimplementation of KGameRenderedItemPrivate

bool KGameRenderedItemPrivate::contains(const QPointF& point) const
{
	Q_UNUSED(point)
	return false;
}

bool KGameRenderedItemPrivate::isObscuredBy(const QGraphicsItem* item) const
{
	Q_UNUSED(item)
	return false;
}

QPainterPath KGameRenderedItemPrivate::opaqueArea() const
{
	return QPainterPath();
}

void KGameRenderedItemPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	//Trivial stuff up to now. The fun stuff starts here. ;-)
	//There is no way to get informed when the viewport's coordinate system
	//(relative to this item's coordinate system) has changed, so we're checking
	//the renderSize in each paintEvent coming from the primary view.
	if (m_primaryView)
	{
		if (m_primaryView == widget || m_primaryView->isAncestorOf(widget))
		{
			if (adjustRenderSize())
			{
				//An adjustment was made, so not paint now, but wait for the next painting.
				return;
			}
		}
		//draw pixmap directly in physical coordinates
		const QPoint basePos = painter->transform().map(QPointF()).toPoint();
		painter->save();
		painter->setTransform(QTransform());
		painter->drawPixmap(basePos, pixmap());
		painter->restore();
	}
	else
	{
		QGraphicsPixmapItem::paint(painter, option, widget);
		return;
	}
}

QPainterPath KGameRenderedItemPrivate::shape() const
{
	return QPainterPath();
}

//END QGraphicsItem reimplementation of KGameRenderedItemPrivate

#include "kgamerendereditem.moc"
