/* **************************************************************************
                           KGameProperty Class
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
/*
    $Id$
*/

#include "kgameproperty.h"
#include "kgamemessage.h"
#include "kplayer.h"
#include "kgame.h"

#define KPLAYERHANDLER_LOAD_COOKIE 6239

KGamePropertyBase::KGamePropertyBase(int id, KGamePropertyHandlerBase* owner)
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
}

void KGamePropertyBase::init()
{

 mOwner=0;

 // this is very useful and used by e.g. KGameDialog so
 // it is activated by default. Big games may profit by deactivating it to get
 // a better performance. 
 setEmittingSignal(true); 
 setOptimized(false); 
 setReadOnly(false);
}

void KGamePropertyBase::registerData(int id, KGamePropertyHandlerBase* owner)
{
// we don't support changing the id
 if (!mOwner) {
	mId = id;
	mOwner = owner;
	mOwner->addProperty(this);
 }
}

void KGamePropertyBase::send()
{
 QByteArray b;
 QDataStream s(b, IO_WriteOnly);
 KGameMessage::createPropertyHeader(s,id());
 save(s);
 if (mOwner) {
	mOwner->sendProperty(s);
 } else {
   kdError(11001) << "KGamePropertyBase::send(): Cannot send because there is no receiver defined" << endl;
 }

}
void KGamePropertyBase::emitSignal()
{
 //kdDebug(11001) << "KGamePropertyBase::emitSignal() mOwnerP="<< mOwner << " id=" << id()   << endl;
 if (mOwner ) {
	mOwner->emitSignal(this);
 } else {
	kdError(11001) << "KGamePropertyBase::emitSignal(): Cannot emitSignal because there is no receiver defined" << endl;
 }
}


//---------------------- KGamePropertyHandler -----------------------------------
class KGamePropertyHandlerBasePrivate
{
public:
	KGamePropertyHandlerBasePrivate()
	{
	}
};

KGamePropertyHandlerBase::KGamePropertyHandlerBase() : QIntDict<KGamePropertyBase>()
{
 init();
}

void KGamePropertyHandlerBase::init()
{
 kdDebug(11001) << "CREATE KGamePropertyHandlerBase("<<this<<")"<<endl;
 mId = 0;
 d = new KGamePropertyHandlerBasePrivate; // for future use - is BC important to us?
}

KGamePropertyHandlerBase::~KGamePropertyHandlerBase()
{
 clear();
 delete d;
}

bool KGamePropertyHandlerBase::processMessage(QDataStream &stream, int id)
{
 //kdDebug(11001) << "KGamePropertyHandler::processMessage: id=" << id << " mId=" << mId << endl;
 if (id != mId) {
	return false; // Is the message meant for us?
 }
 int propertyId;
 KGameMessage::extractPropertyHeader(stream,propertyId);
 //kdDebug(11001) << "KGamePropertyHandler::networkTransmission: Got property " << propertyId << endl;
 KGamePropertyBase* p = find(propertyId);
 if (p) {
	//kdDebug(11001) << "KGamePropertyHandler::processMessage: Loading " << propertyId << endl;
	p->load(stream);
	//kdDebug(11001) << "done" << endl;
 } else {
	if (!p) {
		kdError(11001) << "KGamePropertyHandler::processMessage:property " << propertyId << " not found" << endl;
	}
 }
 return true;
}


bool KGamePropertyHandlerBase::removeProperty(KGamePropertyBase* data)
{
 if (!data) {
	return false;
 }
 return remove(data->id());
}
bool KGamePropertyHandlerBase::addProperty(KGamePropertyBase* data)
{
  //kdDebug(11001) << "KGamePropertyHandler::addproperty("<<data<<endl;
 if (find(data->id())) {
	// this id already exists
	kdError(11001) << "  -> cannot add property " << data->id() << endl;
	return false;
 } else {
	insert(data->id(), data);
 }
 return true;
}

bool KGamePropertyHandlerBase::load(QDataStream &stream)
{
 uint count,i;
 stream >> count;
 kdDebug(11001) << "KGamePropertyHandler::load " << count << " KGameProperty objects " << endl;
 for (i=0;i<count;i++) {
	processMessage(stream,id());
 }
 Q_INT16 cookie;
 stream >> cookie;
 if (cookie==KPLAYERHANDLER_LOAD_COOKIE) {
	//kdDebug(11001) << "   KGamePropertyHandler loaded propertly"<<endl;
 } else {
	kdError(11001) << "KGamePropertyHandler loading error. probably format error"<<endl;
 }
 return true;
}

bool KGamePropertyHandlerBase::save(QDataStream &stream)
{
 kdDebug(11001) << "KGamePropertyHandler::save " << count() << " KGameProperty objects " << endl;
 stream << (uint)count();
 QIntDictIterator<KGamePropertyBase> it(*this);
 while (it.current()) {
	KGamePropertyBase *base=it.current();
	if (base) {
		KGameMessage::createPropertyHeader(stream,base->id());
		base->save(stream);
	}
	++it;
 }
 stream << (Q_INT16)KPLAYERHANDLER_LOAD_COOKIE;
 return true;
}

