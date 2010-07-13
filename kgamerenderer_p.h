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

#ifndef KGAMERENDERER_P_H
#define KGAMERENDERER_P_H

#include <QtCore/QHash>
#include <QtCore/QMetaType>
#include <QtCore/QRunnable>
#include <QtCore/QThreadPool>
#include <QtSvg/QSvgRenderer>
#include <KGameTheme>
#include <KImageCache>

namespace KGRInternal
{
	//Describes the state of a KGameRendererClient.
	struct ClientSpec
	{
		inline ClientSpec(const QString& spriteKey = QString(), int frame = -1, const QSize& size = QSize());
		QString spriteKey;
		int frame;
		QSize size;
	};
	ClientSpec::ClientSpec(const QString& spriteKey_, int frame_, const QSize& size_)
		: spriteKey(spriteKey_)
		, frame(frame_)
		, size(size_)
	{
	}

	//Describes a rendering job which is delegated to a worker thread.
	struct Job
	{
		QSvgRenderer* renderer;
		ClientSpec spec;
		QString cacheKey, elementKey;
		QImage result;
	};

	//Describes a worker thread.
	class Worker : public QRunnable
	{
		public:
			Worker(Job* job, bool isSynchronous, KGameRendererPrivate* parent);

			virtual void run();
		private:
			Job* m_job;
			bool m_synchronous;
			KGameRendererPrivate* m_parent;
	};
};

Q_DECLARE_METATYPE(KGRInternal::Job*)

class KGameRendererPrivate : public QObject
{
	Q_OBJECT
	public:
		KGameRendererPrivate(const QString& defaultTheme, unsigned cacheSize, KGameRenderer* parent);
		bool setTheme(const QString& theme);
		bool instantiateRenderer(bool force = false);
		inline QString spriteFrameKey(const QString& key, int frame, bool normalizeFrameNo = false) const;
		void requestPixmap(const KGRInternal::ClientSpec& spec, KGameRendererClient* client, QPixmap* synchronousResult = 0);
	private:
		inline void requestPixmap__propagateResult(const QPixmap& pixmap, KGameRendererClient* client, QPixmap* synchronousResult);
	public Q_SLOTS:
		void jobFinished(KGRInternal::Job* job, bool isSynchronous); //NOTE: This is invoked from KGRInternal::Worker::run.
	public:
		KGameRenderer* m_parent;

		QString m_defaultTheme, m_currentTheme;
		QString m_frameSuffix, m_sizePrefix, m_frameCountPrefix, m_boundsPrefix;
		unsigned m_cacheSize;
		int m_frameBaseIndex;
		KGameTheme m_theme;

		QSvgRenderer* m_renderer;
		bool m_rendererValid;
		QThreadPool m_workerPool;

		QHash<KGameRendererClient*, QString> m_clients; //maps client -> cache key of current pixmap
		QStringList m_pendingRequests; //cache keys of pixmaps which are currently being rendered

		KImageCache* m_imageCache;
		//In multi-threaded scenarios, there are two possible ways to use KIC's
		//pixmap cache.
		//1. The worker renders a QImage and stores it in the cache. The main
		//   thread reads the QImage again and converts it into a QPixmap,
		//   storing it inthe pixmap cache for later re-use.
		//i.e. QImage -> diskcache -> QImage -> QPixmap -> pixmapcache -> serve
		//2. The worker renders a QImage and sends it directly to the main
		//   thread, which converts it to a QPixmap. The QPixmap is stored in
		//   KIC's pixmap cache, and converted to QImage to be written to the
		//   shared data cache.
		//i.e. QImage -> QPixmap -> pixmapcache -> serve
		//                      \-> QImage -> diskcache
		//We choose a third way:
		//3. The worker renders a QImage which is converted to a QPixmap by the
		//   main thread. The main thread caches the QPixmap itself, and stores
		//   the QImage in the cache.
		//i.e. QImage -> QPixmap -> pixmapcache -> serve
		//           \-> diskcache
		//As you see, implementing an own pixmap cache saves us one conversion.
		//We therefore disable KIC's pixmap cache because we do not need it.
		QHash<QString, QPixmap> m_pixmapCache;
		QHash<QString, int> m_frameCountCache;
		QHash<QString, QRectF> m_boundsCache;
};

class KGameRendererClientPrivate : public QObject
{
	Q_OBJECT
	public:
		KGameRendererClientPrivate(KGameRenderer* renderer, const QString& spriteKey, KGameRendererClient* parent);
	public Q_SLOTS:
		void fetchPixmap();
	public:
		KGameRendererClient* m_parent;
		KGameRenderer* m_renderer;

		QPixmap m_pixmap;
		KGRInternal::ClientSpec m_spec;
};

#endif // KGAMERENDERER_P_H
