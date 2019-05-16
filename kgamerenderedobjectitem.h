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

#ifndef KGAMERENDEREDOBJECTITEM_H
#define KGAMERENDEREDOBJECTITEM_H

#include <QObject>
#include <QGraphicsItem>
class QGraphicsView;

#include <kgamerendererclient.h>
#include <libkdegames_export.h>

class KGameRenderedObjectItemPrivate;

/**
 * @class KGameRenderedObjectItem kgamerenderedobjectitem.h <KGameRenderedObjectItem>
 * @since 4.6
 * @short A QGraphicsObject which displays pixmaps from a KGameRenderer.
 *
 * This item displays a pixmap which is retrieved from a KGameRenderer, and is
 * updated automatically when the KGameRenderer changes the theme.
 *
 * The item has built-in handling for animated sprites (i.e. those with multiple
 * frames). It is a QGraphicsObject and exposes a "frame" property, so you can
 * easily run the animation by plugging in a QPropertyAnimation.
 *
 * @section operationalmodes Modes of operation
 *
 * By default, this item behaves just like a QGraphicsPixmapItem. The size of
 * its bounding rect is equal to the size of the pixmap, i.e. the renderSize().
 *
 * However, the KGameRenderedObjectItem has a second mode of operation, which is
 * enabled by setting a "primary view". (This can be done automatically via
 * KGameRenderer::setDefaultPrimaryView.)
 *
 * If such a primary view is set, the following happens:
 * \li The renderSize of the pixmap is automatically determined from the
 *     painting requests received from the primary view (manual calls to
 *     setRenderSize() are unnecessary and need to be avoided).
 * \li The size of the item's boundingRect() is independent of the renderSize().
 *     The default fixedSize() is 1x1, which means that the item's bounding rect
 *     is the unit square (moved by the configured offset()).
 */
class KDEGAMES_EXPORT KGameRenderedObjectItem : public QGraphicsObject, public KGameRendererClient
{
	Q_OBJECT
	Q_PROPERTY(int frame READ frame WRITE setFrame)
	public:
		///Creates a new KGameRenderedObjectItem which renders the sprite with
		///the given @a spriteKey as provided by the given @a renderer.
		KGameRenderedObjectItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent = nullptr);
		virtual ~KGameRenderedObjectItem();

		///@return the item's offset, which defines the point of the top-left
		///corner of the bounding rect, in local coordinates.
		QPointF offset() const;
		///Sets the item's offset, which defines the point of the top-left
		///corner of the bounding rect, in local coordinates.
		void setOffset(const QPointF& offset);
		///@overload
		void setOffset(qreal x, qreal y);
		///@return the fixed size of this item (or (-1, -1) if this item has no
		///primary view)
		QSizeF fixedSize() const;
		///Sets the fixed size of this item, i.e. the guaranteed size of the
		///item. This works only when a primary view has been set.
		void setFixedSize(const QSizeF& size);

		///Returns a pointer to the current primary view, or 0 if no primary
		///view has been set (which is the default).
		///@see setPrimaryView()
		QGraphicsView* primaryView() const;
		///Sets the primary view of this item. (See class documentation for what
		///the primary view does.) Pass a null pointer to just disconnect from
		///the current primary view. The fixed size is then reset to (-1, -1).
		///If a primary view is set, the fixed size is initialized to (1, 1).
		///@warning While a primary view is set, avoid any manual calls to
		///setRenderSize().
		///@see {Modes of operation}
		void setPrimaryView(QGraphicsView* view);

		//QGraphicsItem reimplementations (see comment in source file for why we need all of this)
		QRectF boundingRect() const override;
		bool contains(const QPointF& point) const override;
		bool isObscuredBy(const QGraphicsItem* item) const override;
		QPainterPath opaqueArea() const override;
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;
		QPainterPath shape() const override;
	protected:
		void receivePixmap(const QPixmap& pixmap) override;
	private:
		friend class KGameRenderedObjectItemPrivate;
		KGameRenderedObjectItemPrivate* const d;
};

#endif // KGAMERENDEREDOBJECTITEM_H
