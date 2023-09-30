/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kchatbaseitemdelegate.h"

// own
#include "kchatbasemodel.h"
// KF
#include <KLocalizedString>
// Qt
#include <QPainter>

KChatBaseItemDelegate::KChatBaseItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

KChatBaseItemDelegate::~KChatBaseItemDelegate()
{
}

void KChatBaseItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //  qDebug() << "KChatBaseItemDelegate::paint";
    KChatBaseMessage m = index.model()->data(index, Qt::DisplayRole).value<KChatBaseMessage>();
    paint(painter, option, index, m.first, m.second);
}

void KChatBaseItemDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index,
                                  const QString &sender,
                                  const QString &message) const
{
    //  qDebug() << "KChatBaseItemDelegate::paint";
    painter->setFont(((KChatBaseModel *)index.model())->nameFont());
    painter->drawText(option.rect.x(), QFontMetrics(option.font).height() + option.rect.y(), i18n("%1: ", sender));
    painter->setFont(((KChatBaseModel *)index.model())->messageFont());
    painter->drawText(option.rect.x() + 3 + QFontMetrics(((KChatBaseModel *)index.model())->nameFont()).boundingRect(i18n("%1: ", sender)).width(),
                      QFontMetrics(option.font).height() + option.rect.y(),
                      message);
}

QSize KChatBaseItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //   qDebug() << "KChatBaseItemDelegate::sizeHint";
    KChatBaseMessage m = index.model()->data(index, Qt::DisplayRole).value<KChatBaseMessage>();
    return sizeHint(option, index, m.first, m.second);
}

QSize KChatBaseItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index, const QString &sender, const QString &message) const
{
    //   qDebug() << "KChatBaseItemDelegate::sizeHint";
    int w = 0;
    w += 6;
    w += QFontMetrics(option.font).boundingRect(sender + i18n("%1: ", sender)).width();
    w += QFontMetrics(option.font).boundingRect(message).width();
    int h = 0;
    h += 2;
    if (QFontMetrics(((KChatBaseModel *)index.model())->nameFont()).lineSpacing()
        > QFontMetrics(((KChatBaseModel *)index.model())->messageFont()).lineSpacing()) {
        h += QFontMetrics(((KChatBaseModel *)index.model())->nameFont()).lineSpacing();
    } else {
        h += QFontMetrics(((KChatBaseModel *)index.model())->messageFont()).lineSpacing();
    }
    return QSize(w, h);
}

#include "moc_kchatbaseitemdelegate.cpp"
