/* **************************************************************************
                           KPlayerData Class
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

#include "kplayerdata.h"
#include "kgamemessage.h"
#include "kplayer.h"
#include "kgame.h"

#define KPLAYERHANDLER_LOAD_COOKIE 6239

KPlayerDataBase::KPlayerDataBase(int id, KPlayerDataHandler* owner)
{
 init();
 registerData(id, owner);
}

KPlayerDataBase::KPlayerDataBase()
{
 init();
}

KPlayerDataBase::~KPlayerDataBase()
{
}

void KPlayerDataBase::init()
{

 mOwner=0;
 setPublic(true); // visible to all players by default#

 // this is very useful and used by e.g. KGameDialog so
 // it is activated by default. Big games may profit by deactivating it to get
 // a better performance. 
 setEmittingSignal(true); 
 
 setLocked(false);
}

void KPlayerDataBase::registerData(int id, KPlayerDataHandler* owner)
{
// we don't support changing the id
 if (!mOwner) {
	mId = id;
	mOwner = owner;
	mOwner->addProperty(this);
 }
}

void KPlayerDataBase::send()
{
 QByteArray b;
 QDataStream s(b, IO_WriteOnly);
 KGameMessage::createPropertyHeader(s,id());
 save(s);
 if (mOwner) {
	mOwner->sendProperty(s, isPublic());
 } else {
   kdError(11001) << "KPlayerDataBase::send(): Cannot send because there is no receiver defined" << endl;
 }

}
void KPlayerDataBase::emitSignal()
{
  //kdDebug(11001) << "KPlayerDataBase::emitSignal() mOwnerP="<< mOwner << " id=" << id()   << endl;
  if (mOwner ) {
     mOwner->emitSignal(this);
  } else {
    kdError(11001) << "KPlayerDataBase::emitSignal(): Cannot emitSignal because there is no receiver defined" << endl;
  }
}


//---------------------- KPlayerDataHandler -----------------------------------
KPlayerDataHandler::KPlayerDataHandler() : QIntDict<KPlayerDataBase>()
{
  kdDebug(11001) << "CREATE KPlayerDataHandler("<<this<<")"<<endl;
  mType=0;
  mOwner=0;
  mId=0;
}

KPlayerDataHandler::KPlayerDataHandler(int id, KPlayer *parent) 
{
  KPlayerDataHandler();
  registerHandler(id,parent);
}
KPlayerDataHandler::KPlayerDataHandler(int id, KGame *parent) 
{
  KPlayerDataHandler();
  registerHandler(id,parent);
}

KPlayerDataHandler::KPlayerDataHandler(int id, void *parent,int type) 
{
  KPlayerDataHandler();
  registerHandler(id,parent,type);
}

void KPlayerDataHandler::registerHandler(int id, KPlayer *parent)
{
  registerHandler(id,(void *)parent,1);
}
void KPlayerDataHandler::registerHandler(int id, KGame *parent)
{
  registerHandler(id,(void *)parent,2);
}
void KPlayerDataHandler::registerHandler(int id, void *parent,int type)
{
  mType=type;
  mOwner=(void *)parent;
  mId=id;
  //kdDebug(11001) << "KPlayerDataHandler::registerHandler("<<id<<","<<parent<<","<<type<<")"<<endl;
}
KPlayerDataHandler::~KPlayerDataHandler()
{
  clear();
}

bool KPlayerDataHandler::processMessage(QDataStream &stream, int id)
{
  //kdDebug(11001) << "KPlayerDataHandler::processMessage: id=" << id << " mId=" << mId << endl;
  if (id!=mId) return false; // Is the message meant for us?
  int propertyId;
  KGameMessage::extractPropertyHeader(stream,propertyId);
  //kdDebug(11001) << "KPlayerDataHandler::networkTransmission: Got property " << propertyId << endl;
  KPlayerDataBase* p = find(propertyId);
  if (p) {
//    kdDebug(11001) << "KPlayerDataHandler::processMessage: Loading " << propertyId << endl;
    p->load(stream);
//    kdDebug(11001) << "done" << endl;
  } else {
    if (!p) {
      kdError(11001) << "KPlayerDataHandler::processMessage:property " << propertyId << " not found" << endl;
    }
  }
  return true;
}


bool KPlayerDataHandler::removeProperty(KPlayerDataBase* data)
{
  if (!data) return false;
  return remove(data->id());
}
bool KPlayerDataHandler::addProperty(KPlayerDataBase* data)
{
  //kdDebug(11001) << "KPlayerDataHandler::addproperty("<<data<<endl;
 if (find(data->id())) {
	// this id already exists
	kdError(11001) << "  -> cannot add property " << data->id() << endl;
	return false;
 } else {
	insert(data->id(), data);
 }
 return true;
}

void KPlayerDataHandler::sendProperty(QDataStream& s, bool isPublic)
{
//AB: IDEA: maybe use KGameClient::sendProperty or so? Seems to be the most
//logical way...
  if (!owner()) return ;
  switch(type())
  {
    case 1:
    {
      KPlayer *p=(KPlayer *)owner();
      //kdDebug(11001) << "KPlayerDataHandler::sendProperty to player "<<p<<endl;
      p->sendProperty(s,isPublic);
    }
    break;
    case 2:
    {
      KGame *g=(KGame *)owner();
      //kdDebug(11001) << "KPlayerDataHandler::sendProperty to game "<<g<<endl;
      g->sendProperty(s,isPublic);
    }
    break;
    default:
    kdError(11001) << "KPlayerDataHandler::sendProperty:: Need to overwrite virtual function" << endl;
    break;
  }
}

void KPlayerDataHandler::emitSignal(KPlayerDataBase *data)
{
    //kdDebug(11001) << "emitting signal from DataHandler to type="<< type() << endl;

  if (!owner()) return ;
  switch(type())
  {
    case 1:
    {
      KPlayer *p=(KPlayer *)owner();
      p->emitSignal(data);
    }
    break;
    case 2:
    {
      KGame *g=(KGame *)owner();
      g->emitSignal(data);
    }
    break;
    default:
    kdError(11001) << "KPlayerDataHandler::emitSignal:: Need to overwrite virtual function" << endl;
    break;
  }
}


bool KPlayerDataHandler::load(QDataStream &stream)
{
  uint count,i;
  stream >> count;
  kdDebug(11001) << "KPlayerDataHandler::load " << count << " KPlayerData objects " << endl;
  for (i=0;i<count;i++)
  {
    processMessage(stream,id());
  }
  Q_INT16 cookie;
  stream >> cookie;
  if (cookie==KPLAYERHANDLER_LOAD_COOKIE)
  {
      //kdDebug(11001) << "   PlayerDataHandler loaded propertly"<<endl;
  }
  else
  {
      kdError(11001) << "   PlayerDataHandler loading error. probably format error"<<endl;
  }
  return true;
}
bool KPlayerDataHandler::save(QDataStream &stream)
{
  kdDebug(11001) << "KPlayerDataHandler::save " << count() << " KPlayerData objects " << endl;
  stream << (uint)count();
  QIntDictIterator<KPlayerDataBase> it(*this);
  while (it.current())
  {
    KPlayerDataBase *base=it.current();
    if (base) 
    {
      KGameMessage::createPropertyHeader(stream,base->id());
      base->save(stream);
    }
    ++it;
  }
  stream << (Q_INT16)KPLAYERHANDLER_LOAD_COOKIE;
  return true;
}

