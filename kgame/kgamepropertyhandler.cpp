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
#include <klocale.h>
#include <typeinfo>

#define KPLAYERHANDLER_LOAD_COOKIE 6239

//---------------------- KGamePropertyHandler -----------------------------------
class KGamePropertyHandlerPrivate
{
public:
	KGamePropertyHandlerPrivate()
	{
	}
};

KGamePropertyHandler::KGamePropertyHandler(int id,const QObject * receiver, const char * sendf, const char *emitf) : QObject(0,0)
{
  init();
  registerHandler(id,receiver,sendf,emitf);
}
KGamePropertyHandler::KGamePropertyHandler() : QObject(0,0)
{
 init();
}

void KGamePropertyHandler::registerHandler(int id,const QObject * receiver, const char * sendf,const char *emitf)
{
  setId(id); 
  if (receiver && emitf)
  {
    kdDebug() << "Connecting SLOT " << emitf << endl;
    connect(this, SIGNAL(signalPropertyChanged(KGamePropertyBase *)), receiver, emitf);
  }
  if (receiver && sendf)
  {
    kdDebug() << "Connecting SLOT " << sendf << endl;
    connect(this, SIGNAL(signalSendMessage(QDataStream &)), receiver, sendf);
  }
}
void KGamePropertyHandler::init()
{
 kdDebug(11001) << "CREATE KGamePropertyHandler("<<this<<")"<<endl;
 mId = 0;
 d = new KGamePropertyHandlerPrivate; // for future use - is BC important to us?
}

KGamePropertyHandler::~KGamePropertyHandler()
{
  clear();
  delete d;
}

bool KGamePropertyHandler::processMessage(QDataStream &stream, int id)
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
   p = mIdDict.find(propertyId);
   p->command(stream,cmd);
   return true;
 }
 p = mIdDict.find(propertyId);
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


bool KGamePropertyHandler::removeProperty(KGamePropertyBase* data)
{
 if (!data) {
	return false;
 }
 mNameMap.remove(data->id());
 return mIdDict.remove(data->id());
}
bool KGamePropertyHandler::addProperty(KGamePropertyBase* data,QString name)
{
  //kdDebug(11001) << "KGamePropertyHandler::addproperty("<<data<<endl;
 if (mIdDict.find(data->id())) {
	// this id already exists
	kdError(11001) << "  -> cannot add property " << data->id() << endl;
	return false;
 }
 else
 {
	mIdDict.insert(data->id(), data);
  // if here is a check for "is_debug" or so we can add the strings only in debug mode
  // and save memory!!
  if (!name.isNull())
  {
    mNameMap.insert(data->id(),name);
    kdDebug() << "KGamePropertyHandler::addProperty: nid="<< (data->id()) << " inserted in Map name=" << mNameMap[data->id()] <<endl;
    kdDebug() << "Typeid=" << typeid(data).name() << endl;
    kdDebug() << "Typeid call=" << data->typeinfo()->name() << endl;
  }
 }
 return true;
}

QString KGamePropertyHandler::propertyName(int id)
{
  QString s;
  if (mIdDict.find(id))
  {
    if (mNameMap.contains(id)) s=mNameMap[id];
    else s.setNum(id);
  }
  else
  {
    // Should _never_ happen
    s=i18n("%1 unregistered").arg(id);
  }
  return s;
}

bool KGamePropertyHandler::load(QDataStream &stream)
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

bool KGamePropertyHandler::save(QDataStream &stream)
{
 kdDebug(11001) << "KGamePropertyHandler::save " << mIdDict.count() << " KGameProperty objects " << endl;
 stream << (uint)mIdDict.count();
 QIntDictIterator<KGamePropertyBase> it(mIdDict);
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

void KGamePropertyHandler::unlockProperties()
{
 QIntDictIterator<KGamePropertyBase> it(mIdDict);
 while (it.current()) {
	it.current()->setReadOnly(false);
	++it;
 }
}

void KGamePropertyHandler::lockProperties()
{
 QIntDictIterator<KGamePropertyBase> it(mIdDict);
 while (it.current()) {
	it.current()->setReadOnly(true);
	++it;
 }
}

void KGamePropertyHandler::emitSignal(KGamePropertyBase *prop)
{
  emit signalPropertyChanged(prop);
}

void KGamePropertyHandler::sendProperty(QDataStream &s)
{
  emit signalSendMessage(s);
}

KGamePropertyBase *KGamePropertyHandler::find(int id)
{
  return mIdDict.find(id);
}

void KGamePropertyHandler::clear()
{
 mIdDict.clear();
 mNameMap.clear();
}

