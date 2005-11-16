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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KGAMEDEBUGDIALOG_H__
#define __KGAMEDEBUGDIALOG_H__

#include <kdialogbase.h>
#include <kdemacros.h>
#include <Q3ListBoxItem>
class KGame;
class KGameIO;
class KPlayer;
class KGamePropertyBase;

class KGameDebugDialogPrivate;

class KDE_EXPORT KGameDebugDialog : public KDialogBase
{
	Q_OBJECT
public:
	KGameDebugDialog(KGame* g, QWidget* parent, bool modal = false);
	~KGameDebugDialog();

	/**
	 * Automatically connects the KGame object to all error dependant slots. 
	 * Create a KGameErrorDialog object, call this function and forget
	 * everything.
	 * @param g The KGame which will emit the erorrs (or not ;-) )
	 **/
	void setKGame(const KGame* g);

public slots:
	/**
	 * Unsets a @ref KGame which has been set using @ref setKGame before.
	 * This is called automatically when the @ref KGame object is destroyed
	 * and you normally don't have to call this yourself.
	 *
	 * Note that @ref setKGame also unsets an already existing @ref KGame
	 * object if exising.
	 **/
	void slotUnsetKGame();

	/**
	 * Update the data of the @ref KGame object
	 **/
	void slotUpdateGameData();

	/**
	 * Update the properties of the currently selected player
	 **/
	void slotUpdatePlayerData();

	/**
	 * Updates the list of players and calls @ref clearPlayerData. Note that
	 * after this call NO player is selected anymore.
	 **/
	void slotUpdatePlayerList();

	void slotClearMessages();

signals:
	/**
	 * This signal is emitted when the "debug messages" page couldn't find
	 * the name of a message id. This is usually the case for user-defined
	 * messages. KGameDebugDialog asks you to give the msgid a name.
	 * @param messageid The ID of the message. As given to @ref
	 * KGame::sendMessage
	 * @param userid User defined msgIds are internally increased by
	 * @ref KGameMessage::IdUser. You don't have to care about this but if
	 * this signal is emitted with userid=false (shouldn't happen) then the
	 * name of an internal message as defined in @ref
	 * KGameMessage::GameMessageIds couldn't be found.
	 * @param name The name of the msgid. You have to fill this!
	 **/
	void signalRequestIdName(int messageid, bool userid, QString& name);

protected:
	void clearPages();

	/**
	 * Clear the data of the player view. Note that the player list is NOT
	 * cleared.
	 **/
	void clearPlayerData();

	/**
	 * Clear the data view of the @ref KGame object
	 **/
	void clearGameData();

	/**
	 * Add a new player to the player list
	 **/
	void addPlayer(KPlayer* p);

	/**
	 * Remove a player from the list
	 **/
	void removePlayer(Q3ListBoxItem* item);

	/**
	 * @return Whether messages with this msgid shall be displayed or not
	 **/
	bool showId(int msgid);

protected slots:
	/**
	 * Update the data of the player specified in item
	 * @param item The @ref QListBoxItem of the player to be updated. Note
	 * that the text of this item MUST be the ID of the player
	 **/
	void slotUpdatePlayerData(Q3ListBoxItem* item);

	void slotShowId();
	void slotHideId();

	/**
	 * A message has been received - see @ref KGame::signalMessageUpdate
	 **/
	void slotMessageUpdate(int msgid, quint32 receiver, quint32 sender);

private:
	void initGamePage();
	void initPlayerPage();
	void initMessagePage();

private:
	KGameDebugDialogPrivate* d;
};


#endif
