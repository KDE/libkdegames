/* **************************************************************************
                           KPlayer Class
                           -------------------
    begin                : 1 January 2001
    copyright            : (C) 2001 by Martin Heni and Andreas Beckermann
    email                : martin@heni-online.de and b_mann@gmx.de
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

#include <stdio.h>

#include <kdebug.h>
#include <klocale.h>
#include <assert.h>

#include "kgame.h"
#include "kgameio.h"
#include "kplayer.h"
#include "kgamemessage.h"

#define KPLAYER_LOAD_COOKIE 7285

class KPlayerPrivate
{
public:
   KPlayerPrivate()
   {
      mNetworkPlayer = 0;
   }

   int mId;
   bool mVirtual; // virtual player
   int mPriority; // tag for replacement

   KPlayer* mNetworkPlayer; // replacement player

   KGamePropertyHandler<KPlayer> mProperties;

// Playerdata
   KGamePropertyQString mName;
   KGamePropertyQString mGroup;
};

KPlayer::KPlayer() : QObject(0,0)
{
   kdDebug(11001) << "CREATE(KPlayer=" << this <<") sizeof(this)="<<sizeof(KPlayer) << endl;
   kdDebug(11001) << "sizeof(m_Group)="<<sizeof(d->mGroup)<<endl;
   d = new KPlayerPrivate;

   d->mProperties.registerHandler(KGameMessage::IdPlayerProperty,this);
   d->mVirtual=false;
   mActive=true;
   mGame=0;
   d->mId=0;
   d->mPriority=0;
   // I guess we cannot translate the group otherwise no
   // international conenctions are possible

   d->mGroup.registerData(KGamePropertyBase::IdGroup, dataHandler());
   d->mGroup = i18n("default");
   d->mName.registerData(KGamePropertyBase::IdName, dataHandler());
   d->mName = i18n("default");

   mAsyncInput.registerData(KGamePropertyBase::IdAsyncInput, dataHandler());
   mAsyncInput.setValue(false);
   mMyTurn.registerData(KGamePropertyBase::IdTurn, dataHandler());
   mMyTurn.setValue(false);
   mUserId.registerData(KGamePropertyBase::IdUserId, dataHandler());
   mUserId.setValue(0);



}

KPlayer::~KPlayer()
{
  kdDebug(11001) << "DESTRUCT(KPlayer=" << this <<") id=" << this->id() << endl;

  // Delete IODevices
  KGameIO *input;
  while((input=mInputList.first()))
  {
//    kdDebug(11001) << "KPlayer: delete input" << endl;
    /*
        if (game()) {
          game()->unregisterListener(input);
    }
    */
    delete input;
  }
  if (mGame) mGame->removePlayer(this);

// note: mProperties does not use autoDelete or so - user must delete objects
// himself
  d->mProperties.clear();
  delete d;
//  kdDebug(11001) << "DESTRUCT(KPlayer=" << this <<") done" << endl;
}

bool KPlayer::forwardMessage(QDataStream &msg,int msgid,int receiver,int sender)
{
  if (!isActive()) {
    return false;
  }
  if (!game()) {
    return false;
  }
  kdDebug(11001) << "KPlayer::forwardMessage to game sender="<<sender<<"" << "recv="<<receiver <<"msgid="<<msgid << endl;
  return game()->sendSystemMessage(msg,msgid,receiver,sender);
}

bool KPlayer::forwardInput(QDataStream &msg,bool transmit,int sender)
{
  if (!isActive()) return false;
  if (!game()) return false;

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
  if (transmit) {
    return game()->sendPlayerInput(msg,this,sender);
  } else {
    return game()->playerInput(msg,this,sender);
  }
}

void KPlayer::setId(int newid)
{
  // Remove it...MH...unused
#ifdef THIS_CAN_BE_REMOVED
//AB: this is unclean. Should be in KGameProcessIO

  // If we have a process input device we need to tellit our id change....
  KGameProcessIO *input=(KGameProcessIO *)findRttiIO(KGameIO::ProcessIO);
  // If the process is not running we need not send as the init of
  // the process transmits the id...this is ONLY if the id of the player
  // changes after the process is setup
  if (input && newid!=d->mId  && input->isRunning()) 
  {
    int cookie=4242;
    if (game()) cookie=game()->cookie();
    QByteArray a;
    QDataStream stream(a,IO_WriteOnly);
    stream << newid;
    input->sendProcess(stream,KGameMessage::IdPlayerId,0,KGameMessage::calcMessageId(0,id()),cookie);
  }
#endif
  // Needs to be after the sendProcess
  d->mId=newid;
}


