/*
    This file is part of the KDE games library
    Copyright (C) 2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kchatbaseitemdelegate.h"
#include "kchatbasemodel.h"

#include <KLocalizedString>
#include <QPainter>
#include <QDebug>

KChatBaseItemDelegate::KChatBaseItemDelegate(QObject *parent) : 
  QAbstractItemDelegate(parent)
{
}

KChatBaseItemDelegate::~KChatBaseItemDelegate()
{
}

void KChatBaseItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const
{
//  qDebug() << "KChatBaseItemDelegate::paint";
 KChatBaseMessage m  = index.model()->data(index, Qt::DisplayRole).value<KChatBaseMessage>();
 paint(painter, option, index,m.first, m.second);
}

void KChatBaseItemDelegate::paint(QPainter *painter, 
				  const QStyleOptionViewItem &option,
				  const QModelIndex &index,
				   const QString& sender,
				   const QString& message) const
{
//  qDebug() << "KChatBaseItemDelegate::paint";
 QFontMetrics fm = painter->fontMetrics();
 painter->setFont(((KChatBaseModel*)index.model())->nameFont());
 painter->drawText(option.rect.x(), 
		   QFontMetrics(option.font).height()+option.rect.y(), i18n("%1: ",sender));
 painter->setFont(((KChatBaseModel*)index.model())->messageFont());
 painter->drawText(option.rect.x() + 3 + QFontMetrics(((KChatBaseModel*)index.model())->nameFont()).width(i18n("%1: ",sender)),
		   QFontMetrics(option.font).height()+option.rect.y(), message);
}

QSize KChatBaseItemDelegate::sizeHint(const QStyleOptionViewItem &  option ,
		    const QModelIndex &  index ) const
{
//   qDebug() << "KChatBaseItemDelegate::sizeHint";
  KChatBaseMessage m  = index.model()->data(index, Qt::DisplayRole).value<KChatBaseMessage>();
  return sizeHint(option, index, m.first, m.second);
}

QSize KChatBaseItemDelegate::sizeHint(const QStyleOptionViewItem &  option ,
		    const QModelIndex &  index,
				   const QString& sender,
				   const QString& message ) const
{
//   qDebug() << "KChatBaseItemDelegate::sizeHint";
  int w = 0;
  w += 6;
  w += QFontMetrics(option.font).width(sender+i18n("%1: ",sender));
  w += QFontMetrics(option.font).width(message);
  int h = 0;
  h += 2;
  if (QFontMetrics(((KChatBaseModel*)index.model())->nameFont()).lineSpacing() > 
    QFontMetrics(((KChatBaseModel*)index.model())->messageFont()).lineSpacing()) 
  {
    h += QFontMetrics(((KChatBaseModel*)index.model())->nameFont()).lineSpacing();
  } 
  else 
  {
    h += QFontMetrics(((KChatBaseModel*)index.model())->messageFont()).lineSpacing();
  }
  return QSize(w,h);
}


