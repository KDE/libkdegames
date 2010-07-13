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
 *
 * This item has built-in handling for animated sprites (i.e. those with
 * multiple frames). It is a QGraphicsObject and exposes a "frame" property, so
 * you can easily run the animation by plugging in a QPropertyAnimation.
 */
class KDEGAMES_EXPORT KGameRenderedItem : public QObject, public QGraphicsPixmapItem, public KGameRendererClient
{
	Q_OBJECT
	Q_PROPERTY(int frame READ frame WRITE setFrame)
	//NOTE: Please keep the following list in sync with the declaration of QGraphicsObject as much as possible.
	Q_PROPERTY(QGraphicsItem* parent READ parentItem WRITE setParentItem DESIGNABLE false)
	Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity FINAL)
	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
	Q_PROPERTY(bool visible READ isVisible WRITE setVisible FINAL)
	Q_PROPERTY(QPointF pos READ pos WRITE setPos)
	Q_PROPERTY(qreal x READ x WRITE setX)
	Q_PROPERTY(qreal y READ y WRITE setY)
	Q_PROPERTY(qreal z READ zValue WRITE setZValue)
	Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)
	Q_PROPERTY(qreal scale READ scale WRITE setScale)
	Q_PROPERTY(QPointF transformOriginPoint READ transformOriginPoint WRITE setTransformOriginPoint)
	Q_INTERFACES(QGraphicsItem)
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
