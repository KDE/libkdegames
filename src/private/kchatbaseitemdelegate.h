/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KCHATBASEITEMDELEGATE_H__
#define __KCHATBASEITEMDELEGATE_H__

// own
#include "kdegamesprivate_export.h"
// Qt
#include <QAbstractItemDelegate>

/**
 * \class KChatBaseItemDelegate kchatbaseitemdelegate.h <KChatBaseItemDelegate>
 *
 * A delegate (see the Qt Model/View module for details) to paint the lines of
 * the KChatBase list model (@ref KChatBaseModel).
 *
 * A KChatBaseItemDelegate paints two text items: first the player part then the
 * text part. This honors KChatBase::addMessage which also uses both.
 *
 * Colors and fonts for both parts are set in the corresponding model.
 */
class KDEGAMESPRIVATE_EXPORT KChatBaseItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    /**
     * Constructs a KChatBaseItemDelegate object
     */
    explicit KChatBaseItemDelegate(QObject *parent = nullptr);

    /**
     * Destruct a KChatBaseItemDelegate object.
     */
    ~KChatBaseItemDelegate() override;

    /**
     * Reimplementation of the default paint method. Draws the item at the
     * given index in the model with good fonts for player name and message.
     *
     * Should be reimplemented in inherited delegates
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QString &sender, const QString &message) const;
    /**
     * Reimplementation of the default sizeHint. Computes the given item size
     * depending on the name and message sizes and on the fonts they use.
     *
     * Should be reimplemented in inherited delegates
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index, const QString &sender, const QString &message) const;
};

#endif
