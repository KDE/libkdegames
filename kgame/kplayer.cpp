/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/*
    $Id$
*/


#include "kgame.h"
#include "kgameio.h"
#include "kplayer.h"
#include "kgamemessage.h"
#include "kgamepropertyhandler.h"

#include <kdebug.h>
#include <klocale.h>

#include <qbuffer.h>

#include <stdio.h>
#include <assert.h>

#define KPLAYER_LOAD_COOKIE 7285

class KPlayerPrivate
{
public:
   KPlayerPrivate()
   {
      mNetworkPlayer = 0;
   }

   Q_UINT32 mId;
   bool mVirtual; // virtual player
   int mPriority; // tag for replacement

   KPlayer* mNetworkPlayer; // replacement player

   KGamePropertyHandler mProperties;

// Playerdata
   KGamePropertyQString mName;
   KGamePropertyQString mGroup;
};

KPlayer::KPlayer() : QObject(0,0)
{
 init();
}

KPlayer::KPlayer(KGame* game) : QObject(0, 0)
{
 init();
 game->addPlayer(this);
}

void KPlayer::init()
{
// note that NO KGame object exists here! so we cannot use KGameProperty::send!
   kdDebug(11001) << "CREATE(KPlayer=" << this <<") sizeof(this)="<<sizeof(KPlayer) << endl;
   kdDebug(11001) << "sizeof(m_Group)="<<sizeof(d->mGroup)<<endl;
   d = new KPlayerPrivate;

   d->mProperties.registerHandler(KGameMessage::IdPlayerProperty,
                                  this,SLOT(sendProperty(int, QDataStream&, bool*)),
                                       SLOT(emitSignal(KGamePropertyBase *)));
   d->mVirtual=false;
   mActive=true;
   mGame=0;
   d->mId=0; // "0" is always an invalid ID!
   d->mPriority=0;
   // I guess we cannot translate the group otherwise no
   // international conenctions are possible

   mUserId.registerData(KGamePropertyBase::IdUserId, this, i18n("UserId"));
   mUserId.setLocal(0);
   d->mGroup.registerData(KGamePropertyBase::IdGroup, this, i18n("Group"));
   d->mGroup.setLocal(i18n("default"));
   d->mName.registerData(KGamePropertyBase::IdName, this, i18n("Name"));
   d->mName.setLocal(i18n("default"));

   mAsyncInput.registerData(KGamePropertyBase::IdAsyncInput, this, i18n("AsyncInput"));
   mAsyncInput.setLocal(false);
   mMyTurn.registerData(KGamePropertyBase::IdTurn, this, i18n("myTurn"));
   mMyTurn.setLocal(false);
   mMyTurn.setEmittingSignal(true);
   mMyTurn.setOptimized(false);
}

KPlayer::~KPlayer()
{
  kdDebug(11001) << "DESTRUCT(KPlayer=" << this <<") id=" << this->id() << endl;

  // Delete IODevices
  KGameIO *input;
  while((input=mInputList.first()))
  {
    delete input;
  }
  if (game()) 
  {
    game()->playerDeleted(this);
  }

// note: mProperties does not use autoDelete or so - user must delete objects
// himself
  d->mProperties.clear();
  delete d;
//  kdDebug(11001) << "DESTRUCT(KPlayer=" << this <<") done" << endl;
}

bool KPlayer::forwardMessage(QDataStream &msg,int msgid,Q_UINT32 receiver,Q_UINT32 sender)
{
  if (!isActive()) 
  {
    return false;
  }
  if (!game()) 
  {
    return false;
  }
  kdDebug(11001) << "KPlayer::forwardMessage to game sender="<<sender<<"" << "recv="<<receiver <<"msgid="<<msgid << endl;
  return game()->sendSystemMessage(msg,msgid,receiver,sender);
}

