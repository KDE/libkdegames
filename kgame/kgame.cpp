/* **************************************************************************
                           KGame class
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

#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <kdebug.h>
#include <klocale.h>

#include <qbuffer.h>
#include <qtimer.h>
#include <qqueue.h>

#include "kplayerdata.h"
#include "kplayer.h"
#include "kgame.h"
#include "kgameio.h"

#include "kgamemessage.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define KGAME_LOAD_COOKIE 4210

// try to place as much as possible here
// many things are *not* possible here as KGame has to use some inline function
class KGamePrivate
{
public:
    KGamePrivate()
    {
        mUniquePlayerNumber = 0;
        mCurrentPlayer = 0 ;
    }

    int mUniquePlayerNumber;
    KPlayer *mCurrentPlayer;
    QQueue<KPlayer> mAddPlayerList;// this is a list of to-be-added players. See addPlayer() docu


    KPlayerDataHandler mProperties;

    //KPlayerDatas
    KPlayerDataInt mMaxPlayer;
    KPlayerDataUInt mMinPlayer;
    KPlayerDataInt mGameStatus; // Game running?
};

// ------------------- GAME CLASS --------------------------
KGame::KGame(int cookie,QObject* parent) : KGameNetwork(cookie,parent)
{
  kdDebug(11001) << "CREATE(KGame=" << this <<") sizeof(this)="<<sizeof(KGame) << endl;
  d = new KGamePrivate;
  
  d->mProperties.registerHandler(KGameMessage::IdGameProperty,this);
  d->mMaxPlayer.registerData(KPlayerDataBase::IdMaxPlayer, dataHandler());
  d->mMaxPlayer.setValue(-1);  // Infinite
  d->mMinPlayer.registerData(KPlayerDataBase::IdMinPlayer, dataHandler());
  d->mMinPlayer.setValue(0);   // Always ok     
  d->mGameStatus.registerData(KPlayerDataBase::IdGameStatus, dataHandler());
  d->mGameStatus.setValue(End);
  d->mUniquePlayerNumber=0;
  mRandom.setSeed(0);

  // BL: FIXME This signal does no longer exist. When we are merging
  // KGame and KGameNetwork, this could be improved!
//  connect(this,SIGNAL(signalConnectionLost(KGameClient *)),
//          this,SLOT(slotConnectionLost(KGameClient *)));
}

KGame::~KGame()
{
  kdDebug(11001) << "DESTRUCT(KGame=" << this <<")" << endl;
  Debug();
  reset();
  delete d;
  kdDebug(11001) << "DESTRUCT(KGame=" << this <<") done" << endl;
}

bool KGame::reset()
{
  deletePlayers();
  deleteInactivePlayers();
  return true;
}

void KGame::deletePlayers()
{
  kdDebug(11001) << "KGame::deletePlayers()" << endl;
  KPlayer *player;
  while((player=mPlayerList.first()))
  {
    // AB: we cannot call KGame::removePlayer here, as we don't want any network
    // transmit in destructing. We just remove the players from the list and
    // delete them.
    mPlayerList.remove(player);
    delete player; // delete removes the player
  }
  kdDebug(11001) << "KGame::deletePlayers() done" << endl;
}

void KGame::deleteInactivePlayers()
{
  KPlayer *player;
  while((player=mInactivePlayerList.first()))
  {
    //player->setGame(0); // prevent call backs
    mInactivePlayerList.remove(player);
    delete player;
  }
}

bool KGame::load(QDataStream &stream)
{
  return loadgame(stream, false);
}

bool KGame::loadgame(QDataStream &stream, bool network)
{
  // Load Game Data
  uint i;
  d->mProperties.load(stream);

  // Load Playerobjects
  uint playercount;
  stream >> playercount;
  kdDebug(11001) << "Loading KGame " << playercount << " KPlayer objects " << endl;
  for (i=0;i<playercount;i++)
  {
    int rtti,id,iovalue;
    stream >> rtti >> id >> iovalue;
    KPlayer *newplayer=findPlayer(id);
    if (!newplayer)
    {
      kdDebug(11001) << "   Player "<< id << "not found...asking user to create one " << endl;
      emit signalCreatePlayer(newplayer,rtti,iovalue,network,this);
    }
    if (!newplayer)
    {
      kdDebug(11001) << "   No user defined player created. Creating default KPlayer " << endl;
      newplayer=new KPlayer;
    }
    else 
    {
      kdDebug(11001) << "   USER Player " << newplayer << " done player->rtti=" << newplayer->rtti() << " rtti=" << rtti << endl;
    }
    newplayer->load(stream);
    if (network) newplayer->setVirtual(true);
    // AB: FIXME: are the players loaded properly? -> should be fixed (AB)
    systemAddPlayer(newplayer);
  }

  Q_INT16 cookie;
  stream >> cookie;
  if (cookie==KGAME_LOAD_COOKIE)
  {
      kdDebug(11001) << "   Game loaded propertly"<<endl;
  }
  else
  {
      kdError(11001) << "   Game loading error. probably format error"<<endl;
  }

  emit signalLoad(stream);
  return true;
}

bool KGame::save(QDataStream &stream,bool saveplayers)
{
  // Save Game Data
  d->mProperties.save(stream);

  if (saveplayers)
  {
    kdDebug(11001) << "Saving KGame " << playerCount() << " KPlayer objects " << endl;
    stream << playerCount();
    KPlayer *player;
    for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() )
    {
       stream << (int)player->rtti();
       stream << (int)player->id();
       stream << (int)player->calcIOValue();
       player->save(stream);
    }
  }
  else
  {
    stream << (uint)0; // no players saved
  }

  stream << (Q_INT16)KGAME_LOAD_COOKIE;


  emit signalSave(stream);
  return true;
}



// ----------------- Player handling -----------------------

KPlayer * KGame::findPlayer(int id) const
{
  for (QListIterator<KPlayer> it(mPlayerList); it.current(); ++it) 
  {
    if (it.current()->id() == id) return it.current();
  }
  for (QListIterator<KPlayer> it(mInactivePlayerList); it.current(); ++it)
  {
    if (it.current()->id() == id) return it.current();
  }
//  KPlayer *player;
//  for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() )
//  {
//    if (player->id()==id) return player;
//  }
//  for ( player=mInactivePlayerList.first(); player != 0; player=mInactivePlayerList.next() )
//  {
//    if (player->id()==id) return player;
//  }
  return 0;
}

// it is necessary that addPlayer and systemAddPlayer are called in the same
// order. Ie if addPlayer(foo) followed by addPlayer(bar) is called, you must
// not call systemAddPlayer(bar) followed by systemAddPlayer(foo), as the
// mAddPlayerList would get confused. Should be no problem as long as comServer
// and the clients are working correctly.
// BUT: if addPlayer(foo) does not arrive by any reason while addPlayer(bar)
// does, we would be in trouble...
void KGame::addPlayer(KPlayer* newplayer, int receiver)
{//transmit to all clients, or to receiver only
  kdDebug(11001) << "Trying to send add player " << "; maxPlayers=" << maxPlayers() << " playerCount=" << playerCount() << endl;
  
  if (maxPlayers() >= 0 && (int)playerCount() >= maxPlayers()) {
    kdWarning(11001) << "cannot add more than " << maxPlayers() << " players" << endl;
    return;
  }

  QByteArray buffer;
  QDataStream stream(buffer,IO_WriteOnly);
  writeToStream(newplayer, receiver, stream);
  d->mAddPlayerList.enqueue(newplayer);
  sendSystemMessage(stream,(int)KGameMessage::IdAddPlayer, receiver, KGameMessage::calcMessageId(gameId(),0));
}

void KGame::writeToStream(KPlayer* p, int owner, QDataStream& stream)
{
// this could be in KGameMessage as well
  stream << (int)p->rtti();
  stream << (int)(owner ? owner : gameId());// the owner of the player -> used for setVirtual
  stream << (int)p->calcIOValue();
  p->save(stream);
}

bool KGame::removePlayer(KPlayer * player, int receiver)
{//transmit to all clients, or to receiver only
 if (!player) {
   return false;
 }
 kdDebug(11001) << "KGame::removePlayer: sending Player (" << player->id() << ") removed " << player << endl;

 // Send also for virtual player?
 bool send = true;
 send = sendSystemMessage(KGameMessage::calcMessageId(0,player->id()),KGameMessage::IdRemovePlayer, receiver);
 if (!send) {// if we couldn't send remove the player directly
   systemRemovePlayer(player);
   return true;
 }
 return send;
 // we will receive the message in networkTransmission()
}

void KGame::systemRemovePlayer(KPlayer* player)
{
 if (player) {
	emit signalPlayerLeftGame(player);
 } else {
	kdWarning(11001) << "cannot remove NULL player" << endl;
	return;
 }
 if (!systemRemove(player)) {
	kdWarning(11001) << "player " << player << "(" << player->id() << ") Could not be found!" << endl;
 }
 delete player;

#warning FIXME: transmit is obsolete!!
//AB: the below part is a TODO
/*
 // Can we add an inactiavte player as replacement?
 if ((maxPlayers() < 0 || (int)playerCount() < maxPlayers()) && mInactivePlayerList.count() > 0) {
   // If not transmit we wait for the other client to activate a player
   bool transmit = true;// oops - obsolete :-( can it be used this way?
   if (transmit) {
     KPlayer *p;
     KPlayer *activateP=0;
     int maxpri=-1;
     // Find the player with the highest priority to reactivate
     for ( p=mInactivePlayerList.first(); p!= 0; p=mInactivePlayerList.next() ) {
       if (p->networkPriority()>maxpri) {
         maxpri=p->networkPriority();
         activateP=p;
       }
     }
     if (activateP) {
       activatePlayer(activateP);
     }
   }
 }
  */

 if (gameStatus()==(int)Run && playerCount()<minPlayers()) {
   kdWarning(11001) << "KGame::removePlayer: not enough players, pausing game\n" << endl;
   setGameStatus(Pause);
 }
