/* **************************************************************************
                           KGameErrorDialog Class
                           -------------------
    begin                : 1 January 2001
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

#ifndef KGAMEERRORDIALOG_H
#define KGAMEERRORDIALOG_H

#include <kdialogbase.h>

class KGame;

/**
 * Use error(), warning() and information() to display the information about a
 * network game. Maybe a better solution is to use KMessageBoxes
 * You can connect to the public slots, too - they will call the static
 * functions, so that you can always have a KGameErrorDialog object lying around
 * without loosing much memory (a KGameErrorMessageDialog Object will be
 * created)
 * @short Error handling for KGame
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameErrorDialog : public QObject
{
	Q_OBJECT
public:
	KGameErrorDialog(QWidget* parent);
	~KGameErrorDialog();

	/**
	 * Automatically connects the KGame object to all error dependant slots. 
	 * Create a KGameErrorDialog object, call this function and forget
	 * everything.
	 * @param g The KGame which will emit the erorrs (or not ;-) )
	 **/
	void setKGame(const KGame* g);

	/**
	 * KGame couldn't establish a connection. Use this if
	 * KGame::initConnection returns false
	 * @param s A string that describes the error further (like port is
	 * already in use). Will be ignored if QString::null
	 **/
	void connectionError(QString s = QString::null);

public slots:
	void slotError(int error, QString text);
	void slotVersionError(Q_UINT32 client);
//	void slotConnectionLost(KGameClient* c);//FIXME

protected:
	void error(const QString& errorText, QWidget* parent = 0);

private:
	const KGame* mGame;
};

/**
 * The real class for error messages. KGameErrorDialog uses this to create error
 * messages (not yet).
 * Use @ref KGameErrorDialog instead.
 * @short Internally used by @ref KGameErrorDialog
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameErrorMessageDialog : public KDialogBase
{
	Q_OBJECT
public:
	KGameErrorMessageDialog(QWidget* parent);
	~KGameErrorMessageDialog();

private:
};

#endif
