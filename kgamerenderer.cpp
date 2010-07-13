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

#include "kgamerenderer.h"
#include "kgamerenderer_p.h"
#include "kgamerendereditem.h"
#include "kgamerendereditem_p.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QPainter>

//TODO: automatically schedule pre-rendering of animation frames
//TODO: multithreaded SVG loading, rendering (caching only in main thread, see KGRP ctor)
//TODO: API for cache access?
//TODO: fetch and cache QSvgRenderer::boundsOnElement for KPat/KBlocks
//TODO: allow multiple themes/caches (compare KGrTheme for usecase)
//TODO: Merge frameCount and spriteExists by representing non-existence as frameCount == -1 in the cache.
//TODO: Split pixmap-fetching functionality from KGRI into new base class KGameRendererClient.

const int cacheSize = 3 * 1 << 20; //3 * 2 ^ 20 bytes = 3 MiB
static const QString cacheName()
{
	const QString appName = QCoreApplication::instance()->applicationName();
	return QString::fromLatin1("kgamerenderer-%1").arg(appName);
}

KGameRendererPrivate::KGameRendererPrivate(const QString& defaultTheme)
	: m_defaultTheme(defaultTheme)
	, m_frameSuffix(QString::fromLatin1("_%1"))
	, m_sizePrefix(QString::fromLatin1("%1-%2-"))
	, m_frameCountPrefix(QString::fromLatin1("fc-"))
	, m_frameBaseIndex(0)
	, m_renderer(0)
	, m_imageCache(cacheName(), cacheSize)
{
	//Problem: In multi-threaded scenarios, there are two possible ways to use
	//KIC's pixmap cache.
	//1. The worker renders a QImage and stores it in the cache. The main thread
	//   reads the QImage again and converts it into a QPixmap, storing it in
	//   the pixmap cache for later re-use.
	//   i.e. QImage -> diskcache -> QImage -> QPixmap -> pixmapcache -> serve
	//2. The worker renders a QImage and sends it directly to the main thread,
	//   which converts it to a QPixmap. The QPixmap is stored in KIC's pixmap
	//   cache, and converted to QImage to be written to the shared data cache.
	//   i.e. QImage -> QPixmap -> pixmapcache -> serve
	//                         \-> QImage -> diskcache
	//We choose a third way:
	//3. The worker renders a QImage which is converted to a QPixmap by the main
	//   thread. The main thread caches the QPixmap itself, and stores the
	//   QImage in the cache.
	//   i.e. QImage -> QPixmap -> pixmapcache -> serve
	//              \-> diskcache
	//As you see, implementing an own pixmap cache saves us one conversion. We
	//therefore disable KIC's pixmap cache because we do not need it.
	m_imageCache.setPixmapCaching(false);
}

KGameRenderer::KGameRenderer(const QString& theme, const QString& defaultTheme)
	: d(new KGameRendererPrivate(defaultTheme))
{
	setTheme(theme);
}

KGameRenderer::~KGameRenderer()
{
	qDeleteAll(d->m_itemInstances);
	delete d->m_renderer;
	delete d;
}

int KGameRenderer::frameBaseIndex() const
{
	return d->m_frameBaseIndex;
}

void KGameRenderer::setFrameBaseIndex(int frameBaseIndex)
{
	d->m_frameBaseIndex = frameBaseIndex;
}

QString KGameRenderer::frameSuffix() const
{
	return d->m_frameSuffix;
}

void KGameRenderer::setFrameSuffix(const QString& suffix)
{
	d->m_frameSuffix = suffix;
}

QString KGameRenderer::theme() const
{
	return d->m_currentTheme;
}

void KGameRenderer::setTheme(const QString& theme)
{
	const QString oldTheme = d->m_currentTheme;
	if (oldTheme == theme)
	{
		return;
	}
	if (!d->setTheme(theme))
	{
		d->setTheme(d->m_defaultTheme);
	}
	//announce change to KGameRenderedItems
	QList<KGameRenderedItem*>::const_iterator it1 = d->m_itemInstances.begin();
	QList<KGameRenderedItem*>::const_iterator it2 = d->m_itemInstances.end();
	for (; it1 != it2; ++it1)
	{
		(*it1)->d->fetchPixmap();
	}
	//announce change publicly
	if (oldTheme != d->m_currentTheme)
	{
		emit themeChanged(d->m_currentTheme);
	}
}

