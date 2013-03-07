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
#include "kgimageprovider.h"

#include <QDeclarativeEngine>
#include <QDeclarativeContext>

#include <kdeclarative.h>
#include <KgThemeProvider>

KgDeclarativeView::KgDeclarativeView(QWidget *parent) :
    QDeclarativeView(parent)
{
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.initialize();

    // binds things like i18n and icons
    kdeclarative.setupBindings();

    setResizeMode(QDeclarativeView::SizeRootObjectToView);
}

void KgDeclarativeView::registerProvider(QString name, KgThemeProvider* prov)
{
    prov->setName(name);
    engine()->addImageProvider(name, new KgImageProvider(prov));
    engine()->rootContext()->setContextProperty(name, prov);
}

#include "kgdeclarativeview.moc"