// return result;
}

bool KGame::systemRemove(KPlayer* p)
{
 if (!p) {
   kdWarning(11001) << "cannot remove NULL player" << endl;
   return false;
 }
 bool result;
 kdDebug(11001) << "KGame::systemRemove: Player (" << p->id() << ") to be removed " << p << endl;

 if (mPlayerList.count() == 0) {
	result = false;
 } else {
	result = mPlayerList.remove(p);
 }

 if (p->isVirtual()) {
 //FIXME:
/*   QPtrDictIterator<KGameClient> it(clientMap());
   for( ; it.current(); ++it ) {
     KGameClient *client=it.current();
     client->playerList()->remove(p);
   }*/
 }

 // otherwise the .clear() does not work
 p->setGame(0);

 return result;
}

bool KGame::inactivatePlayer(KPlayer* player)
{
 if (!player) {
   return false;
 }
 kdDebug(11001) << "send to Inactivate player " << player->id() << endl;

 sendSystemMessage(KGameMessage::calcMessageId(0,player->id()), KGameMessage::IdInactivatePlayer);
 return true;
}

bool KGame::systemInactivatePlayer(KPlayer* player)
{
 if (!player || !player->isActive()) {
   return false;
 }
 kdDebug(11001) << " Inactivate player " << player->id() << endl;

 // Virtual players cannot be deactivated. They will be removed
 if (player->isVirtual()) {
   systemRemovePlayer(player);
 } else {
   mPlayerList.remove(player);
   mInactivePlayerList.append(player);
   player->setActive(false);
 } 
 emit signalPlayerLeftGame(player);
 return true;
}

