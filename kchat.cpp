/* **************************************************************************
                             KChat Class
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
#include <qintdict.h>//used?
#include <qmap.h>

#include <klocale.h>
#include <kdebug.h>

#include "kchat.h"

class KChatPrivate
{
public:
	KChatPrivate()
	{
	}

	bool mAutoAddMessages;

	QMap<int, QString> mPlayerMap;
	int mPlayerId;
	int mFromId;
};

KChat::KChat(QWidget* parent, bool twoPlayerGame) : KChatBase(parent, twoPlayerGame)
{
 init(); 
}

KChat::~KChat()
{
 kdDebug(11000) << "DESTRUCT KChat " << this << endl;
 delete d;
}

void KChat::init()
{
 kdDebug(11001) << "INIT KChat " << this << endl;
 d = new KChatPrivate;
 d->mAutoAddMessages = true;
 d->mPlayerId = 1;
 d->mFromId = 1;
}

void KChat::setFromNickname(const QString& n)
{ d->mFromId = addPlayer(n); }
const QString& KChat::fromName() const
{ return player(fromId()); }
void KChat::setAutoAddMessages(bool add) 
{ d->mAutoAddMessages = add; }
bool KChat::autoAddMessages() const 
{ return d->mAutoAddMessages; }
int KChat::uniqueId()
{ return d->mPlayerId++; }
int KChat::fromId() const
{ return d->mFromId; }
const QString& KChat::player(int id) const
{ return d->mPlayerMap[id]; }

void KChat::returnPressed(const QString& text)
{
 int id = fromId();
 if (id < 0) {
	// don't return - just display "unknown" as name
	kdWarning(11000) << "KChat: no fromNickname has been set!" << endl;
 }
 emit signalSendMessage(id, text);
 if (autoAddMessages()) {
	QString p = player(id);
	if (p.isNull()) {
		p = i18n("Unknown");
	}
	kdDebug(11000) << "auto adding message from player " << p << " ;id=" << id << endl;
	addMessage(p, text);
 }
}

int KChat::addPlayer(const QString& nickname)
{
 int id = uniqueId();
 d->mPlayerMap.insert(id, nickname);
 return id;
}

void KChat::removePlayer(int id)
{
 d->mPlayerMap.remove(id);
}

void KChat::removePlayer(const QString& nickname)
{
 QMap<int, QString>::Iterator it;
 for (it = d->mPlayerMap.begin(); it != d->mPlayerMap.end(); ++it) {
	if (it.data() == nickname) {
		d->mPlayerMap.remove(it);
	}
 }
}


#include "kchat.moc"
