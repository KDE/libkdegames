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
#ifndef __KCHATBASEITEMDELEGATE_H__
#define __KCHATBASEITEMDELEGATE_H__

#include <QtGui/QAbstractItemDelegate>

#include <libkdegames_export.h>

/**
 * A delegate (see the Qt Model/View module for details) to paint the lines of 
 * the KChatBase list model (@ref KChatBaseModel). 
 *
 * A KChatBaseItemDelegate paints two text items: first the player part then the
 * text part. This honors KChatBase::addMessage which also uses both. 
 *
 * Colors and fonts for both parts are set in the corresponding model. 
 **/
class KDEGAMES_EXPORT KChatBaseItemDelegate : public QAbstractItemDelegate
{
  Q_OBJECT

 public:
    /**
      * Constructs a KChatBaseItemDelegate object
      **/
    explicit KChatBaseItemDelegate(QObject *parent = 0);
    
    /**
      * Destruct a KChatBaseItemDelegate object.
      **/
    virtual ~KChatBaseItemDelegate();

    /**
      * Reimplementation of the default paint method. Draws the item at the 
      * given index in the model with good fonts for player name and message.
      * 
      * Should be reimplemented in inherited delegates
      */
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

    void paint(QPainter *painter, 
		const QStyleOptionViewItem &option,
		const QModelIndex &index,
		const QString& sender,
		const QString& message) const;
    /**
      * Reimplementation of the default sizeHint. Computes the given item size 
      * depending on the name and message sizes and on the fonts they use.
      * 
      * Should be reimplemented in inherited delegates
      */
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index, const QString& sender,
			    const QString& message) const;
};

#endif