bool KGame::activatePlayer(KPlayer * player)
{
 if (!player) {
   return false;
 }
 kdDebug(11001) << "KGame::activatePlayer sending (" << player->id() << ")" << endl;
 
 sendSystemMessage(KGameMessage::calcMessageId(0, player->id()), KGameMessage::IdActivatePlayer);
 return true;
}

bool KGame::systemActivatePlayer(KPlayer* player)
{
 if (!player || player->isActive()) {
   return false;
 }
 kdDebug(11001) << "KGame::systemActivatePlayer(" << player->id() << ")" << endl;

 mInactivePlayerList.remove(player);
 player->setActive(true);
 systemAddPlayer(player);
 return true;
}

// -------------------- Properties ---------------------------
void KGame::setGameId(int id)
{
  kdError (11001) << "KGame::setGameID: Is broken at the moment!" << endl;
  // FIXME: This is not possible at the moment!

/*
  KGameNetwork::setGameId(id);
  // Adjust all player id after the gameid got changed...which better happens ONLY by
  // a remote call by the gamemaster!
  KPlayer *player;
  for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() )
  {
    if (player->isVirtual()) continue;
    player->setId(KGameMessage::calcMessageId(gameId(),0)|(player->id()&0x3ff));
  }
  for ( player=mInactivePlayerList.first(); player != 0; player=mInactivePlayerList.next() )
  {
    if (player->isVirtual()) continue;
    player->setId(KGameMessage::calcMessageId(gameId(),0)|(player->id()&0x3ff));
  }
*/
}

