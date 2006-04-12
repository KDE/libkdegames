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
/*
    $Id$
*/

#include "kgameproperty.h"
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
 mOwner = 0;
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

int KGamePropertyBase::registerData(int id, KGame* owner, QString name)
{ return registerData(id, owner->dataHandler(), name);  }

int KGamePropertyBase::registerData(int id, KPlayer* owner, QString name)
{ return registerData(id, owner->dataHandler(), name);  }

int KGamePropertyBase::registerData( KGamePropertyHandler* owner,PropertyPolicy p, QString name)
{ return registerData(-1, owner,p, name);  }

int KGamePropertyBase::registerData(int id, KGamePropertyHandler* owner, QString name)
{ return registerData(id, owner,PolicyUndefined, name);  }

int KGamePropertyBase::registerData(int id, KGamePropertyHandler* owner,PropertyPolicy p, QString name)
{
// we don't support changing the id
 if (!owner) {
	kWarning(11001) << k_funcinfo << "Resetting owner=0. Sure you want to do this?" << endl;
	mOwner=0;
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
 mOwner = 0;
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
	kError(11001) << k_funcinfo << "Cannot send because there is no receiver defined" << endl;
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
	kError(11001) << k_funcinfo << ": Cannot send because there is no receiver defined" << endl;
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
	kError(11001) << k_funcinfo << ": Cannot send because there is no receiver defined" << endl;
	return ;
 }
}

void KGamePropertyBase::emitSignal()
{
 //kDebug(11001) << k_funcinfo << ": mOwnerP="<< mOwner << " id=" << id()   << endl;
 if (mOwner ) {
	mOwner->emitSignal(this);
 } else {
	kError(11001) << k_funcinfo << ":id="<<id()<<" Cannot emitSignal because there is no handler set" << endl;
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

