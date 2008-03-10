/*
    This file is part of the KDE games library

    Copyright 2008 Andreas Pakulat <apaku@gmx.de>

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

#ifndef __CARDCACHE_P_H_
#define __CARDCACHE_P_H_

#include <QThread>
#include <QString>
#include <QSize>
#include "cardcache.h"

class KPixmapCache;
class QMutex;
class KSvgRenderer;
class LoadThread;

class KCardCachePrivate
{
public:
    KPixmapCache* frontcache;
    KPixmapCache* backcache;
    QMutex* frontcacheMutex;
    QMutex* backcacheMutex;
    QMutex* backRendererMutex;
    QMutex* frontRendererMutex;
    LoadThread* loadThread;
    QSize size;
    QString frontTheme;
    QString backTheme;
    KSvgRenderer* backRenderer;
    KSvgRenderer* frontRenderer;
    QPixmap renderFrontSvg( const QString& element );
    QPixmap renderBackSvg( const QString& element );
    void ensureNonNullPixmap( QPixmap& pix );
};

class LoadThread : public QThread
{
    Q_OBJECT
public:
    LoadThread( KCardCache::LoadInfos infos, KCardCachePrivate* d );
    void setSize( const QSize& s );
    void setFrontTheme( const QString& frontTheme );
    void setBackTheme( const QString& backTheme );
    void run();
    void kill();
private:
    KCardCachePrivate* d;
    QString backTheme;
    QString frontTheme;
    QSize size;
    KCardCache::LoadInfos infos;
    bool doKill;
    QMutex* killMutex;
};

#endif
