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

#ifndef __KGAMECHAT_H__
#define __KGAMECHAT_H__

#include <qstring.h>

#include "kchatbase.h"

class KPlayer;
class KGame;
class KGamePropertyBase;

class KGameChatPrivate;

/**
 * This is a chat widget which can easily added to a KGame based game.
 *
 * Call @ref setFromPlayer() first - this will be used as the "from" part of
 * every message you will send. Otherwise it won't work! You can also use the
 * fromPlayer parameter in the constructor though...
 * @short A Chat widget for KGame-based games
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameChat : public KChatBase
{
	Q_OBJECT
public:
	/**
	 *
	 **/
	KGameChat(KGame* g, int msgid, KPlayer* fromPlayer, QWidget * parent);
	KGameChat(KGame* g, int msgId, QWidget* parent);

	~KGameChat();

	/**
	 * TODO: doku
	 * @param player The player of this widget
	 **/
	void setFromPlayer(KPlayer* player);

	void setGame(KGame* g);

	/**
	 * @return The name that will be shown for messages from this widget. Either the
	 * string that was set by @ref setFromName or the name of the player
	 * that was set by @ref setFromPlayer
	 **/
	virtual const QString& fromName() const;

	/**
	 * @return The id of the messages produced by KGameChat. The id will be
	 * used in @ref KGame as parameter msgid in the method @ref KGame::sendMessage
	 **/
	int messageId() const;


public slots:
	virtual void addMessage(const QString& fromName, const QString& message) { KChatBase::addMessage(fromName, message);}
	virtual void addMessage(int fromId, const QString& message);

	void slotReceiveMessage(int, const QByteArray&, int, int sender);

protected:
	/**
	 * The user entered a message which has to be sent. Replace this for
	 * non-KGame use (not yet working)
	 *
	 * @param text The message to be sent
	 * @param toPlayer The id of tha player this message is for (see @ref
	 * addPlayer()) or -1 for broadcast
	 * @param toGroup The group this message is for or QString::null for
	 * broadcast
	 * @param fromPlayer the id of the local player - see @ref
	 * setFromPlayer. -1 if none has been set
	 **/
	virtual void sendMessage(int fromPlayer, const QString& text, int toPlayer, const QString& toGroup);

protected slots:
	void slotPropertyChanged(KGamePropertyBase*, KPlayer*);
	void slotAddPlayer(KPlayer*);
	void slotRemovePlayer(KPlayer*);

protected:
	virtual void returnPressed(const QString&);
	/**
	 * obsolete
	 **/
	void updatePlayers();

private:
	void init(KGame* g, int msgid);

private:
	KGameChatPrivate* d;
};

#endif
