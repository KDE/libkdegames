/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/*
    $Id$
*/

#include "kgameerror.h"
#include "kgamemessage.h"

#include <klocale.h>

QByteArray KGameError::errVersion(int remoteVersion)
{
 QByteArray b;
 QDataStream s(b, IO_WriteOnly);
 s << (Q_INT32)KGameMessage::version();
 s << (Q_INT32)remoteVersion;
 return b;
}

QByteArray KGameError::errCookie(int localCookie, int remoteCookie)
{
 QByteArray b;
 QDataStream s(b, IO_WriteOnly);
 s << (Q_INT32)localCookie;
 s << (Q_INT32)remoteCookie;
 return b;
}

QString KGameError::errorText(int errorCode, const QByteArray& message)
{
 QString text;
 QDataStream s(message, IO_ReadOnly);
 switch (errorCode) {
	case Cookie:
	{
		Q_INT32 cookie1; 
		Q_INT32 cookie2;
		s >> cookie1;
		s >> cookie2;
		text = i18n("Cookie mismatch!\n");
		break;
	}
	case Version:
	{
		Q_INT32 cookie1; 
		Q_INT32 cookie2;
		s >> cookie1;
		s >> cookie2;
		text = i18n("KGame Version mismatch!\n");
		break;
	}
	default:
		text = i18n("Unknown error code %1").arg(errorCode);
 }
 return text;
}

