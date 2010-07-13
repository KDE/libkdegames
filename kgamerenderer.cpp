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
#include "kgamerendererclient.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QPainter>

//TODO: automatically schedule pre-rendering of animation frames
//TODO: multithreaded SVG loading?
//TODO: API for cache access?
//TODO: check cache timestamp vs. theme/SVG timestamp

const int cacheSize = 3 * 1 << 20; //3 * 2 ^ 20 bytes = 3 MiB
static const QString cacheName(QString theme)
{
	const QString appName = QCoreApplication::instance()->applicationName();
	//e.g. "themes/foobar.desktop" -> "themes/foobar"
	if (theme.endsWith(QLatin1String(".desktop")))
		theme.truncate(theme.length() - 8); //8 = strlen(".desktop")
	return QString::fromLatin1("kgamerenderer-%1-%2").arg(appName).arg(theme);
}

KGameRendererPrivate::KGameRendererPrivate(const QString& defaultTheme)
	: m_defaultTheme(defaultTheme)
	, m_frameSuffix(QString::fromLatin1("_%1"))
	, m_sizePrefix(QString::fromLatin1("%1-%2-"))
	, m_frameCountPrefix(QString::fromLatin1("fc-"))
	, m_boundsPrefix(QString::fromLatin1("br-"))
	, m_frameBaseIndex(0)
	, m_renderer(0)
	, m_imageCache(0)
{
	qRegisterMetaType<KGRInternal::Job*>();
}

KGameRenderer::KGameRenderer(const QString& theme, const QString& defaultTheme)
	: d(new KGameRendererPrivate(defaultTheme))
{
	setTheme(theme);
}

