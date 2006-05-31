/*
    This file is part of the KDE games library
    Copyright (C) 2001,2002,2003 Nicolas Hadacek (hadacek@kde.org)

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

#include "kcanvasrootpixmap.h"

#include <q3canvas.h>
#include <QPixmap>


KCanvasRootPixmap::KCanvasRootPixmap(Q3CanvasView *view)
    : KRootPixmap(view), _view(view)
{
    setCustomPainting(true);
    connect(this, SIGNAL(backgroundUpdated(const QPixmap &)),
            SLOT(backgroundUpdatedSlot(const QPixmap &)));
}

void KCanvasRootPixmap::backgroundUpdatedSlot(const QPixmap &pixmap)
{
    if ( _view && _view->canvas() )
        _view->canvas()->setBackgroundPixmap(pixmap);
}

#include "kcanvasrootpixmap.moc"
