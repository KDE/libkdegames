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
#include <qbuffer.h>

#include <klocale.h>
#include <kdebug.h>

#include "kgame.h"
#include "kplayer.h"
#include "kplayerdata.h"

#include "kgamechat.h"

//FIXME:
#define SEND_TO_GROUP_ID 1
#define FIRST_ID 2 // first id, that is free of use, aka not defined above

class KGameChatPrivate
{
public:
	KGameChatPrivate()
	{
		mFromPlayer = 0;
		mGame = 0;
	}
	
	KGame* mGame;
	QIntDict<KPlayer> mIndex2Player;
	KPlayer* mFromPlayer;
	int mMessageId;
};

KGameChat::KGameChat(KGame* g, int msgid, QWidget* parent) : KChatBase(parent)
{
 init(g, msgid); 
}

KGameChat::KGameChat(KGame* g, int msgid, KPlayer* fromPlayer, QWidget* parent) : KChatBase(parent)
{
 init(g, msgid);
 setFromPlayer(fromPlayer);
}

KGameChat::~KGameChat()
{
 kdDebug(11001) << "DESTRUCT KGameChat " << this << endl;
 delete d;
}

void KGameChat::init(KGame* g, int msgId)
{
 kdDebug(11001) << "INIT KGameChat " << this << endl;
 d = new KGameChatPrivate;
 d->mMessageId = msgId;

 setGame(g);
}

void KGameChat::addMessage(int fromId, const QString& text)
{
 if (!d->mGame) {
	kdWarning(11001) << "no KGame object has been set" << endl;
	addMessage(i18n("Player %1").arg(fromId), text);
 } else {
	KPlayer* p = d->mGame->findPlayer(fromId);
	if (p) {
		kdDebug(11001) << "adding message of player " << p->name() << "id=" << fromId << endl;
		addMessage(p->name(), text);
	} else {
		kdWarning(11001) << "Could not find player id " << fromId << endl;
		addMessage(i18n("Unknown"), text);
	}
 }
}

void KGameChat::returnPressed(const QString& text)
{
 if (!d->mFromPlayer) {
	kdWarning(11001) << "KGameChat: You must set a player first!" << endl;
	return;
 }

 int fromPlayer = (d->mFromPlayer ? d->mFromPlayer->id() : -1);
 kdDebug(11001) << "from: " << fromPlayer << endl;
 kdDebug(11001) << "from: " << d->mFromPlayer->name() << endl;

 //TODO:
 int index = 0;//mCombo->currentItem();
 int toPlayer = 0;
 QString toGroup;
 if (d->mFromPlayer && index == 1) {
	toGroup = d->mFromPlayer->group();
 } else if (index >= FIRST_ID) {
//	toPlayer = mPlayers.at(index - FIRST_ID)->id();
 }

 sendMessage(fromPlayer, text, toPlayer, toGroup);
}

const QString& KGameChat::fromName() const
{ return d->mFromPlayer ? d->mFromPlayer->name() : QString::null; }

void KGameChat::setFromPlayer(KPlayer* p)
{
//TODO
 if (d->mFromPlayer) {
//	mCombo->changeItem(p->group(), SEND_TO_GROUP_ID);
 } else {
//	mCombo->insertItem(i18n("Send to my group (\"%1\")").arg(p->group()), SEND_TO_GROUP_ID);
 }
 d->mFromPlayer = p;
}

void KGameChat::updatePlayers()
{
kdError(11001) << "KGameChat::updatePlayers() is not working and shouldn't be called" << endl;
 QIntDictIterator<KPlayer> it(d->mIndex2Player);
 while (it.current()) {
	slotRemovePlayer(it.current());
	++it;
 }
 d->mIndex2Player.clear();
 
//FIXME
// Problem: one KGame can contain several KPlayers :-(
// But: do we want *every* of those players (including computer players) in the
// combo box? 
// If not: Check which player is created/resides on which KGame (via
// KPlayer::game()) and add only one of every KGame Object. Problem: which one
// (if more than one) ? 
//FIXME(end)
 KGame::KGamePlayerList l = *d->mGame->playerList();
 for (KPlayer* p = l.first(); p; p = l.next()) {
	slotAddPlayer(p);
 }

}

