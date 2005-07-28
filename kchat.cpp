/*
    This file is part of the KDE games library
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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

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
QString KChat::fromName() const
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
