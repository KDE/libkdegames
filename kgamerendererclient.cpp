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
	, m_spec(spriteKey, -1, QSize())
{
}

KGameRendererClient::KGameRendererClient(KGameRenderer* renderer, const QString& spriteKey)
	: d(new KGameRendererClientPrivate(renderer, spriteKey, this))
{
	renderer->d->m_clients.insert(this, QString());
	//The following may not be triggered directly because it may call receivePixmap() which is a pure virtual method at this point.
	QTimer::singleShot(0, d, SLOT(fetchPixmap()));
}

KGameRendererClient::~KGameRendererClient()
{
	d->m_renderer->d->m_clients.remove(this);
	delete d;
}

KGameRenderer* KGameRendererClient::renderer() const
{
	return d->m_renderer;
}

QString KGameRendererClient::spriteKey() const
{
	return d->m_spec.spriteKey;
}

void KGameRendererClient::setSpriteKey(const QString& spriteKey)
{
	if (d->m_spec.spriteKey != spriteKey)
	{
		d->m_spec.spriteKey = spriteKey;
		d->fetchPixmap();
	}
}

int KGameRendererClient::frameCount() const
{
	return d->m_renderer->frameCount(d->m_spec.spriteKey);
}

int KGameRendererClient::frame() const
{
	return d->m_spec.frame;
}

void KGameRendererClient::setFrame(int frame)
{
	if (d->m_spec.frame != frame)
	{
		//do some normalization ourselves
		const int frameCount = this->frameCount();
		if (frameCount <= 0 || frame <= 0)
		{
			frame = -1;
		}
		else
		{
			const int frameBaseIndex = d->m_renderer->frameBaseIndex();
			frame = (frame - frameBaseIndex) % frameCount + frameBaseIndex;
		}
		if (d->m_spec.frame != frame)
		{
			d->m_spec.frame = frame;
			d->fetchPixmap();
		}
	}
}

QSize KGameRendererClient::renderSize() const
{
	return d->m_spec.size;
}

void KGameRendererClient::setRenderSize(const QSize& renderSize)
{
	if (d->m_spec.size != renderSize)
	{
		d->m_spec.size = renderSize;
		d->fetchPixmap();
	}
}

QHash<QColor, QColor> KGameRendererClient::customColors() const
{
	return d->m_spec.customColors;
}

void KGameRendererClient::setCustomColors(const QHash<QColor, QColor>& customColors)
{
	if (d->m_spec.customColors != customColors)
	{
		d->m_spec.customColors = customColors;
		d->fetchPixmap();
	}
}

void KGameRendererClientPrivate::fetchPixmap()
{
	m_renderer->d->requestPixmap(m_spec, m_parent);
}
