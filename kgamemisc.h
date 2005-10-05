/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

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
/*
    $Id$
*/
#ifndef __KGAMEMISC_H__
#define __KGAMEMISC_H__

#include <qstring.h>
#include <kdemacros.h>
class KGameMiscPrivate;
/**
 * This class contains several (usually static) functions I really did not know
 * a class for. If you know a class for any of these member s please drop one of
 * the above copyright holders a mail (or just kde-games-devel@kde.org)
 **/
class KDE_EXPORT KGameMisc
{
public:
	KGameMisc();
	~KGameMisc();
	
	static QString randomName();
	
private:
	KGameMiscPrivate* d;
};

#endif
