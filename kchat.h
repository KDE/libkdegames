/* **************************************************************************
                             KGameChat Class
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

#ifndef __KCHAT_H__
#define __KCHAT_H__

#include <qstring.h>

#include "kchatbase.h"

class KChatPrivate;

/**
 * A chat widget for non-KGame use.
 *
 * Docu is TODO
 * @short A chat widget for non-KGame games
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KChat : public KChatBase
{
	Q_OBJECT
public:
	/**
	 * @param twoPlayerGame If true the combo box where the player can
	 * choose to send to a single player or to all players will not be added
	 * as you will hardly need it in 2-player games...
	 **/
	KChat(QWidget* parent, bool twoPlayerGame = false);

	~KChat();

	/**
	 * Equivalent to player(fromId())
	 * @return The name that will be shown for messages from this widget.
	 * That is the string from @ref setFromNickname
	 **/
	virtual const QString& fromName() const;

	/**
	 * This sets the name that will be shown on all chat widgets if this
	 * widget sends a message. See @ref signalSendMessage
	 * @ref The name of the player owning this widget
	 **/
	void setFromNickname(const QString& name);

//	TODO:
//	void setPlayerList(QIntDict<QString>);// use this for non-KGame use

	/**
	 * Adds a player nickname.
	 * @return The unique ID of the player
	 **/
	int addPlayer(const QString& nick);

	/**
	 * Removes all players with this nickname. Better don't use this as it
	 * will remove *all* players with this nickname. Save the id instead and
	 * call removePlayer(id)
	 * @param nick The nickname of the removed players
	 **/
	void removePlayer(const QString& nick);

	/**
	 * Removes the player with this id, as returned by @ref addPlayer
	 * @param id The id of the player to be removed
	 **/
	void removePlayer(int id);


	/**
	 * @return true if the messages which will be sent from here will be
	 * added automatically using @ref KChatBase::addMessage. See also @ref
	 * setAutoAddMessages
	 **/
	bool autoAddMessages() const;

	/**
	 * Usually the messages which will be sent from here (see @ref
	 * signalSendMessage) are added autmatically to this widget. But under
	 * some circumstances that would be very unhandy. So you can deactivate
	 * this behaviour here and call @ref KChatBase::addMessage yourself
	 * @param add If true (default) messages sent from here will be added
	 * automatically. Otherwise you will have to add them yourself
	 **/
	void setAutoAddMessages(bool add);

	/**
	 * @return The nickname of the player which belongs to this id
	 **/
	const QString& player(int id) const;

	/**
	 * @return The ID that belongs to the local player. See @setFromNickname
	 **/
	int fromId() const;
	

signals:
	/**
	 * This signal is emitted when the player wants to send a message.
	 *
	 * The message is added automatically using @ref KChatBase::addMessage if @ref
	 * autoAddMessages is enabled.
	 * @param id The id of the player who sends the message - see
	 * @ref setFromNickname and @ref player
	 * @param msg The message itself
	 **/
	void signalSendMessage(int id, const QString& msg);
	
protected:
	/**
	 * This emits @ref signalSendMessage and, if @ref autoAddMessages is
	 * true, calls @ref KChatBase::addMessage
	 **/
	virtual void returnPressed(const QString&);

	void updatePlayers();//TODO

	/**
	 * The Id of the next player. Incremented after every call.
	 **/
	int uniqueId();

private:
	void init();

	KChatPrivate* d;
};

#endif
