/* **************************************************************************
                           KGameConnectDialog Class
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

#ifndef KGAMECONNECTDIALOG_H
#define KGAMECONNECTDIALOG_H

#include <kdialogbase.h>

class KGameConnectDialogPrivate;

/**
 * This Dialog is used to create a game. You call initConnection(port,
 * QString::null, parent, true) to create a network game (as a server)
 * or initConnection(port, host, parent) to join a network game.
 * @short Dialog to ask for host and port
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameConnectDialog : public KDialogBase
{
	Q_OBJECT
public:
	KGameConnectDialog(QWidget* parent = 0);
	~KGameConnectDialog();
	
	/**
	 * Shows a dialog to either connect to an existing game or to create a
	 * server game, depending on user's choice.
	 * @param port The port the user wants to connect to.
	 * @param host The host the user wants to connect to. Will be
	 * QString::null if server game is chosen
	 * @param parent The parent of the dialog
	 * @param server True to create a network game per default, false to
	 * join a game by default
	 **/
	static int initConnection(unsigned short int& port, QString& host, QWidget* parent, bool server = false);

	/**
	 * @param host The host to connect to by default
	 **/
	void setHost(const QString& host);

	/**
	 * @return The host to connect to or QString::null if the user wants to
	 * be the MASTER
	 **/ 
	QString host() const;

	/**
	 * @param port The port that will be shown by default
	 **/
	void setPort(unsigned short int port);

	/**
	 * @return The port to connect to / to listen
	 **/
	unsigned short int port() const;

	/**
	 * Specifies which state is the default (0 = server game; 1 = join game)
	 * @param state The default state. 0 For a server game, 1 to join a game
	 **/
	void setDefault(int state);

protected slots:
	/**
	 * The type has changed, ie the user switched between creating or
	 * joining.
	 **/
	void slotTypeChanged(int);
	
private:
	KGameConnectDialogPrivate* d;

};

#endif
