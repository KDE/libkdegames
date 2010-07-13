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
#include <QtSvg/QSvgRenderer>
#include <KGameTheme>
#include <KImageCache>

class KGameRendererPrivate
{
	public:
		KGameRendererPrivate(const QString& defaultTheme);
		bool setTheme(const QString& theme);
		bool instantiateRenderer();
		inline QString spriteFrameKey(const QString& key, int frame) const;

		QString m_defaultTheme, m_currentTheme;
		QString m_frameSuffix, m_sizePrefix, m_frameCountPrefix;
		int m_frameBaseIndex;
		KGameTheme m_theme;
		QSvgRenderer* m_renderer;
		QList<KGameRendererClient*> m_clients;

		//NOTE: See ctor implementation for why we do not use KImageCache's pixmap cache.
		KImageCache m_imageCache;
		QHash<QString, QPixmap> m_pixmapCache;
		QHash<QString, int> m_frameCountCache;
};

class KGameRendererClientPrivate : public QObject
{
	Q_OBJECT
	public:
		KGameRendererClientPrivate(KGameRenderer* renderer, const QString& spriteKey, KGameRendererClient* parent);
	public Q_SLOTS:
		void fetchPixmap();
		void fetchPixmapInternal();
	public:
		KGameRendererClient* m_parent;
		KGameRenderer* m_renderer;

		QPixmap m_pixmap;
		QString m_spriteKey;
		QSize m_renderSize;
		int m_frame, m_frameCount, m_frameBaseIndex;
		bool m_outdated;
};

#endif // KGAMERENDERER_P_H
