/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (kde at heni-online.de)

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

#include "kgameerror.h"

// own
#include "kgamemessage.h"
// KF
#include <KLocalizedString>

QByteArray KGameError::errVersion(int remoteVersion)
{
 QByteArray b;
 QDataStream s(&b, QIODevice::WriteOnly);
 s << (qint32)KGameMessage::version();
 s << (qint32)remoteVersion;
 return b;
}

QByteArray KGameError::errCookie(int localCookie, int remoteCookie)
{
 QByteArray b;
 QDataStream s(&b, QIODevice::WriteOnly);
 s << (qint32)localCookie;
 s << (qint32)remoteCookie;
 return b;
}

QString KGameError::errorText(int errorCode, const QByteArray& message)
{
 QDataStream s(message);
 return errorText(errorCode, s);
}

QString KGameError::errorText(int errorCode, QDataStream& s)
{
 QString text;
 switch (errorCode) {
	case Cookie:
	{
		qint32 cookie1; 
		qint32 cookie2;
		s >> cookie1;
		s >> cookie2;
		text = i18n("Cookie mismatch!\nExpected Cookie: %1\nReceived Cookie: %2", cookie1, cookie2);
		break;
	}
	case Version:
	{
		qint32 version1;
		qint32 version2;
		s >> version1;
		s >> version2;
		text = i18n("KGame Version mismatch!\nExpected Version: %1\nReceived Version: %2\n", version1, version2);
		break;
	}
	default:
		text = i18n("Unknown error code %1", errorCode);
 }
 return text;
}

