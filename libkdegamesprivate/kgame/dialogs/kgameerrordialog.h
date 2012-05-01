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

#ifndef __KGAMEERRORDIALOG_H__
#define __KGAMEERRORDIALOG_H__

#include <kdialog.h>

class KGame;
class KGameErrorDialogPrivate;

/**
 * \class KGameErrorDialog kgameerrordialog.h <KGameErrorDialog>
 * 
 * Use error(), warning() and information() to display the information about a
 * network game. Maybe a better solution is to use KMessageBoxes
 * You can connect to the public slots, too - they will call the static
 * functions, so that you can always have a KGameErrorDialog object lying around
 * without losing much memory (a KGameErrorMessageDialog Object will be
 * created)
 * @short Error handling for KGame
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameErrorDialog : public QObject
{
	Q_OBJECT
public:
	explicit KGameErrorDialog(QWidget* parent);
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
	 * already in use). Will be ignored if QString()
	 **/
	void connectionError(const QString& s = QString());

public Q_SLOTS:
	void slotError(int error, const QString& text);
	
	/**
	 * The connection to the @ref KMessageServer has been lost
	 *
	 * See @ref KGameNetwork::signalConnectionBroken
	 **/
	void slotServerConnectionLost();

	/**
	 * The connection to a client has been lost by accident
	 *
	 * See @ref KGameNetwork::signalClientDisconnected
	 **/
	void slotClientConnectionLost(quint32 clientID,bool broken);
	
	/**
	 * Unsets a @ref KGame which has been set using @ref setKGame before.
	 * This is called automatically when the @ref KGame object is destroyed
	 * and you normally don't have to call this yourself.
	 *
	 * Note that @ref setKGame also unsets an already existing @ref KGame
	 * object if exising.
	 **/
	void slotUnsetKGame();

protected:
	void error(const QString& errorText, QWidget* parent = 0);

private:
	KGameErrorDialogPrivate* d;
};

/**
 * \class KGameErrorMessageDialog kgameerrordialog.h <KGameErrorDialog>
 * 
 * The real class for error messages. KGameErrorDialog uses this to create error
 * messages (not yet).
 * Use @ref KGameErrorDialog instead.
 * @short Internally used by @ref KGameErrorDialog
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameErrorMessageDialog : public KDialog
{
	Q_OBJECT
public:
	KGameErrorMessageDialog(QWidget* parent);
	~KGameErrorMessageDialog();

private:
};

#endif
