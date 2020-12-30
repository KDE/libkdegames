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

/**
 * @class KgDeclarativeView
 * @since 4.11
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
 */
class KDEGAMES_EXPORT KgDeclarativeView : public QQuickWidget
{
    Q_OBJECT
public:
    ///Construcs a new KgDeclarativeView with KDE specific environment.
    ///@param parent The parent widget for this view (usually the main
    ///window of the game)
    explicit KgDeclarativeView(QWidget *parent=nullptr);

private:
    class Private;
    Private* const d;

};

#endif //KGDECLARATIVEVIEW_H
