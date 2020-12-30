/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgameproperty.h"

// own
#include "kgamepropertyhandler.h"
#include "kgamemessage.h"
#include "kplayer.h"
#include "kgame.h"

#define KPLAYERHANDLER_LOAD_COOKIE 6239

KGamePropertyBase::KGamePropertyBase(int id, KGame* parent)
{
 init();
 registerData(id, parent);
}

KGamePropertyBase::KGamePropertyBase(int id, KPlayer* parent)
{
 init();
 registerData(id, parent);
}

KGamePropertyBase::KGamePropertyBase(int id, KGamePropertyHandler* owner)
{
 init();
 registerData(id, owner);
}

KGamePropertyBase::KGamePropertyBase()
{
 init();
}

KGamePropertyBase::~KGamePropertyBase()
{
 unregisterData();
}

void KGamePropertyBase::init()
{
 mOwner = nullptr;
 setDirty(false);

 // this is very useful and used by e.g. KGameDialog so
 // it is activated by default. Big games may profit by deactivating it to get
 // a better performance. 
 setEmittingSignal(true);

 setOptimized(false); 
 
 //setReadOnly(false);
 mFlags.bits.locked = false ; // setLocked(false); is NOT possible as it checks whether isLocked() allows to change the status

 // local is default
 setPolicy(PolicyLocal);
}

int KGamePropertyBase::registerData(int id, KGame* owner, const QString& name)
{ return registerData(id, owner->dataHandler(), name);  }

int KGamePropertyBase::registerData(int id, KPlayer* owner, const QString& name)
{ return registerData(id, owner->dataHandler(), name);  }

int KGamePropertyBase::registerData( KGamePropertyHandler* owner,PropertyPolicy p, const QString& name)
{ return registerData(-1, owner,p, name);  }

int KGamePropertyBase::registerData(int id, KGamePropertyHandler* owner, const QString& name)
{ return registerData(id, owner,PolicyUndefined, name);  }

int KGamePropertyBase::registerData(int id, KGamePropertyHandler* owner,PropertyPolicy p, const QString& name)
{
// we don't support changing the id
 if (!owner) {
	qCWarning(GAMES_PRIVATE_KGAME) << "Resetting owner=0. Sure you want to do this?";
	mOwner=nullptr;
	return -1;
 }
 if (!mOwner) {
	if (id==-1) {
		id=owner->uniquePropertyId();
	}
	mId = id;
	mOwner = owner;
	mOwner->addProperty(this, name);
	if (p!=PolicyUndefined) {
		setPolicy(p);
	} else {
		setPolicy(mOwner->policy());
	}
 }
 return mId;
}

void KGamePropertyBase::unregisterData()
{
 if (!mOwner) {
	return;
 }
 mOwner->removeProperty(this);
 mOwner = nullptr;
}

bool KGamePropertyBase::sendProperty()
{
 QByteArray b;
 QDataStream s(&b, QIODevice::WriteOnly);
 KGameMessage::createPropertyHeader(s, id());
 save(s);
 if (mOwner) {
	return mOwner->sendProperty(s);
 } else {
	qCCritical(GAMES_PRIVATE_KGAME) << "Cannot send because there is no receiver defined";
	return false;
 }
}

bool KGamePropertyBase::sendProperty(const QByteArray& data)
{
 QByteArray b;
 QDataStream s(&b, QIODevice::WriteOnly);
 KGameMessage::createPropertyHeader(s, id());
 s.writeRawData(data.data(), data.size());
 if (mOwner) {
	return mOwner->sendProperty(s);
 } else {
	qCCritical(GAMES_PRIVATE_KGAME) << ": Cannot send because there is no receiver defined";
	return false;
 }
}

bool KGamePropertyBase::lock()
{
 if (isLocked()) {
	return false;
 }
 setLock(true);
 return true;
}

bool KGamePropertyBase::unlock(bool force)
{
 if (isLocked() && !force) {
	return false;
 }
 setLock(false);
 return true;
}

void KGamePropertyBase::setLock(bool l)
{
 QByteArray b;
 QDataStream s(&b, QIODevice::WriteOnly);
 KGameMessage::createPropertyCommand(s, IdCommand, id(), CmdLock);
 
 s << (qint8)l;
 if (mOwner) {
	mOwner->sendProperty(s);
 } else {
	qCCritical(GAMES_PRIVATE_KGAME) << ": Cannot send because there is no receiver defined";
	return ;
 }
}

void KGamePropertyBase::emitSignal()
{
 //qCDebug(GAMES_PRIVATE_KGAME) << ": mOwnerP="<< mOwner << "id=" << id();
 if (mOwner ) {
	mOwner->emitSignal(this);
 } else {
	qCCritical(GAMES_PRIVATE_KGAME) << ":id="<<id()<<" Cannot emitSignal because there is no handler set";
 }
}

void KGamePropertyBase::command(QDataStream& s, int cmd, bool isSender)
{
 switch (cmd) {
	case CmdLock:
	{
		if (!isSender) {
			qint8 locked;
			s >> locked;
			mFlags.bits.locked = (bool)locked ;
			break;
		}
	}
	default: // probably in derived classes
		break;
 }
}

