/***************************************************************************
 *   Copyright 2012 Stefan Majewsky <majewsky@gmx.net>                     *
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

#ifndef KGTHEMESELECTOR_H
#define KGTHEMESELECTOR_H

#include <QtWidgets/QWidget>
#include <kgthemeprovider.h>

#include <libkdegames_export.h>

/**
 * @class KgThemeSelector kgthemeselector.h <KgThemeSelector>
 * @brief Theme selection widget.
 *
 * This widget allows the user to change the theme selection of a
 * KgThemeProvider. Selections are immediately applied to allow the user
 * to quickly preview themes. In simple cases, the widget can be used
 * standalone with the showAsDialog() method.
 *
 * @code
 * K_GLOBAL_STATIC_WITH_ARGS(KgThemeSelector, selector, (provider))
 * ...
 * selector->showAsDialog();
 * @endcode
 */
class KDEGAMES_EXPORT KgThemeSelector : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(KgThemeSelector)
	public:
		///Flags which control the behavior of KgThemeSelector.
		enum Option {
			DefaultBehavior = 0,
			///Enable downloading of additional themes with KNewStuff3.
			///This requires a KNS3 config file to be installed for this app.
			EnableNewStuffDownload = 1 << 0
		};
		Q_DECLARE_FLAGS(Options, Option)

		explicit KgThemeSelector(KgThemeProvider* provider, Options options = DefaultBehavior, QWidget* parent = 0);
		virtual ~KgThemeSelector();
	public Q_SLOTS:
		///Create and show a non-modal dialog which displays this selector.
		///The dialog will be automatically cleaned up when it's closed, but it
		///is ensured that the selector is not deleted.
		///
		///This method does nothing if the selector widget is already visible.
		void showAsDialog(const QString& caption = QString());
	private:
		class Dialog;
		class Private;
		Private* const d;

		Q_PRIVATE_SLOT(d, void _k_updateListSelection(const KgTheme*));
		Q_PRIVATE_SLOT(d, void _k_updateProviderSelection());
		Q_PRIVATE_SLOT(d, void _k_showNewStuffDialog());
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KgThemeSelector::Options)

#endif // KGTHEMESELECTOR_H