void KGame::setMaxPlayers(uint maxnumber)
{ d->mMaxPlayer.setValue(maxnumber); }

void KGame::setMinPlayers(uint minnumber)
{ d->mMinPlayer.setValue(minnumber); }

uint KGame::minPlayers() const
{ return d->mMinPlayer.value(); }

int KGame::maxPlayers() const
{ return d->mMaxPlayer.value(); }

uint KGame::playerCount() const
{ return mPlayerList.count(); }

int KGame::gameStatus() const
{ return d->mGameStatus.value(); }

bool KGame::isRunning() const
{ return d->mGameStatus.value() == Run; }

KPlayerDataHandler* KGame::dataHandler()
{ return &d->mProperties; }


bool KGame::sendPlayerInput(QDataStream &msg, KPlayer *player, int sender)
{
  kdDebug(11001) << "KGame::Got playerInput ..." << endl;
  if (!player) return false;
  if (!isRunning()) return false;

  kdDebug(11001) << " ... transmitting over network" << endl;
  sendSystemMessage(msg, (int)KGameMessage::IdPlayerInput, KGameMessage::calcMessageId(0,player->id()), sender);
  return true;
}

bool KGame::playerInput(QDataStream &msg, KPlayer *player, int sender)
{
  kdDebug(11001) << "KGame::Got playerInput from comServer... sender: " << sender << endl;
  if (!player) return false;
  if (!isRunning()) return false;

  kdDebug(11001) << " ... emmitting signal" << endl;
  emit signalPlayerInput(msg,player); 
  
  d->mCurrentPlayer=player;
  // Check for game over and if not allow the next player to move
  int over=gameOver(player);
  if (over!=0) emit signalGameOver(over,player,this);
  else if (!player->asyncInput()) QTimer::singleShot(0,this,SLOT(prepareNext()));

  return true;
}

// Per default we do not do anything
int KGame::gameOver(KPlayer *)
{
  return 0;
}

void KGame::prepareNext()
{
  nextPlayer(d->mCurrentPlayer);
}

bool KGame::nextPlayer(KPlayer *last,bool exclusive)
{
   int lastId,minId,nextId;
   KPlayer *nextplayer,*minplayer;
   if (last) lastId=last->id();
   else lastId=-1;
   minId=0xffff;
   nextId=minId;
   nextplayer=0;
   minplayer=0;

  KPlayer *player;
  for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() )
  {
    // Find the first player for a cycle
    if (player->id()<minId)
    {
      minId=player->id();
      minplayer=player;
    }
    if (player==last) continue;
    // Find the next player which is bigger than the current one
    if (player->id()>lastId && player->id()<nextId)
    {
      nextId=player->id();
      nextplayer=player;
    }
  }

  // Cycle to the beginning
  if (!nextplayer)
  {
    nextplayer=minplayer;
  }

  kdDebug(11001) << "KGame::nextPlayer ##### lastId=" << lastId << " excluseive=" << exclusive << "  minId=" << minId << " bextid=" << nextId << " count=" <<mPlayerList.count()  << endl;
  if (nextplayer) nextplayer->setTurn(true,exclusive);
  else return false;
  return true;
}

void KGame::setGameStatus(int status)
{
  kdDebug(11001) << "KGame::gameStatus(" << status << ")" << endl;
  if (status==(int)Run && playerCount()<minPlayers())
  {
    kdDebug(11001) << "KGame::gameStatus: not enough players, pausing game\n" << endl;
    status=Pause;
  }
  d->mGameStatus.setValue(status);
}


