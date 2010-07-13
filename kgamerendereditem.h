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

#include <libkdegames_export.h>

class KGameRenderedItemPrivate;
class KGameRenderer;
class KGameRendererPrivate;

/**
 * @class KGameRenderedItem
 * @since 4.6
 * @short A QGraphicsPixmapItem which reacts to theme changes automatically.
 *
 * This class is a QGraphicsPixmapItem which retrieves its pixmap from a
 * KGameRenderer, and updates it automatically when the KGameRenderer changes
 * the theme.
 *
 * This item has built-in handling for animated sprites (i.e. those with
 * multiple frames). It is a QGraphicsObject and exposes a "frame" property, so
 * you can easily run the animation by plugging in a QPropertyAnimation.
 */
class KDEGAMES_EXPORT KGameRenderedItem : public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT
	Q_PROPERTY(int frame READ frame WRITE setFrame)
	public:
		///Creates a new KGameRenderedItem which renders the sprite with the
		///given @a spriteKey as provided by the given @a renderer.
		KGameRenderedItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent = 0);
		virtual ~KGameRenderedItem();

		///@return the renderer used by this item
		KGameRenderer* renderer() const;
		///@return the frame count, or -1 for non-animated frames
		///@see KGameRenderer::frameCount()
		int frameCount() const;

		///@return the key of the sprite rendered by this item
		QString spriteKey() const;
		///Defines the key of the sprite which is rendered by this item.
		void setSpriteKey(const QString& spriteKey);
		///@return the current frame number, or -1 for non-animated sprites
		int frame() const;
		///For animated sprites, show another frame. The given frame number is
		///normalized by taking the modulo of the frame count, so the following
		///code works fine:
		///@code
		///    KGameRenderedItem item(...); ...
		///    item.setFrame(item.frame() + 1);  //cycle to next frame
		///    item.setFrame(KRandom::random()); //choose a random frame
		///@endcode
		void setFrame(int frame);
		///@return the size of the pixmap requested from KGameRenderer
		QSize renderSize() const;
		///Defines the size of the pixmap that will be requested from
		///KGameRenderer. You usually want to set this size such that the pixmap
		///does not have to be scaled when it is rendered onto your primary view
		///(for speed reasons).
		///
		///The default render size is very small (width = height = 3 pixels), so
		///that you notice when you forget to set this. ;-)
		void setRenderSize(const QSize& renderSize);
	private:
		friend class KGameRenderedItemPrivate;
		friend class KGameRenderer;
		friend class KGameRendererPrivate;
		KGameRenderedItemPrivate* const d;
};

#endif // KGAMERENDEREDITEM_H
