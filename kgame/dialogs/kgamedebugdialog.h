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

#ifndef __KGAMEDEBUGDIALOG_H__
#define __KGAMEDEBUGDIALOG_H__

#include <kdialogbase.h>

class KGame;
class KGameIO;
class KPlayer;
class KGamePropertyBase;

class KGameDebugDialogPrivate;

class KGameDebugDialog : public KDialogBase
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
	void updateGameData();

	/**
	 * Update the properties of the currently selected player
	 **/
	void updatePlayerData();

	/**
	 * Updates the list of players and calls @ref clearPlayerData. Note that
	 * after this call NO player is selected anymore.
	 **/
	void updatePlayerList();

signals:
	/**
	 * As it is not possible to find out which type a @ref KGameProperty has
	 * you have to provide its value first. This signal is emitted when the
	 * value for the property prop is requested - it is not emitted for
	 * default types (like @ref KPlayer::name or @ref KPlayer::group). You
	 * probably want to connect a slot to this signal which tests for the
	 * @ref KGamePropertyBase::id and sets the value according to this id.
	 * Example:
	 * <pre>
	 * switch (prop->id()) {
	 * 	case myPropertyId1:
	 * 		value = (KGameProperty<QString>*)->value();
	 * 		break;
	 * 	case myPropertyId2:
	 * 		value = QString::number((KGamePropertyInt*)prop->value());
	 * 		break;
	 * 	default:
	 * 		value = i18n("Unknown");
	 * 		break;
	 * }
	 * @param prop The @ref KGameProperty the value is requested for
	 * @param value The value of this property. You have to set this.
	 **/
	void signalRequestValue(KGamePropertyBase* prop, QString& value);

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

protected slots:
	/**
	 * Update the data of the player specified in item
	 * @param item The @ref QListBoxItem of the player to be updated. Note
	 * that the text of this item MUST be the ID of the player
	 **/
	void updatePlayerData(QListBoxItem* item);

	/**
	 * Add a new player to the player list
	 **/
	void addPlayer(KPlayer* p);

	/**
	 * Remove a player from the list
	 **/
	void removePlayer(QListBoxItem* item);

private:
	void initGamePage();
	void initPlayerPage();

private:
	KGameDebugDialogPrivate* d;
};


#endif