void KPlayer::setGroup(const QString& group)
{ d->mGroup.setValue(group); }

const QString& KPlayer::group() const
{ return d->mGroup.value(); }

void KPlayer::setName(const QString& name)
{ d->mName.setValue(name); }

const QString& KPlayer::name() const
{ return d->mName.value(); }

int KPlayer::id() const
{ return d->mId; }

KGamePropertyHandlerBase* KPlayer::dataHandler()
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
  if (!input) return false;
  mInputList.append(input); 
  input->setPlayer(this);
  return true;
}

// input=0, remove all
bool KPlayer::removeGameIO(KGameIO *targetinput)
{
  bool result=true;
  if (!targetinput) // delete all
  {
    KGameIO *input;
    while((input=mInputList.first()))
    {
      mInputList.remove(input);
    }
  }
  else
  {
    result=mInputList.remove(targetinput);
  }
  return result;
}


KGameIO * KPlayer::findRttiIO(int rtti) const
{
  QListIterator<KGameIO> it(mInputList);
  while (it.current()) 
  {
    if (it.current()->rtti() == rtti) return it.current();
    ++it;
  }
  return 0;
} 

int KPlayer::calcIOValue()
{
  int value=0;
  QListIterator<KGameIO> it(mInputList);
  while (it.current()) 
  {
    value|=it.current()->rtti();
    ++it;
  }
  return value;
} 

bool KPlayer::setTurn(bool b,bool exclusive)
{
  if (!isActive()) return false;
  // forward to all mirror players...ehm why????? TODO: MH 28022001
  // if (transmit) game()->sendSystemMessage(b,KGameMessage::IdTurn,KGameMessage::calcMessageId(0,id()));
   
  // if we get to do an exclusive turn all other players are disallowed
  if (exclusive && b && game())
  {
     KPlayer *player;
     KGame::KGamePlayerList *list=game()->playerList();
     for ( player=list->first(); player != 0; player=list->next() )
     {
       if (player==this) continue;
       player->setTurn(false,false);
     }
  }

  // Return if nothing changed
  if (b==mMyTurn.value()) return true;

  mMyTurn.setValue(b);

  // notify KGameIO's from the turn (computer player)
  //AB: this is probably a bug. The IOs should be notified when the property
  //mMyTurn is *received*!
  QListIterator<KGameIO> it(mInputList);
  while (it.current()) 
  {
    it.current()->notifyTurn(b);
    ++it;
  }

  return true;
}

bool KPlayer:: load(QDataStream &stream)
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

  emit signalLoad(stream);
  return true;
}
bool KPlayer::save(QDataStream &stream)
{
  stream << (Q_INT32)id() << (Q_INT32)networkPriority();
  
  d->mProperties.save(stream);

  stream << (Q_INT16)KPLAYER_LOAD_COOKIE;

  emit signalSave(stream);
  return true;
}


void KPlayer::networkTransmission(QDataStream &stream,int msgid,int sender)
{
  kdDebug(11001) << "KPlayer::ReceiveNetworkTransmission: msgid=" << msgid << " sender=" << sender << " we are=" << id() << endl;
  // PlayerProperties processed
  if (d->mProperties.processMessage(stream,msgid)) {
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
    /*
    case KGameMessage::IdTurn:
      {
        kdDebug(11001) << "KPlayer::networkTransmission: Got remote player turn" << endl;
        int turn;
        stream >> turn;
        setTurn((bool)turn,false,false);
      }
    break;
    */
    default:
          emit signalNetworkData((int)msgid,stream,(int)sender,this);
          kdDebug(11001) << "KPlayer::ReceiveNetworkTransmision: User data msgid " << msgid << endl;
    break;
  }

}

KGamePropertyBase* KPlayer::findProperty(int id) const
{
  return d->mProperties[id];
}

bool KPlayer::addProperty(KGamePropertyBase* data)
{
  return d->mProperties.addProperty(data);
}

void KPlayer::sendProperty(QDataStream& s)
{
  if (!game()) return ;
  game()->sendPlayerProperty(s, id());
}

void KPlayer::emitSignal(KGamePropertyBase *me)
{
  emit signalPropertyChanged(me,this);
}

void KPlayer::unlockProperties()
{
 QIntDictIterator<KGamePropertyBase> it(d->mProperties);
 while (it.current()) {
	it.current()->setLocked(false);
	++it;
 }
}

void KPlayer::lockProperties()
{
 QIntDictIterator<KGamePropertyBase> it(d->mProperties);
 while (it.current()) {
	it.current()->setLocked(true);
	++it;
 }
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
