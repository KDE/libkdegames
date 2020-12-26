/***************************************************************************
 *   Copyright 2009-2012 Stefan Majewsky <majewsky@gmx.net>                *
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

#ifndef KG_THEMESELECTOR_P_H
#define KG_THEMESELECTOR_P_H

// Qt
#include <QStyledItemDelegate>

class KgThemeDelegate : public QStyledItemDelegate
{
	public:
		enum Roles
		{
			DescriptionRole = Qt::UserRole,
			AuthorRole,
			AuthorEmailRole,
			IdRole //not displayed, but used internally
		};

		explicit KgThemeDelegate(QObject* parent = nullptr);
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		///@note The implementation is independent of @a option and @a index.
		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		QRect thumbnailRect(const QRect& baseRect) const;
};

#endif // KG_THEMESELECTOR_P_H
