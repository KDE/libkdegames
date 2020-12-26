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

#ifndef __KGAMEERROR_H_
#define __KGAMEERROR_H_

// Qt
#include <QString>

/**
 * \class KGameError kgameerror.h <KGame/KGameError>
 */
class KGameError
{
public:
	KGameError() { }
	~KGameError() { }

	enum ErrorCodes {
		Cookie = 0, // Cookie mismatch
		Version = 1 // Version mismatch
	};

	/**
	 * Generate an error message with Erorr Code = ErrCookie
	 **/
	static QByteArray errCookie(int localCookie, int remoteCookie);
	static QByteArray errVersion(int remoteVersion);

	/**
	 * Create an erorr text using a QDataStream (QByteArray) which was
	 * created using @ref KGameError. This is the opposite function to all
	 * the errXYZ() function (e.g. @ref errVersion).
	 * You want to use this to generate the message that shall be
	 * displayed to the user.
	 * @return an error message
	 **/
	static QString errorText(int errorCode, QDataStream& message);
	static QString errorText(int errorCode, const QByteArray& message);

};

#endif
