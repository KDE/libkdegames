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
class QGraphicsView;

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
 *
 * You can use the primaryView property to have your item update its render size
 * automatically.
 *
 * This item has built-in handling for animated sprites (i.e. those with
 * multiple frames). It is a QGraphicsObject and exposes a "frame" property, so
 * you can easily run the animation by plugging in a QPropertyAnimation.
 */
class KDEGAMES_EXPORT KGameRenderedItem : public QGraphicsObject, public KGameRendererClient
{
	Q_OBJECT
	Q_PROPERTY(int frame READ frame WRITE setFrame)
	public:
		///Creates a new KGameRenderedItem which renders the sprite with the
		///given @a spriteKey as provided by the given @a renderer.
		KGameRenderedItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent = 0);
		virtual ~KGameRenderedItem();

		///Returns a pointer to the current primary view, or 0 if no primary
		///view has been set (which is the default).
		///\see setPrimaryView()
		QGraphicsView* primaryView() const;
		///Sets the primary view of this item. If a primary view is set:
		///\li The renderSize of the pixmap is automatically determined from the
		///    painting requests recieved from the primary view.
		///\li The size of the item's boundingRect() is always 1x1 in item
		///    coordinates (without primary view, it equals the pixmap size).
		///\warning While a primary view is set, avoid any manual calls to
		///setRenderSize().
		void setPrimaryView(QGraphicsView* view);

		//QGraphicsItem reimplementations (see comment in source file for why we need all of this)
		virtual QRectF boundingRect() const;
		virtual bool contains(const QPointF& point) const;
		virtual bool isObscuredBy(const QGraphicsItem* item) const;
		virtual QPainterPath opaqueArea() const;
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
		virtual QPainterPath shape() const;
	protected:
		virtual void receivePixmap(const QPixmap& pixmap);
	private:
		friend class KGameRenderedItemPrivate;
		KGameRenderedItemPrivate* const d;
};

#endif // KGAMERENDEREDITEM_H
