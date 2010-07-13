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
#include "kgamerendereditem_p.h"
#include "kgamerenderer.h"
#include "kgamerenderer_p.h"

#include <QtCore/QTimer>

KGameRenderedItemPrivate::KGameRenderedItemPrivate(KGameRenderer* renderer, const QString& spriteKey, KGameRenderedItem* parent)
	: m_parent(parent)
	, m_renderer(renderer)
	, m_spriteKey(spriteKey)
	, m_renderSize(3, 3)
	, m_frame(-1)
	, m_frameCount(-1)
	, m_outdated(false) //must be false or the initial fetchPixmap() in the parent ctor won't work
{
}

KGameRenderedItem::KGameRenderedItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent)
	: QGraphicsPixmapItem(parent)
	, d(new KGameRenderedItemPrivate(renderer, spriteKey, this))
{
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	renderer->d->m_itemInstances << this;
	d->fetchPixmap();
}

KGameRenderedItem::~KGameRenderedItem()
{
	d->m_renderer->d->m_itemInstances.removeAll(this);
	delete d;
}

KGameRenderer* KGameRenderedItem::renderer() const
{
	return d->m_renderer;
}

QString KGameRenderedItem::spriteKey() const
{
	return d->m_spriteKey;
}

const QGraphicsPixmapItem* KGameRenderedItem::pixmapItem() const
{
	return d->m_item;
}

int KGameRenderedItem::frameCount() const
{
	return d->m_frameCount;
}

int KGameRenderedItem::frame() const
{
	return d->m_frame;
}

void KGameRenderedItem::setFrame(int frame)
{
	//check if sprite is animated
	if (d->m_frameCount > 0 && d->m_frame != frame)
	{
		d->m_frame = frame;
		//fetch pixmap without further delay for a smooth animation
		d->m_outdated = true;
		d->fetchPixmapInternal();
	}
}

QSize KGameRenderedItem::renderSize() const
{
	return d->m_renderSize;
}

void KGameRenderedItem::setRenderSize(const QSize& renderSize)
{
	if (d->m_renderSize != renderSize)
	{
		d->m_renderSize = renderSize;
		d->fetchPixmap();
	}
}

void KGameRenderedItemPrivate::fetchPixmap()
{
	if (!m_outdated)
	{
		m_outdated = true;
		QTimer::singleShot(0, this, SLOT(fetchPixmapInternal()));
	}
}

void KGameRenderedItemPrivate::fetchPixmapInternal()
{
	if (m_outdated)
	{
		//update frame count, normalize frame number
		m_frameCount = m_renderer->frameCount(m_spriteKey);
		m_frameBaseIndex = m_renderer->frameBaseIndex();
		if (m_frameCount <= 0)
		{
			m_frame = -1;
		}
		else
		{
			m_frame = (m_frame - m_frameBaseIndex) % m_frameCount + m_frameBaseIndex;
		}
		//actually fetch pixmap
		m_parent->setPixmap(m_renderer->spritePixmap(m_spriteKey, m_renderSize, m_frame));
		m_outdated = false;
	}
}

#include "kgamerendereditem.moc"
#include "kgamerendereditem_p.moc"