bool KGameRendererPrivate::setTheme(const QString& theme)
{
	if (theme.isEmpty())
	{
		return false;
	}
	//load desktop file
	if (!m_theme.load(theme))
	{
		m_theme.load(m_currentTheme);
		return false;
	}
	//check if cache contains the right theme
	QByteArray cachedTheme;
	m_imageCache.find(QString::fromLatin1("kgr_theme"), &cachedTheme);
	const bool themeIsCached = QString::fromUtf8(cachedTheme) == theme;
	//if necessary, load renderer
	if (!themeIsCached)
	{
		if (!instantiateRenderer())
		{
			//In this case, we tried to load a theme for the first time, and
			//found that the SVG file is not installed properly. The command
			//order in this method is such that we can still deny to change
			//the theme without breaking the previous theme.
			return false;
		}
	}
	//theme is cached - just delete the old renderer
	else if (m_currentTheme != theme)
	{
		delete m_renderer;
		m_renderer = 0;
	}
	//clear caches
	if (!themeIsCached)
	{
		m_pixmapCache.clear();
		m_frameCountCache.clear();
		m_imageCache.clear();
		m_imageCache.insert(QString::fromLatin1("kgr_theme"), theme.toUtf8());
	}
	//done
	m_currentTheme = theme;
	return true;
}

bool KGameRendererPrivate::instantiateRenderer()
{
	QSvgRenderer* oldRenderer = m_renderer;
	m_renderer = new QSvgRenderer(m_theme.graphics());
	if (m_renderer->isValid())
	{
		delete oldRenderer;
		return true;
	}
	else
	{
		delete m_renderer;
		m_renderer = oldRenderer;
		return false;
	}
}

const KGameTheme* KGameRenderer::gameTheme() const
{
	return &d->m_theme;
}

QString KGameRendererPrivate::spriteFrameKey(const QString& key, int frame) const
{
	return frame >= 0 ? key + m_frameSuffix.arg(frame) : key;
}

int KGameRenderer::frameCount(const QString& key) const
{
	//look up in in-process cache
	QHash<QString, int>::const_iterator it = d->m_frameCountCache.find(key);
	if (it != d->m_frameCountCache.end())
	{
		return it.value();
	}
	//look up in shared cache
	int count = d->m_frameBaseIndex;
	bool countFound = false;
	const QString cacheKey = d->m_frameCountPrefix + key;
	if (!d->m_renderer)
	{
		QByteArray buffer;
		if (d->m_imageCache.find(cacheKey, &buffer))
		{
			count = buffer.toInt();
			countFound = true;
		}
	}
	//determine from SVG
	if (!countFound)
	{
		d->instantiateRenderer();
		while (d->m_renderer->elementExists(d->spriteFrameKey(key, count)))
		{
			++count;
		}
		count -= d->m_frameBaseIndex;
		if (count == 0)
		{
			count = -1;
		}
		d->m_imageCache.insert(cacheKey, QByteArray::number(count));
	}
	d->m_frameCountCache.insert(key, count);
	return count;
}

bool KGameRenderer::spriteExists(const QString& key) const
{
	//TODO: Is there a way to determine existence without loading the renderer?
	if (!d->m_renderer)
	{
		if (!d->instantiateRenderer())
			return false;
	}
	//check for existence of non-animated sprite
	if (d->m_renderer->elementExists(key))
	{
		return true;
	}
	//check for existence of animated sprite
	else
	{
		return d->m_renderer->elementExists(d->spriteFrameKey(key, 0));
	}
}

QPixmap KGameRenderer::spritePixmap(const QString& key, const QSize& size, int frame) const
{
	if (size.isEmpty())
	{
		return QPixmap();
	}
	//generate key
	const QString elementKey = d->spriteFrameKey(key, frame);
	const QString cacheKey = d->m_sizePrefix.arg(size.width()).arg(size.height()) + elementKey;
	//check pixmap cache
	QHash<QString, QPixmap>::const_iterator it = d->m_pixmapCache.find(cacheKey);
	if (it != d->m_pixmapCache.end())
	{
		return it.value();
	}
	//check (slower) image cache
	QPixmap pix;
	if (!d->m_imageCache.findPixmap(cacheKey, &pix))
	{
		if (!d->m_renderer)
		{
			if (!d->instantiateRenderer())
			{
				return QPixmap();
			}
		}
		//generate pixmap from SVG
		pix = QPixmap(size);
		pix.fill(Qt::transparent);
		QPainter painter(&pix);
		d->m_renderer->render(&painter, elementKey);
		painter.end();
		//store in cache for next access
		d->m_imageCache.insertPixmap(cacheKey, pix);
	}
	d->m_pixmapCache.insert(cacheKey, pix);
	return pix;
}

#include "kgamerenderer.moc"