bool KPlayer::forwardInput(QDataStream &msg,bool transmit,Q_UINT32 sender)
{
  if (!isActive()) 
  {
    return false;
  }
  if (!game()) 
  {
    return false;
  }

  kdDebug(11001) << "KPlayer::forwardInput to game playerInput(sender="<<sender<<")" << endl;
  if (!asyncInput() && !myTurn())
  {
    kdDebug(11001) << "KPlayer::forwardInput rejected cause it is not our turn" << endl;
    return false;
  }

  // AB: I hope I remember the usage correctly:
  // this function is called twice (on sender side) - once with transmit = true
  // where it sends the input to the comserver and once with transmit = false
  // where it really looks at the input
  if (transmit)
  {
    kdDebug(11001) << "indirect playerInput" << endl;
    return game()->sendPlayerInput(msg,this,sender);
  } 
  else
  {
    kdDebug(11001) << "direct playerInput" << endl;
    return game()->systemPlayerInput(msg,this,sender);
  }
}

void KPlayer::setId(Q_UINT32 newid)
{
  // Needs to be after the sendProcess
  d->mId=newid;
}


void KPlayer::setGroup(const QString& group)
{ d->mGroup = group; }

const QString& KPlayer::group() const
{ return d->mGroup.value(); }

void KPlayer::setName(const QString& name)
{ d->mName = name; }

const QString& KPlayer::name() const
{ return d->mName.value(); }

Q_UINT32 KPlayer::id() const
{ return d->mId; }

KGamePropertyHandler * KPlayer::dataHandler()
{ return &d->mProperties; }

void KPlayer::setVirtual(bool v)
{ d->mVirtual = v; }

bool KPlayer::isVirtual() const
{ return d->mVirtual;}

void KPlayer::setNetworkPlayer(KPlayer* p)
{ d->mNetworkPlayer = p; }

KPlayer* KPlayer::networkPlayer() const
{ return d->mNetworkPlayer; }

int KPlayer::networkPriority() const
{ return d->mPriority; }

void KPlayer::setNetworkPriority(int p)
{ d->mPriority = p; }

bool KPlayer::addGameIO(KGameIO *input)
{
  if (!input) 
  {
    return false;
  }
  mInputList.append(input); 
  input->initIO(this); // set player and init device
  return true;
}

// input=0, remove all
bool KPlayer::removeGameIO(KGameIO *targetinput,bool deleteit)
{
  kdDebug(11001) << "KPlayer::removeGameIO()" << targetinput << " delete=" << deleteit<< endl;
  bool result=true;
  if (!targetinput) // delete all
  {
    KGameIO *input;
    while((input=mInputList.first()))
    {
      if (input) removeGameIO(input,deleteit);
    }
  }
  else
  {
//    kdDebug(11001) << "remove IO " << targetinput << endl;
    if (deleteit) 
    {
      delete targetinput;
    }
    else
    {
      targetinput->setPlayer(0);
      result=mInputList.remove(targetinput);
    }
  }
  return result;
}


KGameIO * KPlayer::findRttiIO(int rtti) const
{
  QPtrListIterator<KGameIO> it(mInputList);
  while (it.current()) 
  {
    if (it.current()->rtti() == rtti) 
    {
      return it.current();
    }
    ++it;
  }
  return 0;
} 

int KPlayer::calcIOValue()
{
  int value=0;
  QPtrListIterator<KGameIO> it(mInputList);
  while (it.current()) 
  {
    value|=it.current()->rtti();
    ++it;
  }
  return value;
} 

bool KPlayer::setTurn(bool b, bool exclusive)
{
  kdDebug(11001) << "KPlayer::setTurn " << id() << " (" << this << ") to " << b << endl;
  if (!isActive()) 
  {
    return false;
  }
   
  // if we get to do an exclusive turn all other players are disallowed
  if (exclusive && b && game())
  {
     KPlayer *player;
     KGame::KGamePlayerList *list=game()->playerList();
     for ( player=list->first(); player != 0; player=list->next() )
     {
       if (player==this) 
       {
         continue;
       }
       player->setTurn(false,false);
     }
  }

  // Return if nothing changed
  mMyTurn = b;

  return true;
}