void KGame::networkTransmission(QDataStream &stream,int msgid,int receiver,int sender, Q_UINT32 clientID)
{
 kdDebug(11001) << "KGame::networkTransmission:: msgid=" << msgid << " sender=" << sender << " receiver=" << receiver << endl;
 // message targets a playerobject. If we find it we forward the message to the
 // player. Otherwise we proceed here and hope the best that the user processes
 // the message
 if (KGameMessage::calcPlayerId(receiver)) {
	KPlayer *p=findPlayer(receiver);
	if (p && p->isActive()) {
		kdDebug(11001) << "forwarding message to player " << receiver << endl;
		p->networkTransmission(stream,msgid,sender);
		emit signalMessageUpdate(msgid,receiver,sender); // hmmmm...but how else do we notice that somehting happended?
		return;
	}
 }
 // If it is not for a player it is meant for us!!!! Otherwise the
 // gamenetwork would not have passed the message to us!

 // GameProperties processed
 if (d->mProperties.processMessage(stream,msgid)) {
// 	kdDebug(11001 ) << "KGame: message taken by property - returning" << endl;
	return ;
 }

 switch(msgid) {
	case KGameMessage::IdSetupGame:  // Client: First step in setup game
	{
		kdDebug(11001) << "KGame::networkTransmission:: Got IdSetupGame" << endl;
		setupGame(stream, sender);
		break;
	}
	case KGameMessage::IdContinueSetup:  // Master: second step in game setup
	{
		kdDebug(11001) << "KGame::networkTransmission() - IdContinueSetup" << endl;
		int count,i;
		QString ip;
		int port;
		stream >> ip >> port;
//		if (client) {
//			client->setIP(ip);//AB: obsolete?
//			client->setPort(port);//AB: obsolete?
//		}
		stream >> count;
		kdDebug(11001) << "IdContinueSetup: IP="<< ip <<endl;
		// stream >> count;
		kdDebug(11001) << "IdContinueSetup:We have to inactivate " << count << " players" << endl;
		for (i=0;i<count;i++) {
			int pid;
			stream >> pid;
			kdDebug(11001) << "Inactivate " << pid << endl;
			inactivatePlayer(findPlayer(pid));
		}
		QByteArray bufferS;
		QDataStream streamS(bufferS,IO_WriteOnly);
		save(streamS);
		// send back to sender therefore sender and receiver exchanged !!!
		sendSystemMessage(streamS,KGameMessage::IdGameSave,sender);
		sendSystemMessage(-1,KGameMessage::IdSendPlayer,sender);
		syncRandom();
      }
    break;
    case KGameMessage::IdSendPlayer:  // Send allplayer to the network
      {
        int unused;
        stream >> unused;
        kdDebug(11001) << "CLIENT: Got IdSendPlayer unused=" << unused << endl;
        SendAllPlayer(false);
      }
    break;
    case KGameMessage::IdActivatePlayer:  // Activate Player
      {
        int id;
        stream >> id;
        kdDebug(11001) << "Got IdActivatePlayer id=" << id << endl;
        systemActivatePlayer(findPlayer(id));
      }
    break;
    case KGameMessage::IdInactivatePlayer:  // Inactivate Player
      {
        int id;
        stream >> id;
        kdDebug(11001) << "Got IdInactivatePlayer id=" << id << endl;
        systemInactivatePlayer(findPlayer(id));
      }
    break;
    case KGameMessage::IdAddPlayer:
      {
        kdDebug(11001) << "KGame::slotNetworkTransmission:: Got IdAddPlayer" << endl;
        systemAddPlayer(stream);
      }
    break;
    case KGameMessage::IdRemovePlayer: // Client should delete player id
      {
        int id;
        stream >> id;
        kdDebug(11001) << "KGame::slotNetworkTransmission:: Got IdRemovePlayer " << id << endl;
        KPlayer *p=findPlayer(id);
        if (p) systemRemovePlayer(p);
      }
    break;
    case KGameMessage::IdGameSave:
      {
        kdDebug(11001) << "KGame::slotNetworkTransmission:: Got IdGameSave" << endl;
        loadgame(stream,true);
        KPlayer *player;
        int gameid=KGameMessage::calcMessageId(gameId(),0);
        for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() )
        {
          if ((player->id()&gameid)==gameid)
          {
            //kdDebug(11001) << "$$$ do not virtualizing player "<<player->id()<< " gameid="<<gameid << "=" << gameId() << endl;
            continue; // local player
          }
          // Create virtual input device
          // kdDebug(11001) << "$$$ Virtualizing player "<<player->id()<<endl;
          // player->setVirtual(true);

          // BL: Is this needed any longer? FIXME
          // if (client) client->playerList()->append(player);
        }
      }
    break;
    case KGameMessage::IdRandomSeed:  // Master forces a new random seed on us
      {
        int newseed;
        stream >> newseed;
        kdDebug(11001) << "CLIENT: setting random seed to " << newseed << endl;
        mRandom.setSeed(newseed);
      }
    break;
    case KGameMessage::IdMessage: // A user defined message arrived
      {
        kdDebug(11001) << "KGame::slotNetworkTransmision:: Got IdMessage -> emmiting signal "  << endl;
        emit signalMessage(stream);
      }
    break;
    default:
      {
        if (msgid < KGameMessage::IdUser)
        {
          kdError(11001) << "incorrect message id " << msgid << " - emit anyway"
			  << endl;
        }
        kdDebug(11001) << "KGame::slotNetworkTransmision:: User data msgid " << msgid << endl;
        emit signalNetworkData(msgid - KGameMessage::IdUser,((QBuffer*)stream.device())->readAll(),(int)receiver,sender);
      }
    break;
  }
  // hmmmm...but how else do we notice that somehting happended?
  emit signalMessageUpdate((int)msgid,receiver,sender);
}

