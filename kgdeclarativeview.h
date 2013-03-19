/***************************************************************************
 *   Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>                *
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

#ifndef KGDECLARATIVEVIEW_H
#define KGDECLARATIVEVIEW_H

#include <QDeclarativeView>

#include <libkdegames_export.h>

class KgThemeProvider;

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
class KDEGAMES_EXPORT KgDeclarativeView : public QDeclarativeView
{
    Q_OBJECT
public:
    ///Construcs a new KgDeclarativeView with KDE specific environment.
    ///@param parent The parent widget for this view (usually the main
    ///window of the game)
    KgDeclarativeView(QWidget *parent=0);

    ///Registers the @param provider with QML's root context with ID
    ///@param name and constructs a KgImageProvider corresponding
    ///to this @param provider and adds it to the QML engine which
    ///will receive sprite requests
    void registerProvider(const QString& name, KgThemeProvider* provider);

};

#endif //KGDECLARATIVEVIEW_H
