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

#ifndef KGAMERENDEREDITEM_H
#define KGAMERENDEREDITEM_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>

#include <kgamerendererclient.h>
#include <libkdegames_export.h>

class KGameRenderedItemPrivate;

/**
 * @class KGameRenderedItem
 * @since 4.6
 * @short A QGraphicsPixmapItem which reacts to theme changes automatically.
 *
 * This class is a QGraphicsPixmapItem which retrieves its pixmap from a
 * KGameRenderer, and updates it automatically when the KGameRenderer changes
 * the theme.
 */
class KDEGAMES_EXPORT KGameRenderedItem : public QGraphicsPixmapItem, public KGameRendererClient
{
	public:
		///Creates a new KGameRenderedItem which renders the sprite with the
		///given @a spriteKey as provided by the given @a renderer.
		KGameRenderedItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent = 0);
		virtual ~KGameRenderedItem();
	protected:
		virtual void receivePixmap(const QPixmap& pixmap);
	private:
		friend class KGameRenderedItemPrivate;
		KGameRenderedItemPrivate* const d;
};

#endif // KGAMERENDEREDITEM_H
