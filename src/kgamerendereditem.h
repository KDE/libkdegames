/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMERENDEREDITEM_H
#define KGAMERENDEREDITEM_H

// own
#include "kgamerendererclient.h"
#include <libkdegames_export.h>
// Qt
#include <QObject>
#include <QGraphicsItem>
// Std
#include <memory>

class KGameRenderedItemPrivate;

/**
 * @class KGameRenderedItem kgamerendereditem.h <KGameRenderedItem>
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
		KGameRenderedItem(KGameRenderer* renderer, const QString& spriteKey, QGraphicsItem* parent = nullptr);
		virtual ~KGameRenderedItem();
	protected:
		void receivePixmap(const QPixmap& pixmap) override;
	private:
		friend class KGameRenderedItemPrivate;
		std::unique_ptr<KGameRenderedItemPrivate> const d;
};

#endif // KGAMERENDEREDITEM_H
