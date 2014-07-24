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

#include <kdeclarative/kdeclarative.h>

KgDeclarativeView::KgDeclarativeView(QWidget *parent) :
    QQuickWidget(parent),
    d(0) //unused for now, for future expandability
{
    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    
    // binds things like i18n and icons
    kdeclarative.setupBindings();

    setResizeMode(SizeRootObjectToView);
}

#include "kgdeclarativeview.moc"
