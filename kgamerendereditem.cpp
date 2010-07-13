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

class KGameRenderedItemPrivate
{
	public:
		KGameRenderedItemPrivate(KGameRenderedItem* parent);
		QGraphicsPixmapItem* m_pixmapItem;
};

KGameRenderedItemPrivate::KGameRenderedItemPrivate(KGameRenderedItem* parent)
	: m_pixmapItem(new QGraphicsPixmapItem(parent))
{
}

KGameRenderedItem::KGameRenderedItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, KGameRendererClient(renderer, spriteKey)
	, d(new KGameRenderedItemPrivate(this))
{
	d->m_pixmapItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

KGameRenderedItem::~KGameRenderedItem()
{
	delete d;
}

void KGameRenderedItem::receivePixmap(const QPixmap& pixmap)
{
	d->m_pixmapItem->setPixmap(pixmap);
}

//BEGIN QGraphicsItem reimplementation
//NOTE: The purpose of this reimplementation is to make sure that all interactional events are sent to this item, not to the contained QGraphicsPixmapItem which provides the visual representation (and the metrics calculations).

QRectF KGameRenderedItem::boundingRect() const
{
	return d->m_pixmapItem->mapRectToParent(d->m_pixmapItem->boundingRect());
}

bool KGameRenderedItem::contains(const QPointF& point) const
{
	return d->m_pixmapItem->contains(d->m_pixmapItem->mapFromParent(point));
}

bool KGameRenderedItem::isObscuredBy(const QGraphicsItem* item) const
{
	return d->m_pixmapItem->isObscuredBy(item);
}

QPainterPath KGameRenderedItem::opaqueArea() const
{
	return d->m_pixmapItem->mapToParent(d->m_pixmapItem->opaqueArea());
}

void KGameRenderedItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(painter) Q_UNUSED(option) Q_UNUSED(widget)
	//actual painting is done by d->m_pixmapItem
}

QPainterPath KGameRenderedItem::shape() const
{
	return d->m_pixmapItem->mapToParent(d->m_pixmapItem->shape());
}

//END QGraphicsItem reimplementation

#include "kgamerendereditem.moc"