bool KPlayer::load(QDataStream &stream)
{
  Q_INT32 id,priority;
  stream >> id >> priority;
  setId(id);
  setNetworkPriority(priority);

  // Load Player Data
  //FIXME: maybe set all properties setEmitSignal(false) before?
  d->mProperties.load(stream);

  Q_INT16 cookie;
  stream >> cookie;
  if (cookie==KPLAYER_LOAD_COOKIE)
  {
      kdDebug(11001) << "   Player loaded propertly"<<endl;
  }
  else
  {
      kdError(11001) << "   Player loading error. probably format error"<<endl;
  }

  // emit signalLoad(stream);
  return true;
}
bool KPlayer::save(QDataStream &stream)
{
  stream << (Q_INT32)id() << (Q_INT32)networkPriority();
  
  d->mProperties.save(stream);

  stream << (Q_INT16)KPLAYER_LOAD_COOKIE;

  //emit signalSave(stream);
  return true;
}


void KPlayer::networkTransmission(QDataStream &stream,int msgid,Q_UINT32 sender)
{
  //kdDebug(11001) << "KPlayer::ReceiveNetworkTransmission: msgid=" << msgid << " sender=" << sender << " we are=" << id() << endl;
  // PlayerProperties processed
  bool issender;
  if (game()) 
  {
    issender=sender==game()->gameId();
  }
  else 
  {
    issender=true;
  }
  if (d->mProperties.processMessage(stream,msgid,issender)) 
  {
	return ;
  }
  switch(msgid)
  {
    case KGameMessage::IdPlayerInput:
      {
        kdDebug(11001) << "KPlayer::networkTransmission: Got player move KPlayer (virtual) forwards it to the game object" << endl;
        forwardInput(stream,false);
      }
    break;
    default:
    // FIXME ab: is this correct
          emit signalNetworkData(msgid,((QBuffer*)stream.device())->readAll(),sender,this);
    // or this?  -> KGame::networkTransmission uses msgid - IdUser but it wasn't
    // used here...
//         emit signalNetworkData(msgid - KGameMessage::IdUser,((QBuffer*)stream.device())->readAll(),sender,this);
          kdDebug(11001) << "KPlayer::ReceiveNetworkTransmision: User data msgid " << msgid << endl;
    break;
  }

}

KGamePropertyBase* KPlayer::findProperty(int id) const
{
  return d->mProperties.find(id);
}

bool KPlayer::addProperty(KGamePropertyBase* data)
{
  return d->mProperties.addProperty(data);
}

void KPlayer::sendProperty(int msgid, QDataStream& stream, bool* sent)
{
  if (game())
  {
    bool s = game()->sendPlayerProperty(msgid, stream, id());
    if (s)
    {
      *sent = true;
    }
  }
}

void KPlayer::emitSignal(KGamePropertyBase *me)
{
  // Notify KGameIO (Process) for a new turn
  if (me->id()==KGamePropertyBase::IdTurn)
  {
    //kdDebug(11001) << "KPlayer::emitSignal for KGamePropertyBase::IdTurn " << endl;
    QPtrListIterator<KGameIO> it(mInputList);
    while (it.current()) 
    {
      it.current()->notifyTurn(mMyTurn.value());
      ++it;
    }
  }
  emit signalPropertyChanged(me,this);
}

// --------------------- DEBUG --------------------
void KPlayer::Debug()
{
   kdDebug(11001) << "------------------- KPLAYER -----------------------" << endl;
   kdDebug(11001) << "this:    " << this << endl;
   kdDebug(11001) << "rtti:    " << rtti() << endl;
   kdDebug(11001) << "id  :    " << id() << endl;
   kdDebug(11001) << "Name :   " << name() << endl;
   kdDebug(11001) << "Group:   " << group() << endl;
   kdDebug(11001) << "Async:   " << asyncInput() << endl;
   kdDebug(11001) << "myTurn:  " << myTurn() << endl;
   kdDebug(11001) << "Virtual: " << isVirtual() << endl;
   kdDebug(11001) << "Active:  " << isActive() << endl;
   kdDebug(11001) << "Priority:" << networkPriority() << endl;
   kdDebug(11001) << "Game   : " << game() << endl;
   kdDebug(11001) << "#IOs:    " << mInputList.count() << endl;
   kdDebug(11001) << "---------------------------------------------------" << endl;
}

#include "kplayer.moc"
