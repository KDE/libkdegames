/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGDECLARATIVEVIEW_H
#define KGDECLARATIVEVIEW_H

// own
#include <libkdegames_export.h>
// Qt
#include <QQuickWidget>
// Std
#include <memory>

#if KDEGAMES_ENABLE_DEPRECATED_SINCE(7, 5)

/**
 * @class KgDeclarativeView
 * @short A QDeclarativeView that adds KDE specific module paths and javascript bindings.
 *
 * This class is a QDeclarativeView that sets up a KDE specific environment
 * like custom QML module paths and javascript function bindings (like i18n)
 * to the declarative view.
 *
 * For porting KDE games to QML, the central widget of main window is to
 * be replaced by KgDeclarativeView (inside which all the .qml graphics take
 * place), leaving the toolbars, menubars and statusbars as they are, and
 * updating their specifics via signals from QML to C++ part of the code.
 * @since 4.11
 * @deprecsated Since 7.5, use QQuickWidget and add any used things yourself, like the KLocalizedContext.
 */
class KDEGAMES_EXPORT KgDeclarativeView : public QQuickWidget
{
    Q_OBJECT

public:
    /// Construcs a new KgDeclarativeView with KDE specific environment.
    /// @param parent The parent widget for this view (usually the main
    /// window of the game)
    KDEGAMES_DEPRECATED_VERSION(7, 5, "Use QQuickWidget and add any used things yourself, like the KLocalizedContext.")
    explicit KgDeclarativeView(QWidget *parent = nullptr);
    ~KgDeclarativeView() override;

private:
    std::unique_ptr<class KgDeclarativeViewPrivate> const d;
};

#endif

#endif // KGDECLARATIVEVIEW_H
