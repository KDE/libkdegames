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

#include "kgamerendererclient.h"
#include "kgamerenderer.h"
#include "kgamerenderer_p.h"

#include <QtCore/QTimer>

KGameRendererClientPrivate::KGameRendererClientPrivate(KGameRenderer* renderer, const QString& spriteKey, KGameRendererClient* parent)
	: m_parent(parent)
	, m_renderer(renderer)
	, m_spriteKey(spriteKey)
	, m_renderSize(3, 3)
	, m_frame(-1)
	, m_frameCount(-1)
	, m_outdated(false) //must be false or the initial fetchPixmap() in the parent ctor won't work
{
}

KGameRendererClient::KGameRendererClient(KGameRenderer* renderer, const QString& spriteKey)
	: d(new KGameRendererClientPrivate(renderer, spriteKey, this))
{
	renderer->d->m_clients << this;
	d->fetchPixmap();
}

KGameRendererClient::~KGameRendererClient()
{
	d->m_renderer->d->m_clients.removeAll(this);
	delete d;
}

KGameRenderer* KGameRendererClient::renderer() const
{
	return d->m_renderer;
}

QString KGameRendererClient::spriteKey() const
{
	return d->m_spriteKey;
}

void KGameRendererClient::setSpriteKey(const QString& spriteKey)
{
	if (d->m_spriteKey != spriteKey)
	{
		d->m_spriteKey = spriteKey;
		d->fetchPixmap();
	}
}

int KGameRendererClient::frameCount() const
{
	return d->m_frameCount;
}

int KGameRendererClient::frame() const
{
	return d->m_frame;
}

void KGameRendererClient::setFrame(int frame)
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

QPixmap KGameRendererClient::pixmap() const
{
	return d->m_pixmap;
}

QSize KGameRendererClient::renderSize() const
{
	return d->m_renderSize;
}

void KGameRendererClient::setRenderSize(const QSize& renderSize)
{
	if (d->m_renderSize != renderSize)
	{
		d->m_renderSize = renderSize;
		d->fetchPixmap();
	}
}

void KGameRendererClientPrivate::fetchPixmap()
{
	if (!m_outdated)
	{
		m_outdated = true;
		QTimer::singleShot(0, this, SLOT(fetchPixmapInternal()));
	}
}

void KGameRendererClientPrivate::fetchPixmapInternal()
{
	if (m_outdated)
	{
		//update frame count, normalize frame number (for animated frames)
		m_frameCount = m_renderer->frameCount(m_spriteKey);
		m_frameBaseIndex = m_renderer->frameBaseIndex();
		if (m_frameCount <= 0)
		{
			m_frame = -1;
		}
		else if (m_frame >= 0)
		{
			m_frame = (m_frame - m_frameBaseIndex) % m_frameCount + m_frameBaseIndex;
		}
		//actually fetch pixmap
		m_pixmap = m_renderer->spritePixmap(m_spriteKey, m_renderSize, m_frame);
		m_outdated = false;
		m_parent->recievePixmap(m_pixmap);
	}
}
