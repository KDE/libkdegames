/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGTHEMESELECTOR_H
#define KGTHEMESELECTOR_H

// own
#include "kgthemeprovider.h"
#include <libkdegames_export.h>
// Qt
#include <QWidget>
// Std
#include <memory>

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

		explicit KgThemeSelector(KgThemeProvider* provider, Options options = DefaultBehavior, QWidget* parent = nullptr);
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
		std::unique_ptr<class KgThemeSelectorPrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KgThemeSelector::Options)

#endif // KGTHEMESELECTOR_H
