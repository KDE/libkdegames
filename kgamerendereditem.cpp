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

#include "kgamerendereditem.h"

class KGameRenderedItemPrivate
{
	//NOTE: reserved for later use
};

KGameRenderedItem::KGameRenderedItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent)
	: QGraphicsPixmapItem(parent)
	, KGameRendererClient(renderer, spriteKey)
	, d(0)
{
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

KGameRenderedItem::~KGameRenderedItem()
{
	delete d;
}

void KGameRenderedItem::receivePixmap(const QPixmap& pixmap)
{
	QGraphicsPixmapItem::setPixmap(pixmap);
}

#include "kgamerendereditem.moc"
