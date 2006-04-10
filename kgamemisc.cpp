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

#include <qstringlist.h>

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
        i18nc( "A list of language typical names ( for games ), separated by spaces",
              "Adam Alex Andreas Andrew Bart Ben Bernd Bill "
              "Chris Chuck Daniel Don Duncan Ed Emily Eric "
              "Gary Greg Harry Ian Jean Jeff Jan Kai Keith Ken "
              "Kirk Marc Mike Neil Paul Rik Robert Sam Sean "
              "Thomas Tim Walter" ) );
    KRandomSequence random;
    return names.at( random.getLong( names.count() ) );
}
