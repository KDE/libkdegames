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

#include <krandomsequence.h>
#include <klocale.h>

#include "kgamemisc.h"

class KGameMiscPrivate
{
public:
	KGameMiscPrivate()
	{
	}

};

KGameMisc::KGameMisc()
{
// not yet used
// d = new KGamePrivate;
}

KGameMisc::~KGameMisc()
{
 // don't forget to delete it as soon as it is used!
// delete d;
}

QString KGameMisc::randomName()// do we need i18n? I think yes
{
    QStringList names = QStringList::split( QChar(' '),
        i18n( "A list of language typical names ( for games ), separated by spaces",
              "Adam Alex Andreas Andrew Bart Ben Bernd Bill "
              "Chris Chuck Daniel Don Duncan Ed Emily Eric "
              "Gary Greg Harry Ian Jean Jeff Jan Kai Keith Ken "
              "Kirk Marc Mike Neil Paul Rik Robert Sam Sean "
              "Thomas Tim Walter" ) );
    KRandomSequence random;
    return *names.at( random.getLong( names.count() ) );
}
