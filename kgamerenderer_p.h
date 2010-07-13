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
		QList<KGameRenderedItem*> m_itemInstances;

		KImageCache m_imageCache;
		QHash<QString, QPixmap> m_highAvailabilityCache;
		QHash<QString, int> m_frameCountCache;
	//NOTE: Why are there two caches? The KImageCache shares the pixmaps between multiple processes (and saves them on disk). The high-availability cache is used purely in-process, and brings a much better performance especially for animations.
};

#endif // KGAMERENDERER_P_H
