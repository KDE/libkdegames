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

#include "kgdeclarativeview.h"

// KF
#include <KDeclarative/KDeclarative>
#include <KLocalizedContext>
// Qt
#include <QQmlContext>

KgDeclarativeView::KgDeclarativeView(QWidget *parent) :
    QQuickWidget(parent),
    d(nullptr) //unused for now, for future expandability
{
    QQmlEngine* engine = this->engine();
    KDeclarative::KDeclarative::setupEngine(engine);

    KLocalizedContext *localizedContextObject = new KLocalizedContext(engine);
    engine->rootContext()->setContextObject(localizedContextObject);

    setResizeMode(SizeRootObjectToView);
}


