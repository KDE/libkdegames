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
/*
    $Id$
*/

#include "kgamepropertyhandler.h"
#include "kgameproperty.h"
#include "kgamemessage.h"
#include "kplayer.h"
#include "kgame.h"

#define KPLAYERHANDLER_LOAD_COOKIE 6239

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
 KGamePropertyBase* p;
 int propertyId;
 KGameMessage::extractPropertyHeader(stream,propertyId);
 //kdDebug(11001) << "KGamePropertyHandler::networkTransmission: Got property " << propertyId << endl;
 if (propertyId==KGamePropertyBase::IdCommand)
 {
   int cmd;
   KGameMessage::extractPropertyCommand(stream,propertyId,cmd);
   kdDebug() << "KGamePropertyHandlerBase::processMessage: Got COMMAND for id= "<<propertyId <<endl;
   p = find(propertyId);
   p->command(stream,cmd);
   return true;
 }
 p = find(propertyId);
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
 for (i = 0; i < count; i++) {
	processMessage(stream, id());
 }
 Q_INT16 cookie;
 stream >> cookie;
 if (cookie == KPLAYERHANDLER_LOAD_COOKIE) {
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
		KGameMessage::createPropertyHeader(stream, base->id());
		base->save(stream);
	}
	++it;
 }
 stream << (Q_INT16)KPLAYERHANDLER_LOAD_COOKIE;
 return true;
}

