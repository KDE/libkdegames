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

#include <qbuffer.h>
#include <qmap.h>
#include <qintdict.h>

#include <klocale.h>
#include <kdebug.h>

#include "kgame.h"
#include "kplayer.h"
#include "kgameproperty.h"
#include "kgamemessage.h"

#include "kgamechat.h"

//FIXME:
#define FIRST_ID 2 // first id, that is free of use, aka not defined above

class KGameChatPrivate
{
public:
	KGameChatPrivate()
	{
		mFromPlayer = 0;
		mGame = 0;
		mToMyGroup = -1;
	}
	
	KGame* mGame;
	KPlayer* mFromPlayer;
	int mMessageId;


	QIntDict<KPlayer> mIndex2Player;

	QMap<int, int> mSendId2PlayerId;
	int mToMyGroup; // just as the above - but for the group, not for players
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

 setKGame(g);
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
 if (!d->mGame) {
	kdWarning(11001) << "KGameChat: You must set a game first!" << endl;
	return;
 }

 kdDebug(11001) << "from: " << d->mFromPlayer->id() << "==" << d->mFromPlayer->name() << endl;

 int id = sendingEntry();

 if (isToGroupMessage(id)) {
	// note: there is currently no support for other groups than the players
	// group! It might be useful to send to other groups, too
	QString group = d->mFromPlayer->group(); 
	kdDebug(11001) << "send to group " << group << endl;
	int sender = d->mFromPlayer->id();
	d->mGame->sendGroupMessage(text, messageId(), sender, group);

	//TODO
	//AB: this message is never received!! we need to connect to
	//KPlayer::networkData!!!
	//TODO
	
 } else {
	int toPlayer = 0;
	if (!isSendToAllMessage(id) && isToPlayerMessage(id)) {
		toPlayer = playerId(id);
		if (toPlayer == -1) {
			kdError(11001) << "KGameChat: don't know that player "
					<< "- internal ERROR" << endl;
		}
	} 
	int receiver = toPlayer;
	int sender = d->mFromPlayer->id();
	d->mGame->sendMessage(text, messageId(), receiver, sender);
 }
}

int KGameChat::messageId() const
{ return d->mMessageId; }

bool KGameChat::isSendToAllMessage(int id) const
{ return (id == KChatBase::SendToAll); }

bool KGameChat::isToGroupMessage(int id) const
{ return (id == d->mToMyGroup); }

bool KGameChat::isToPlayerMessage(int id) const
{
kdDebug(11001) << id << endl;
kdDebug(11001) << d->mSendId2PlayerId.contains(id) << endl;
return d->mSendId2PlayerId.contains(id); }

QString KGameChat::sendToPlayerEntry(const QString& name) const
{ return i18n("Send to %1").arg(name); }

int KGameChat::playerId(int id) const
{
 if (!isToPlayerMessage(id)) {
	return -1;
 }

 return d->mSendId2PlayerId[id];
}

int KGameChat::sendingId(int playerId) const
{
 QMap<int, int>::Iterator it;
 for (it = d->mSendId2PlayerId.begin(); it != d->mSendId2PlayerId.end(); ++it) {
	if (it.data() == playerId) {
		return it.key();
	}
 }
 return -1;
}

const QString& KGameChat::fromName() const
{ return d->mFromPlayer ? d->mFromPlayer->name() : QString::null; }

bool KGameChat::hasPlayer(int id) const
{
 return (sendingId(id) != -1);
}

void KGameChat::setFromPlayer(KPlayer* p)
{
 if (d->mFromPlayer) {
	changeSendingEntry(p->group(), d->mToMyGroup);
 } else {
	if (d->mToMyGroup != -1) {
		kdWarning(11001) << "send to my group exists already - removing" << endl;
		removeSendingEntry(d->mToMyGroup);
	}
	d->mToMyGroup = nextId();
	addSendingEntry(i18n("Send to my group (\"%1\")").arg(p->group()), d->mToMyGroup);
 }
 d->mFromPlayer = p;
}


void KGameChat::setKGame(KGame* g)
{
 if (d->mGame) {
	slotUnsetKGame();
 }
 d->mGame = g;

 if (g) {
	connect(d->mGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
			this, SLOT(slotAddPlayer(KPlayer*)));
	connect(d->mGame, SIGNAL(signalPlayerLeftGame(KPlayer*)), 
			this, SLOT(slotRemovePlayer(KPlayer*)));
	connect(d->mGame, SIGNAL(signalNetworkData(int, const QByteArray&, int, int)),
			this, SLOT(slotReceiveMessage(int, const QByteArray&, int, int)));
	connect(d->mGame, SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));

	QList<KPlayer> playerList = *d->mGame->playerList();
	for (int unsigned i = 0; i < playerList.count(); i++) {
		slotAddPlayer(playerList.at(i));
	}
 }
}

void KGameChat::slotUnsetKGame()
{
//TODO: test this method!

 if (!d->mGame) {
	return;
 }
 disconnect(d->mGame, 0, this, 0);
 removeSendingEntry(d->mToMyGroup);
 QMap<int, int>::Iterator it;
 for (it = d->mSendId2PlayerId.begin(); it != d->mSendId2PlayerId.end(); ++it) {
	removeSendingEntry(it.data());
 }
}

void KGameChat::slotAddPlayer(KPlayer* p)
{
 if (hasPlayer(p->id())) {
	kdWarning(11001) << "KGameChat: player was added before" << endl;
	return;
 }

 int sendingId = nextId();
 addSendingEntry(comboBoxItem(p->name()), sendingId);
 d->mSendId2PlayerId.insert(sendingId, p->id());
 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*))); 

 //TODO: remove the player when the he is removed from game!!!

}

void KGameChat::slotRemovePlayer(KPlayer* p)
{
 if (!hasPlayer(p->id())) {
	kdDebug(11001) << "KGameChat: cannot remove non-existent player" << endl;
	return;
 }

 int id = sendingId(p->id());
 removeSendingEntry(id);
 p->disconnect(this);
 d->mSendId2PlayerId.remove(id);
}

void KGameChat::slotPropertyChanged(KGamePropertyBase* prop, KPlayer* player)
{
 if (prop->id() == KGamePropertyBase::IdName) {
//	kdDebug(11001) << "new Name" << endl;
	changeSendingEntry(player->name(), sendingId(player->id()));
/*
	mCombo->changeItem(comboBoxItem(player->name()), index);
 */
 } else if (prop->id() == KGamePropertyBase::IdGroup) {
 //TODO
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

#include "kgamechat.moc"
