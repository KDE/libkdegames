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

#ifndef KCANVASROOTPIXMAP_H
#define KCANVASROOTPIXMAP_H

#include <krootpixmap.h>
#include <libkdegames_export.h>

class Q3CanvasView;

/**
 * Implement KRootPixmap for a QCanvasView.
 *
 * The pixmap will be set as the background of the
 * QCanvas associated with the view :
 * <ul>
 * <li>for correct positioning of the background pixmap, the given
 * QCanvasView should be positioned at the origin of the canvas.</li>
 * <li>no other view of the same canvas should use KCanvasRootPixmap.</li>
 * <li>other views of the canvas will have the same background pixmap.</li>
 * </ul>
 */
class KDEGAMES_EXPORT KCanvasRootPixmap : public KRootPixmap
{
 Q_OBJECT

 public:
    /**
     * Constructor.
     */
    KCanvasRootPixmap(Q3CanvasView *view);

 private slots:
    void backgroundUpdatedSlot(const QPixmap &);

 private:
    Q3CanvasView *_view;

    class KCanvasRootPixmapPrivate;
    KCanvasRootPixmapPrivate *d;
};

#endif

