/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KGAMECONNECTDIALOG_H__
#define __KGAMECONNECTDIALOG_H__

#include <kdialogbase.h>

class KGameConnectDialogPrivate;
class KGameConnectWidgetPrivate;

class KGameConnectWidget : public QWidget
{
	Q_OBJECT
public:
	KGameConnectWidget(QWidget* parent);
	virtual ~KGameConnectWidget();

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

signals:
	void signalNetworkSetup();

private:
	KGameConnectWidgetPrivate* d;

};

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
	KGameConnectDialog(QWidget* parent = 0,int buttonmask=Ok|Cancel);
	virtual ~KGameConnectDialog();
	
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

signals:
	void signalNetworkSetup();

private:
	KGameConnectDialogPrivate* d;
};

#endif
