/* **************************************************************************
                           KGameMisc Class
                           -------------------
    begin                : 15 April 2001
    copyright            : (C) 2001 by Andreas Beckermann and Martin Heni 
    email                : b_mann@gmx.de and martin@heni-online.de
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Additional license: Any of the above copyright holders can add an     *
 *   enhanced license which complies with the license of the KDE core      *
 *   libraries so that this file resp. this library is compatible with     *
 *   the KDE core libraries.                                               *
 *   The user of this program shall have the choice which license to use   *
 *                                                                         *
 ***************************************************************************/
/*
    $Id$
*/
#ifndef __KGAMEMSIC_H__
#define __KGAMEMISC_H__

#include <qstring.h>

class KGameMiscPrivate;
/**
 * This class contains several (usually static) functions I really did not know
 * a class for. If you know a class for any of these member s please drop one of
 * the above copyright holders a mail (or just kde-games-devel@kde.org)
 **/
class KGameMisc
{
public:
	KGameMisc();
	~KGameMisc();
	
	static QString randomName();
	
private:
	KGameMiscPrivate* d;
};

#endif
