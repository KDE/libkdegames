/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>

    SPDX-License-Identifier: LGPL-2.0-only
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