KGameRenderer::~KGameRenderer()
{
	//cleanup clients
	QHash<KGameRendererClient*, QString>::const_iterator it1 = d->m_clients.begin(), it2 = d->m_clients.end();
	for (; it1 != it2; ++it1)
		delete it1.key();
	//cleanup own stuff
	delete d->m_renderer;
	delete d->m_imageCache;
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
	d->m_frameSuffix = suffix.contains(QLatin1String("%1")) ? suffix : QLatin1String("_%1");
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
	//announce change to KGameRendererClients
	QHash<KGameRendererClient*, QString>::iterator it1 = d->m_clients.begin(), it2 = d->m_clients.end();
	for (; it1 != it2; ++it1)
	{
		it1.value().clear(); //because the pixmap is outdated
		it1.key()->d->fetchPixmap();
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
	//open cache
	KImageCache* oldCache = m_imageCache;
	m_imageCache = new KImageCache(cacheName(m_theme.fileName()), cacheSize);
	m_imageCache->setPixmapCaching(false); //see big comment in KGRPrivate class declaration
	//if cache is opened for the first time, try to instantiate renderer immediately
	//FIXME: This logic breaks if the cache evicts the "kgr" key. The proper solution would be a static cacheExists() method in KSharedDataCache (which I have requested already).
	if (!m_imageCache->contains(QString::fromLatin1("kgr")))
	{
		if (instantiateRenderer(true))
		{
			m_imageCache->insert(QString::fromLatin1("kgr"), "1");
		}
		else
		{
			//The SVG file is broken, so we deny to change the theme without
			//breaking the previous theme.
			delete m_imageCache;
			m_imageCache = oldCache;
			return false;
		}
	}
	//theme is cached - just delete the old renderer after making sure that no worker threads are using it anymore
	else if (m_currentTheme != theme)
	{
		m_workerPool.waitForDone();
		delete m_renderer;
		m_renderer = 0;
	}
	//clear caches
	m_pixmapCache.clear();
	m_frameCountCache.clear();
	m_boundsCache.clear();
	//done
	m_currentTheme = theme;
	return true;
}

bool KGameRendererPrivate::instantiateRenderer(bool force /* = false */)
{
	if (!m_renderer || force)
	{
		QSvgRenderer* oldRenderer = m_renderer;
		m_renderer = new QSvgRenderer(m_theme.graphics());
		if (m_renderer->isValid())
		{
			//before the old renderer is deleted, we must make sure that no worker threads are using it anymore
			m_workerPool.waitForDone();
			delete oldRenderer;
			m_rendererValid = true;
		}
		else
		{
			delete m_renderer;
			m_renderer = oldRenderer;
			m_rendererValid = m_renderer->isValid();
			return false;
		}
	}
	return m_rendererValid;
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
	//look up in shared cache (if SVG is not yet loaded)
	int count = -1;
	bool countFound = false;
	const QString cacheKey = d->m_frameCountPrefix + key;
	if (!d->m_renderer)
	{
		QByteArray buffer;
		if (d->m_imageCache->find(cacheKey, &buffer))
		{
			count = buffer.toInt();
			countFound = true;
		}
	}
	//determine from SVG
	if (!countFound)
	{
		if (d->instantiateRenderer())
		{
			//look for animated sprite first
			count = d->m_frameBaseIndex;
			while (d->m_renderer->elementExists(d->spriteFrameKey(key, count)))
			{
				++count;
			}
			count -= d->m_frameBaseIndex;
			//look for non-animated sprite instead
			if (count == 0)
			{
				if (!d->m_renderer->elementExists(key))
				{
					count = -1;
				}
			}
			d->m_imageCache->insert(cacheKey, QByteArray::number(count));
		}
	}
	d->m_frameCountCache.insert(key, count);
	return count;
}

QRectF KGameRenderer::boundsOnSprite(const QString& key, int frame) const
{
	const QString elementKey = d->spriteFrameKey(key, frame);
	//look up in in-process cache
	QHash<QString, QRectF>::const_iterator it = d->m_boundsCache.find(elementKey);
	if (it != d->m_boundsCache.end())
	{
		return it.value();
	}
	//look up in shared cache (if SVG is not yet loaded)
	QRectF bounds;
	bool boundsFound = false;
	const QString cacheKey = d->m_boundsPrefix + elementKey;
	if (!d->m_renderer)
	{
		QByteArray buffer;
		if (d->m_imageCache->find(cacheKey, &buffer))
		{
			QDataStream stream(buffer);
			stream >> bounds;
			boundsFound = true;
		}
	}
	//determine from SVG
	if (!boundsFound)
	{
		if (d->instantiateRenderer())
		{
			bounds = d->m_renderer->boundsOnElement(elementKey);
			//save in shared cache
			QByteArray buffer;
			{
				QDataStream stream(&buffer, QIODevice::WriteOnly);
				stream << bounds;
			}
			d->m_imageCache->insert(cacheKey, buffer);
		}
	}
	d->m_boundsCache.insert(elementKey, bounds);
	return bounds;
}

bool KGameRenderer::spriteExists(const QString& key) const
{
	return this->frameCount(key) >= 0;
}

QPixmap KGameRenderer::spritePixmap(const QString& key, const QSize& size, int frame) const
{
	QPixmap result;
	d->requestPixmap(KGRInternal::ClientSpec(key, frame, size), 0, &result);
	return result;
}

//Helper function for KGameRendererPrivate::requestPixmap.
void KGameRendererPrivate::requestPixmap__propagateResult(const QPixmap& pixmap, KGameRendererClient* client, QPixmap* synchronousResult)
{
	if (client)
	{
		client->receivePixmap(pixmap);
	}
	if (synchronousResult)
	{
		*synchronousResult = pixmap;
	}
}

void KGameRendererPrivate::requestPixmap(const KGRInternal::ClientSpec& spec, KGameRendererClient* client, QPixmap* synchronousResult)
{
	//NOTE: If client == 0, the request is synchronous and must be finished when this method returns. This behavior is used by KGR::spritePixmap(). Instead of KGameRendererClient::receivePixmap, the QPixmap* argument is then used to return the result.
	//parse request
	if (spec.size.isEmpty())
	{
		requestPixmap__propagateResult(QPixmap(), client, synchronousResult);
		return;
	}
	const QString elementKey = spriteFrameKey(spec.spriteKey, spec.frame);
	const QString cacheKey = m_sizePrefix.arg(spec.size.width()).arg(spec.size.height()) + elementKey;
	//check if update is needed
	if (client)
	{
		if (m_clients.value(client) == cacheKey)
		{
			return;
		}
		m_clients[client] = cacheKey;
	}
	//try to serve from high-speed cache
	QHash<QString, QPixmap>::const_iterator it = m_pixmapCache.find(cacheKey);
	if (it != m_pixmapCache.end())
	{
		requestPixmap__propagateResult(it.value(), client, synchronousResult);
		return;
	}
	//try to serve from low-speed cache
	QPixmap pix;
	if (m_imageCache->findPixmap(cacheKey, &pix))
	{
		m_pixmapCache.insert(cacheKey, pix);
		requestPixmap__propagateResult(pix, client, synchronousResult);
		return;
	}
	//if asynchronous request, is such a rendering job already running?
	if (client && m_pendingRequests.contains(cacheKey))
	{
		return;
	}
	//make sure that renderer is available
	if (!instantiateRenderer())
	{
		return;
	}
	//create job
	KGRInternal::Job* job = new KGRInternal::Job;
	job->renderer = m_renderer;
	job->cacheKey = cacheKey;
	job->elementKey = elementKey;
	job->spec = spec;
	const bool synchronous = !client;
	KGRInternal::Worker* worker = new KGRInternal::Worker(job, synchronous, this);
	if (synchronous)
	{
		worker->run();
		//if everything worked fine, result is in high-speed cache now
		*synchronousResult = m_pixmapCache.value(cacheKey);
	}
	else
	{
		m_workerPool.start(new KGRInternal::Worker(job, !client, this));
		m_pendingRequests << cacheKey;
	}
}

void KGameRendererPrivate::jobFinished(KGRInternal::Job* job, bool isSynchronous)
{
	//read job
	const QString cacheKey = job->cacheKey;
	const QImage result = job->result;
	delete job;
	//check who wanted this pixmap
	m_pendingRequests.removeAll(cacheKey);
	const QList<KGameRendererClient*> requesters = m_clients.keys(cacheKey);
	//put result into image cache
	m_imageCache->insertImage(cacheKey, result);
	//convert result to pixmap (and put into pixmap cache) only if it is needed now
	//This optimization saves the image-pixmap conversion for intermediate sizes which occur during smooth resize events or window initializations.
	if (!isSynchronous && requesters.isEmpty())
	{
		return;
	}
	const QPixmap pixmap = QPixmap::fromImage(result);
	m_pixmapCache.insert(cacheKey, pixmap);
	foreach (KGameRendererClient* requester, requesters)
	{
		requester->receivePixmap(pixmap);
	}
}

//BEGIN KGRInternal::Job/Worker

KGRInternal::Worker::Worker(KGRInternal::Job* job, bool isSynchronous, KGameRendererPrivate* parent)
	: m_job(job)
	, m_synchronous(isSynchronous)
	, m_parent(parent)
{
}

static const uint transparentRgba = QColor(Qt::transparent).rgba();

void KGRInternal::Worker::run()
{
	QImage image(m_job->spec.size, QImage::Format_ARGB32_Premultiplied);
	image.fill(transparentRgba);
	QPainter painter(&image);
	m_job->renderer->render(&painter, m_job->elementKey);
	painter.end();
	m_job->result = image;
	//talk back to the main thread
	QMetaObject::invokeMethod(
		m_parent, "jobFinished", Qt::AutoConnection,
		Q_ARG(KGRInternal::Job*, m_job), Q_ARG(bool, m_synchronous)
	);
	//NOTE: KGR::spritePixmap relies on Qt::DirectConnection when this method is run in the main thread.
}

//END KGRInternal::Job/Worker

#include "kgamerenderer.moc"
#include "kgamerenderer_p.moc"