void KGame::setupGame(QDataStream& stream, int sender)//AB: sender obsolete?
{
 // remove the local players from all lists first. We add them regulary after
 // the other player we received from the MASTER
 updatePlayerIds();
 QList<KPlayer> toBeAdded;
 QListIterator<KPlayer> it(mPlayerList);
 while (it.current()) {
	toBeAdded.append(it.current());
	systemRemove(it.current());
	++it;
 }
 if (mPlayerList.count() > 0) {
	kdFatal(11001) << "KGame::setupGame(): Player list is not empty!" << endl;
 }

 Q_INT32 server,maxPlayer,gameid;
 Q_INT32 count,parentGameId;
 KGameMessage::extractSetupGame(stream, server, maxPlayer, gameid, parentGameId, count);
 kdDebug(11001) << "   Server: " << server << endl;
 kdDebug(11001) << "   maxPlayer: " << maxPlayer << endl ;
 kdDebug(11001) << "   Gameid: " << gameid << endl;
 kdDebug(11001) << "   playerCount Server " << count << endl;

 // insert all players from the MASTER into the game
 for (int i = 0; i < count; i++) {
	systemAddPlayer(stream);
 }

 // now ask the MASTER to add our local players
 // if this is not possible by any reason (e.g. count > maxPlayers) then the
 // client must not even connect!!
 // So: deny connections which would cause playerCount to be > maxPlayers // TODO
 for (unsigned int i = 0; i < toBeAdded.count(); i++) {
	addPlayer(toBeAdded.at(i));
 }


/*
// AB: this stuff could be obsolete.
// we will see if we need it again one day - do we still have to activate a
// player?
 QByteArray bufferS;
 QDataStream streamS(bufferS,IO_WriteOnly);
 streamS << ((KGameClientSocket*)client)->IP();
 streamS << ((KGameClientSocket*)client)->port();
 int diffPlayer=maxPlayer - playerCount() - count;
 int listcnt = idList.count();
 int remoteCnt=0;
 // If this is less than zero we need to inactivate some players
 if (maxPlayer>0 && diffPlayer<0) {
	// First check how many remote players to inactivate
	KPlayer *p;
	for (i=0;i<(unsigned int)(-diffPlayer);i++) {
		int pid=*(idList.at(listcnt-1-i));
		p=findPlayer(pid);
		if (!p) {
			remoteCnt++;
		}
	}
	kdDebug(11001) << "Inactivate " << remoteCnt << " remote players" << endl;
	streamS << remoteCnt; // so many remote players to inactivate
	// now send which ones
	for (i=0;i<(unsigned int)(-diffPlayer);i++) {
		int pid=*(idList.at(listcnt-1-i));
		p=findPlayer(pid);
		if (p) {
			inactivatePlayer(p);
		} else {
			streamS << pid;
		}
	}
 } else {
	streamS << (int)0; // 0 players to inactivate
 }
  // send back to sender therefore sender and receiver exchanged !!!
 sendSystemMessage(streamS,KGameMessage::IdContinueSetup,sender);
*/
}

void KGame::syncRandom()
{
  int newseed=(int)mRandom.getLong(65535);
  sendSystemMessage(newseed,KGameMessage::IdRandomSeed); // Broadcast
  mRandom.setSeed(newseed);
}

