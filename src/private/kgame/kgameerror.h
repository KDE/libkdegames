/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>

    SPDX-License-Identifier: LGPL-2.0-only
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