void KGameChat::setGame(KGame* g)
{
 if (d->mGame) {
	disconnect(g, 0, this, 0);
 }
 d->mGame = g;
 connect(d->mGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
		this, SLOT(slotAddPlayer(KPlayer*)));
 connect(d->mGame, SIGNAL(signalPlayerLeftGame(KPlayer*)), 
		this, SLOT(slotRemovePlayer(KPlayer*)));
 connect(d->mGame, SIGNAL(signalNetworkData(int, const QByteArray&, int, int)),
		this, SLOT(slotReceiveMessage(int, const QByteArray&, int, int)));

 QList<KPlayer> playerList = *d->mGame->playerList();
 for (int i = 0; i < playerList.count(); i++) {
	slotAddPlayer(playerList.at(i));
 }
}

void KGameChat::slotAddPlayer(KPlayer* p)
{ // no one will prevent you from adding one player twice!! check before calling this!
 p->disconnect(this);
 addSendingEntry(comboBoxItem(p->name()), SEND_TO_GROUP_ID);
// mIndex2Player.insert(mCombo->count() - 1, p);
 connect(p, SIGNAL(signalPropertyChanged(KPlayerDataBase*, KPlayer*)),
		this, SLOT(slotPropertyChanged(KPlayerDataBase*, KPlayer*))); 
 // where to put the id ?? TODO
}

void KGameChat::slotRemovePlayer(KPlayer* p)
{
 int index = -1;
 QIntDictIterator<KPlayer> it(d->mIndex2Player);
 while (it.current() && index == -1) {
	if (it.current() == p) {
		index = it.currentKey();
	}
	++it;
 }
 if (index < 0) {
	return;
 }

 this->disconnect(p);// uff - or is it p->disconnect(this);?
/*
 removeSendingEntry(findId(index));//FIXME
 mCombo->removeItem(index);
 mIndex2Player.remove(index);
 it.toFirst();
 while (it.current()) {
	if (it.currentKey() > index) {
		mIndex2Player.insert(it.currentKey() - 1, mIndex2Player[it.currentKey()]);
		mIndex2Player.remove(it.currentKey());
	}
	++it;
 }*/
}

void KGameChat::slotPropertyChanged(KPlayerDataBase* prop, KPlayer* player)
{
 if (prop->id() == KPlayerDataBase::IdName) {
	kdDebug(11001) << "new Name" << endl;
	int index = -1;
	QIntDictIterator<KPlayer> it(d->mIndex2Player);
	while (it.current() && index == -1) {
		if (it.current() == player) {
			index = it.currentKey();
		}
		++it;
	}
	if (index < 0) {
		return;
	}
	
/*
	mCombo->changeItem(comboBoxItem(player->name()), index);
 */
 } else if (prop->id() == KPlayerDataBase::IdGroup) {
 //TODO
 }
}

void KGameChat::sendMessage(int fromPlayer, const QString& text, int toPlayer, const QString& toGroup)
{
//TODO: toPlayer
 if (toPlayer > 0) {
	kdDebug(11001) << "must be implemented" << endl;
 } //else if (toGroup.length() > 0) {}

 if (toGroup.length() > 0) {
 //TODO
	d->mGame->sendGroupMessage(text, messageId(), fromPlayer, toGroup);
 } else {
	d->mGame->sendMessage(text, messageId(), 0, fromPlayer);
 }
}

void KGameChat::slotReceiveMessage(int msgid, const QByteArray& buffer, int receiver, int sender)
{
 QDataStream msg(buffer, IO_ReadOnly);
 if (msgid != messageId()) {
	return;
 }

// TODO: the data from KGameMessages::createHeader() remain in the buffer!!!!!!!
// how to duplicate it now? we could just call extractHeader() but that is ugly

// QByteArray buffer;
// buffer.duplicate(((QBuffer*)msg.device())->buffer());
// QDataStream stream(buffer, IO_ReadOnly);
 QString text;
// stream >> text;
 msg >> text;

 addMessage(sender, text);
}

int KGameChat::messageId() const
{ return d->mMessageId; }

#include "kgamechat.moc"