void KGame::Debug()
{
   KGameNetwork::Debug();
   kdDebug(11001) << "------------------- KGAME -------------------------" << endl;
   kdDebug(11001) << "this:          " << this << endl;
   kdDebug(11001) << "uniquePlayer   " << d->mUniquePlayerNumber << endl;
   kdDebug(11001) << "gameStatus     " << gameStatus() << endl;
   kdDebug(11001) << "MaxPlayers :   " << maxPlayers() << endl;
   kdDebug(11001) << "NoOfPlayers :  " << playerCount() << endl;
   kdDebug(11001) << "NoOfInactive:  " << mInactivePlayerList.count() << endl;
   kdDebug(11001) << "---------------------------------------------------" << endl;
}

void KGame::slotConnectionLost(Q_UINT32 clientID)
{
  // We lost the socket connection...we better remove all players
  // belong to that socket NOW

  // BL: FIXME
  // This code doesn't work any more, since we have to KGameClient object
  // holding a playerList.
/*
  kdDebug(11001) << "KGame::slotConnectionLost(" << client << ")" << endl;
  if (!client) return ;
  KPlayer *player;
  kdDebug(11001) << "Playerlist of client=" << client->playerList()->count() << " count" << endl;
  while( (player=client->playerList()->first()) )
  {
    client->playerList()->remove(player);
    removePlayer(player,true);
    delete player;
  }
*/
}


// -------------------- Synchronisation -----------------------

// this initializes a newly connected client.
// we send the number of players (including type) as well as game status and
// properties to the client. After the initialization has been completed both
// clients should have the same status (ie players, properties, etc)
void KGame::negotiateNetworkGame(Q_UINT32 clientID)
{
 if (!gameMaster()) {
	kdError(11001) << "negotiateNetworkGame(): Serious WARNING..only gameMaster should call this" << endl;
	return ;
 }
 kdDebug(11001) << "negotiateNetworkGame(): clientID=" << clientID << endl;

 QByteArray buffer;
 QDataStream streamGS(buffer,IO_WriteOnly);

//AB: is clientID correct and useful ? could be obsolete
 KGameMessage::createSetupGame(streamGS, isOfferingConnections(),
		maxPlayers(), clientID, gameId(), playerCount());

 // Find out how many players we are going to send so that
 // the client can adapt
 KPlayer *player;
 for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() ) {
//	streamGS << (Q_INT32) player->id() << (Q_INT32)player->networkPriority();
	writeToStream(player, KGameMessage::calcMessageId(gameId(), 0), streamGS);
 }


 // send to the newly connected client *only*
 // AB: check if this is really received/used by the "receiver" client only. We
 // have currently at least two network ports which could break this...
 int receiver = KGameMessage::calcMessageId(clientID, 0);
 sendSystemMessage(((QBuffer*)streamGS.device())->buffer(), KGameMessage::IdSetupGame, receiver);
}

// omittagged: So many tagged player should not be send
bool KGame::SendAllPlayer(bool sendvirtual,int receiver)
{
  // client=0 is broadcast !

  // Send all player
  KPlayer *player;
  for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() )
  {
    if (!sendvirtual && player->isVirtual()) continue;
    kdDebug(11001) << "KGame::Sending addPlayer(" << player->id() << "," << player->isVirtual() << ")"  << endl;
    addPlayer(player, receiver);
  }
  kdDebug(11001) << "Client send all players to master " << endl;
  return true;
}


bool KGame::sendGroupMessage(QDataStream &msg, int msgid, int sender, const QString& group)
{
  KPlayer *player;
  for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() )
  {
    if (player && player->group()==group)
    {
      sendSystemMessage(msg,msgid,KGameMessage::calcMessageId(0,player->id()), sender);
    }
  }
  return true;
}

bool KGame::sendGroupMessage(const QString& msg, int msgid, int sender, const QString& group)
{
  QByteArray buffer;
  QDataStream stream(buffer, IO_WriteOnly);
  stream << msg;
  return sendGroupMessage(stream, msgid, sender, group);
}

void KGame::systemAddPlayer(QDataStream& stream)
{
// no need to check whether maxPlayers() is valid, as this is done in addPlayer
// AB: small bug - when a player is added and another one before systemAddPlayer
// is called, playerNo could already be > maxPlayers() // FIXME
  int rtti, owner, iovalue;
  stream >> rtti;
  stream >> owner;
  stream >> iovalue;
  kdDebug(11001) << "KGame::systemAddPlayer() rtti: " << rtti << " io=" << iovalue << endl;
  KPlayer *newplayer=0;
  if (owner == gameId()) {
    // we sent the message so the player is already available
    kdDebug(11001) << "dequeue previously added player" << endl;
    newplayer = d->mAddPlayerList.dequeue();
  } else {
    emit signalCreatePlayer(newplayer, rtti, iovalue, owner != gameId(), this);
    if (!newplayer) {
      kdDebug(11001) << "No user defined player created. Creating default KPlayer " << endl;
      newplayer = new KPlayer;
    } else {
      kdDebug(11001) << "   USER Player " << newplayer << " done player->rtti=" << newplayer->rtti() << " rtti=" << rtti << endl;
    }
    newplayer->load(stream);
  }
  if (!newplayer) {
    kdError(11001) << "KGame::systemAddPlayer() internal error: trying to add NULL player" << endl;
    return;
  }

  if (owner != gameId()) {
    newplayer->setVirtual(true);
  } else {
    newplayer->setVirtual(false);
  }

  systemAddPlayer(newplayer);// the final, local, adding
}

void KGame::systemAddPlayer(KPlayer* newplayer)
{
  if (!newplayer) {
    kdFatal(11001) << "trying to add NULL player in KGame::systemAddPlayer()" << endl;
  }
  if (newplayer->id() == 0) {
    d->mUniquePlayerNumber++;
    newplayer->setId(KGameMessage::calcMessageId(gameId(), 0) | d->mUniquePlayerNumber);
    kdDebug(11001) << "systemAddPlayer: player " << newplayer << " now has id " << newplayer->id() << endl;
  }

  if (findPlayer(newplayer->id())) {
    kdError(11001) << "ERROR: Double adding player !!!!! NOT GOOD !!!!!! " << newplayer->id() << "...I delete it again" << endl;
    delete newplayer;
  } else {
    // Create virtual input device
//    if (senderClient) {
//      senderClient->playerList()->append(newplayer);//AB: emm obsolete? KGameComServer...
//    }

    kdDebug(11001) << "Trying to add player " << newplayer <<" maxPlayers="<<maxPlayers()<<" playerCount="<<playerCount() << endl;
    // Add the virtual player to the game
    mPlayerList.append(newplayer); 
    newplayer->setGame(this);
    kdDebug() << newplayer->isVirtual() << endl;
    kdDebug(11001) << "@@@@@@@@@@@@ Player (id=" << newplayer->id() << ") #Players="
          << mPlayerList.count() << " added " << newplayer 
          << "  (virtual=" << newplayer->isVirtual() << ") @@@@@@@@@@@@" << endl;
    emit signalPlayerJoinedGame(newplayer);
  }
}


bool KGame::addProperty(KPlayerDataBase* data)
{
  return d->mProperties.addProperty(data);
}

void KGame::sendPlayerProperty(QDataStream& s, int playerId, bool /*isPublic*/)
{
// if (isPublic) // TODO
  sendSystemMessage(s, KGameMessage::IdPlayerProperty, KGameMessage::calcMessageId(0,playerId));
}
void KGame::sendProperty(QDataStream& s, bool /*isPublic*/)
{
// if (isPublic) // TODO
//  kdDebug(11001) << "KGame::sendProperty " << endl;
  sendSystemMessage(s, KGameMessage::IdGameProperty);
}
void KGame::emitSignal(KPlayerDataBase *me)
{
  emit signalPropertyChanged(me,this);
}

KPlayerDataBase* KGame::findProperty(int id) const
{
  return d->mProperties[id];
}

void KGame::updatePlayerIds()
{
// AB: quick hack - needs extensive testing!
// especially: what about the virtual players?
// do they exist here? this is only called if by setupGame()

  KPlayer *player;
  for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() )
  {
    if (player->isVirtual()) continue;
    player->setId(KGameMessage::calcMessageId(gameId(),0)|(player->id()&0x3ff));
  }
  for ( player=mInactivePlayerList.first(); player != 0; player=mInactivePlayerList.next() )
  {
    if (player->isVirtual()) continue;
    player->setId(KGameMessage::calcMessageId(gameId(),0)|(player->id()&0x3ff));
  }
}


#include "kgame.moc"
