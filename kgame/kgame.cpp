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

#include <qbuffer.h>
#include <qtimer.h>
#include <qqueue.h>

#include <klocale.h>
#include <krandomsequence.h>
#include <kdebug.h>

#include "kgameproperty.h"
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
        mCookie=0;
    }

    int mCookie;  // Application identifier
    int mUniquePlayerNumber;
    KPlayer *mCurrentPlayer;
    QQueue<KPlayer> mAddPlayerList;// this is a list of to-be-added players. See addPlayer() docu
    KRandomSequence* mRandom;


    KGamePropertyHandler<KGame> mProperties;

    // player lists
    KGame::KGamePlayerList mPlayerList;
    KGame::KGamePlayerList mInactivePlayerList;

    //KGamePropertys
    KGamePropertyInt mMaxPlayer;
    KGamePropertyUInt mMinPlayer;
    KGamePropertyInt mGameStatus; // Game running?

};

// ------------------- GAME CLASS --------------------------
KGame::KGame(int cookie,QObject* parent) : KGameNetwork(cookie,parent)
{
 kdDebug(11001) << "CREATE(KGame=" << this <<") sizeof(this)="<<sizeof(KGame) << endl;
 d = new KGamePrivate;
  
 d->mProperties.registerHandler(KGameMessage::IdGameProperty,this);
 d->mMaxPlayer.registerData(KGamePropertyBase::IdMaxPlayer, dataHandler());
 d->mMaxPlayer.setValue(-1);  // Infinite
 d->mMinPlayer.registerData(KGamePropertyBase::IdMinPlayer, dataHandler());
 d->mMinPlayer.setValue(0);   // Always ok     
 d->mGameStatus.registerData(KGamePropertyBase::IdGameStatus, dataHandler());
 d->mGameStatus.setValue(End);
 d->mUniquePlayerNumber=0;
 d->mCookie=cookie;
 d->mRandom = new KRandomSequence;
 d->mRandom->setSeed(0);

 connect(this, SIGNAL(signalClientConnected(Q_UINT32)),
 	this, SLOT(slotClientConnected(Q_UINT32)));
 connect(this, SIGNAL(signalClientDisconnected(Q_UINT32)),
 	this, SLOT(slotClientDisconnected(Q_UINT32)));


 // BL: FIXME This signal does no longer exist. When we are merging
 // MH: super....and how do I find out about the lost conenction now?
 // KGame and KGameNetwork, this could be improved!
//  connect(this,SIGNAL(signalConnectionLost(KGameClient *)),
//          this,SLOT(slotConnectionLost(KGameClient *)));
}

KGame::~KGame()
{
 kdDebug(11001) << "DESTRUCT(KGame=" << this <<")" << endl;
 Debug();
 reset();
 delete d->mRandom;
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
 while((player=d->mPlayerList.first())) {
   // AB: we cannot call KGame::removePlayer here, as we don't want any network
   // transmit in destructing. We just remove the players from the list and
   // delete them.
   d->mPlayerList.remove(player);
   delete player; // delete removes the player
 }
 kdDebug(11001) << "KGame::deletePlayers() done" << endl;
}

void KGame::deleteInactivePlayers()
{
 KPlayer *player;
 while((player=d->mInactivePlayerList.first())) {
   //player->setGame(0); // prevent call backs
   d->mInactivePlayerList.remove(player);
   delete player;
 }
}

bool KGame::load(QDataStream &stream)
{ return loadgame(stream, false); }

bool KGame::loadgame(QDataStream &stream, bool network)
{
 // Load Game Data
 uint i;
 d->mProperties.load(stream);

 // Load Playerobjects
 uint playercount;
 stream >> playercount;
 kdDebug(11001) << "Loading KGame " << playercount << " KPlayer objects " << endl;
 for (i=0;i<playercount;i++) {
   int rtti,id,iovalue;
   stream >> rtti >> id >> iovalue;
   KPlayer *newplayer=findPlayer(id);
   if (!newplayer) {
     kdDebug(11001) << "   Player "<< id << "not found...asking user to create one " << endl;
     emit signalCreatePlayer(newplayer,rtti,iovalue,network,this);
   }
   if (!newplayer) {
     kdDebug(11001) << "   No user defined player created. Creating default KPlayer " << endl;
     newplayer=new KPlayer;
   } else {
     kdDebug(11001) << "   USER Player " << newplayer << " done player->rtti=" << newplayer->rtti() << " rtti=" << rtti << endl;
   }
   newplayer->load(stream);
   if (network) {
     newplayer->setVirtual(true);
   }
   // AB: FIXME: are the players loaded properly? -> should be fixed (AB)
   systemAddPlayer(newplayer);
 }

 Q_INT16 cookie;
 stream >> cookie;
 if (cookie==KGAME_LOAD_COOKIE) {
   kdDebug(11001) << "   Game loaded propertly"<<endl;
 } else {
   kdError(11001) << "   Game loading error. probably format error"<<endl;
 }

 emit signalLoad(stream);
 return true;
}

bool KGame::save(QDataStream &stream,bool saveplayers)
{
 // Save Game Data
 d->mProperties.save(stream);

 if (saveplayers) {
   kdDebug(11001) << "Saving KGame " << playerCount() << " KPlayer objects " << endl;
   stream << playerCount();
   KPlayer *player;
   for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) {
     stream << (int)player->rtti();
     stream << (int)player->id();
     stream << (int)player->calcIOValue();
     player->save(stream);
   }
 } else {
   stream << (uint)0; // no players saved
 }

 stream << (Q_INT16)KGAME_LOAD_COOKIE;


 emit signalSave(stream);
 return true;
}


// ----------------- Player handling -----------------------

KPlayer * KGame::findPlayer(int id) const
{
 for (QListIterator<KPlayer> it(d->mPlayerList); it.current(); ++it) {
   if (it.current()->id() == id) {
     return it.current();
   }
 }
 for (QListIterator<KPlayer> it(d->mInactivePlayerList); it.current(); ++it) {
   if (it.current()->id() == id) {
     return it.current();
   }
 }
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
 savePlayer(stream,newplayer, receiver);
 d->mAddPlayerList.enqueue(newplayer);
 sendSystemMessage(stream,(int)KGameMessage::IdAddPlayer, receiver, KGameMessage::calcMessageId(gameId(),0));
}

void KGame::savePlayer(QDataStream &stream,KPlayer* p, int owner)
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

 if (d->mPlayerList.count() == 0) {
   result = false;
 } else {
   result = d->mPlayerList.remove(p);
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
   d->mPlayerList.remove(player);
   d->mInactivePlayerList.append(player);
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

 d->mInactivePlayerList.remove(player);
 player->setActive(true);
 addPlayer(player);
 return true;
}

// -------------------- Properties ---------------------------
int KGame::cookie() 
{
 return d->mCookie;
}
void KGame::setGameId(int id)
{
 kdError (11001) << "KGame::setGameID " << id << ": Is broken at the moment!" << endl;
 // FIXME: This is not possible at the moment!

/*
//AB: the following is completely obsolete. can probably be removed.
 KGameNetwork::setGameId(id);
 // Adjust all player id after the gameid got changed...which better happens ONLY by
 // a remote call by the gamemaster(admin?)!
 KPlayer *player;
 for ( player=mPlayerList.first(); player != 0; player=mPlayerList.next() ) {
   if (player->isVirtual()) continue;
   player->setId(KGameMessage::calcMessageId(gameId(),0)|(player->id()&0x3ff));
 }
 for ( player=mInactivePlayerList.first(); player != 0; player=mInactivePlayerList.next() ) {
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
{ return d->mPlayerList.count(); }

int KGame::gameStatus() const
{ return d->mGameStatus.value(); }

bool KGame::isRunning() const
{ return d->mGameStatus.value() == Run; }

KGamePropertyHandlerBase* KGame::dataHandler()
{ return &d->mProperties; }

KGame::KGamePlayerList* KGame::inactivePlayerList()
{ return &d->mInactivePlayerList; }

KGame::KGamePlayerList* KGame::playerList()
{ return &d->mPlayerList; }

const KGame::KGamePlayerList* KGame::playerList() const
{ return &d->mPlayerList; }

KRandomSequence* KGame::random()
{ return d->mRandom; }


bool KGame::sendPlayerInput(QDataStream &msg, KPlayer *player, int sender)
{
 if (!player) {
   kdError(11001) << "KGame::sendPlayerInput(): NULL player" << endl;
   return false;
 }
 if (!isRunning()) {
   kdError(11001) << "KGame::sendPlayerInput(): game not running" << endl;
   return false;
 }

 kdDebug(11001) << "KGame: transmitting playerInput over network" << endl;
 sendSystemMessage(msg, (int)KGameMessage::IdPlayerInput, KGameMessage::calcMessageId(0,player->id()), sender);
 return true;
}

bool KGame::playerInput(QDataStream &msg, KPlayer *player, int sender)
{
  /*
  QBuffer *b=(QBuffer *)msg.device();
  kdDebug() << "size of stream=" << b->buffer().size() << endl;
  char *p=b->buffer().data();
  for (int i=0;i<b->buffer().size();i++) fprintf(stderr,"%02x ",p[i]);fprintf(stderr,"\n");
  */

  
 if (!player) {
   kdError(11001) << "KGame::playerInput(): NULL player" << endl;
   return false;
 }
 if (!isRunning()) {
   kdError(11001) << "KGame::playerInput(): game not running" << endl;
   return false;
 }
 kdDebug(11001) << "KGame: Got playerInput from messageServer... sender: " << sender << endl;
 kdDebug(11001) << " ... emmitting signal" << endl;
 emit signalPlayerInput(msg,player); 
  
 d->mCurrentPlayer=player;
 // Check for game over and if not allow the next player to move
 int over=gameOver(player);
 if (over!=0) {
   emit signalGameOver(over,player,this);
 } else if (!player->asyncInput()) {
   QTimer::singleShot(0,this,SLOT(prepareNext()));
 }

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
 if (last) {
   lastId=last->id();
 } else {
   lastId=-1;
 }
 minId=0xffff;
 nextId=minId;
 nextplayer=0;
 minplayer=0;

 KPlayer *player;
 for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) {
   // Find the first player for a cycle
   if (player->id()<minId) {
     minId=player->id();
     minplayer=player;
   }
   if (player==last) {
     continue;
   }
   // Find the next player which is bigger than the current one
   if (player->id()>lastId && player->id()<nextId) {
     nextId=player->id();
     nextplayer=player;
   }
 }

 // Cycle to the beginning
 if (!nextplayer) {
   nextplayer=minplayer;
 }

 kdDebug(11001) << "KGame::nextPlayer ##### lastId=" << lastId << " excluseive=" << exclusive << "  minId=" << minId << " bextid=" << nextId << " count=" <<d->mPlayerList.count()  << endl;
 if (nextplayer) {
   nextplayer->setTurn(true,exclusive);
 }
 else {
   return false;
 }
 return true;
}

void KGame::setGameStatus(int status)
{
 kdDebug(11001) << "KGame::gameStatus(" << status << ")" << endl;
 if (status==(int)Run && playerCount()<minPlayers()) {
   kdDebug(11001) << "KGame::gameStatus: not enough players, pausing game\n" << endl;
   status=Pause;
 }
 d->mGameStatus.setValue(status);
}

void KGame::networkTransmission(QDataStream &stream,int msgid,int receiver,int sender, Q_UINT32 /*clientID*/)
{//clientID is unused
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
//   kdDebug(11001 ) << "KGame: message taken by property - returning" << endl;
   return ;
 }

 switch(msgid) {
   case KGameMessage::IdSetupGame:  // Client: First step in setup game
   {
     Q_INT16 v;
     Q_INT32 c;
     stream >> v >> c;
     kdDebug(11001) << " ===================> (Client) KGame::networkTransmission:: Got IdSetupGame ================== " << endl;
     kdDebug(11001) << "our game id is " << gameId() << " Lib version=" << v << " App Cookie=" << c << endl; 
     if (c!=cookie()) {
       kdError(11001) << "IdGameSetup: Negotiate Game: cookie mismatch I'am="<<cookie()<<" master="<<c<<endl;
       // MH TODO: disconnect from master
     } else {
       setupGame(sender);
     }
   }
   break;
   case KGameMessage::IdSetupGameContinue:  // Master: second step in game setup
   {
     kdDebug(11001) << "=====>(Master) KGame::networkTransmission() - IdSetupGameContinue" << endl;
     setupGameContinue(stream, sender);
     syncRandom();
   }
   break;
   case KGameMessage::IdGameReactivatePlayer:  // Client: Reactivate player
   {
     kdDebug(11001) << "=====>(client) KGame::networkTransmission() - IdGameRactivatePlayer" << endl;
     gameReactivatePlayer(stream, sender);
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
     if (p) {
       systemRemovePlayer(p);
     }
   }
   break;
   case KGameMessage::IdGameLoad:
   {
     kdDebug(11001) << "====> (Client) KGame::slotNetworkTransmission:: Got IdGameLoad" << endl;
     loadgame(stream,true);
     KPlayer *player;
     int gameid=KGameMessage::calcMessageId(gameId(),0);
     for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) {
       if ((player->id()&gameid)==gameid) {
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
   case KGameMessage::IdSyncRandom:  // Master forces a new random seed on us
   {
     int newseed;
     stream >> newseed;
     kdDebug(11001) << "CLIENT: setting random seed to " << newseed << endl;
     d->mRandom->setSeed(newseed);
   }
   break;
   case KGameMessage::IdDisconnect:
   {
   // if we disconnect we *always* start a local game. 
   // this could lead into problems if we just change the message server
     if (sender != gameId()) {
         kdDebug(11001) << "client " << sender << " leaves game" << endl;
	 return;
     }
     kdDebug(11001) << "leaving the game" << endl;
     // start a new local game
     // no other client is by default connected to this so this call should be
     // enough
     setMaster();
   }
   break;
   default:
    {
     if (msgid < KGameMessage::IdUser) {
       kdError(11001) << "incorrect message id " << msgid << " - emit anyway"
                      << endl;
     }
     kdDebug(11001) << "KGame::slotNetworkTransmision:: User data msgid " << msgid << endl;
     emit signalNetworkData(msgid - KGameMessage::IdUser,((QBuffer*)stream.device())->readAll(),(int)receiver,sender);
   }
   break;
 }

 // notice the game that something has changed
 emit signalMessageUpdate((int)msgid,receiver,sender);
}

// called by the IdSetupGameContinue Message - MASTER SIDE
void KGame::setupGameContinue(QDataStream& stream, int sender)
{
 QValueList<int> playerId;
 QValueList<int> playerPriority;
 stream >> playerId;
 stream >> playerPriority;

 QValueList<int> clientActivate;
 clientActivate=playerId;  // first we try to activate all client players
 kdDebug(11001) << " Master calculates how many players to activate cleint has cnt=" << clientActivate.count() << endl;
 kdDebug(11001) << "Priorites cnt=" << playerPriority.count() << endl;

 kdDebug(11001) << "setupGameContinue:: idcount=" << playerId.count() << " pricnt=" << playerPriority.count() << endl;

 // Add all other network players to the client list - we are master here
 QListIterator<KPlayer> it(d->mPlayerList);
 while (it.current()) {
  playerId.append(it.current()->id());
  playerPriority.append(it.current()->networkPriority());
 ++it;
 }

 while(maxPlayers()>=0 && playerId.count()>(unsigned int)maxPlayers()) {
   int minvalue=playerPriority[0];
   int minpos=0;
   for (unsigned int i=0;i<playerPriority.count();i++) {
     // if here is a < the clients player will be kicked out first
     // if it is a <= the masters player is out first (which is not so good)
     if (playerPriority[i]<minvalue) {
       minvalue=playerPriority[i];
       minpos=i;
     }
   }
   if (playerId.end()!=clientActivate.find(playerId[minpos])) {
     kdDebug(11001) << " Client should not activate player ID="<<playerId[minpos]<<endl; 
     clientActivate.remove(playerId[minpos]);  // remove from client
   } else {
     kdDebug(11001) << " Master should INactivate player ID="<<playerId[minpos]<<endl; 
     systemInactivatePlayer(findPlayer(playerId[minpos])); // remove from rest of game (master)
   }
   playerPriority.remove(playerPriority.at(minpos));
   playerId.remove(playerId.at(minpos));
 }

 // Save the game over the network
 QByteArray bufferS;
 QDataStream streamS(bufferS,IO_WriteOnly);
 save(streamS);
 sendSystemMessage(streamS,KGameMessage::IdGameLoad,sender);
 
 // Now send out the player list which the client can activate
 QByteArray bufferR;
 QDataStream streamR(bufferR,IO_WriteOnly);
 streamR << clientActivate;
 sendSystemMessage(streamR,KGameMessage::IdGameReactivatePlayer,sender);
}

// called by the IdGameReactivatePlayer Message - CLIENT SIDE
void KGame::gameReactivatePlayer(QDataStream& stream, int sender)  
{
 QValueList<int> activatePlayer;
 stream >> activatePlayer;
 kdDebug(11001) << "gameReactivatePlayer cnt="<< activatePlayer.count() << endl;
 QValueList<int>::Iterator it;
 for( it = activatePlayer.begin(); it != activatePlayer.end(); ++it ) {
   int id=(*it);
   kdDebug(11001) << "CLIENT: reactivate player id=" <<   id << endl;
   systemActivatePlayer(findPlayer(id));
 }
}

// called by the IdSetupGame Message - CLIENT SIDE
void KGame::setupGame(int sender)  
{
 // remove the local players from all lists first. We add them regulary after
 // the other player we received from the MASTER
 updatePlayerIds();

 QValueList<int> playerId;
 QValueList<int> playerPriority;
 
 // Deactivate all players
 // QList<KPlayer> toBeAdded;
 QListIterator<KPlayer> it(d->mPlayerList);
 while (it.current()) {
   //toBeAdded.append(it.current());
   //systemRemove(it.current());
   playerId.append(it.current()->id());
   playerPriority.append(it.current()->networkPriority());
   systemInactivatePlayer(it.current());
   ++it;
 }
 if (d->mPlayerList.count() > 0) {
   kdFatal(11001) << "KGame::setupGame(): Player list is not empty!" << endl;
 }

 QByteArray bufferS;
 QDataStream streamS(bufferS,IO_WriteOnly);
 streamS << playerId;
 streamS << playerPriority;
 sendSystemMessage(streamS,KGameMessage::IdSetupGameContinue,sender);
}

void KGame::syncRandom()
{
 int newseed=(int)d->mRandom->getLong(65535);
 sendSystemMessage(newseed,KGameMessage::IdSyncRandom); // Broadcast
 d->mRandom->setSeed(newseed);
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
 kdDebug(11001) << "NoOfInactive:  " << d->mInactivePlayerList.count() << endl;
 kdDebug(11001) << "---------------------------------------------------" << endl;
}

void KGame::slotClientConnected(Q_UINT32 clientID)
{
 if (isAdmin()) {
	negotiateNetworkGame(clientID);
 }
}

void KGame::slotClientDisconnected(Q_UINT32 clientID)
{
 //TODO: remove/replace the players of this client and do some other cleanup
 //stuff (can the game still be continued?)

//FIXME: the code below was in slotConnectionLost which is obsolete. Must be
//replaced/rewritten somehow!
/*
 kdDebug(11001) << "KGame::slotConnectionLost(" << client << ")" << endl;
 if (!client) return ;
 KPlayer *player;
 kdDebug(11001) << "Playerlist of client=" << client->playerList()->count() << " count" << endl;
 while( (player=client->playerList()->first()) ) {
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
 kdDebug(11001) << "===========================negotiateNetworkGame(): clientID=" << clientID << " =========================== "<< endl;
 if (!isAdmin()) {
   kdError(11001) << "negotiateNetworkGame(): Serious WARNING..only gameAdmin should call this" << endl;
   return ;
 }

 QByteArray buffer;
 QDataStream streamGS(buffer,IO_WriteOnly);

 // write Game setup specific data
 //streamGS << (Q_INT32)maxPlayers();
 //streamGS << (Q_INT32)minPlayers();

 // send to the newly connected client *only*
 // AB: check if this is really received/used by the "receiver" client only. We
 // have currently at least two network ports which could break this...
 int receiver = KGameMessage::calcMessageId(clientID, 0);
 Q_INT16 v=KGameMessage::version();
 Q_INT32 c=cookie();
 streamGS << v << c;
 sendSystemMessage(streamGS, KGameMessage::IdSetupGame, receiver);
}

bool KGame::sendGroupMessage(const QByteArray &msg, int msgid, int sender, const QString& group)
{
// AB: group must not be i18n'ed!! we should better use an id for group and use
// a groupName() for the name // FIXME
 KPlayer *player;
 for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) {
   if (player && player->group()==group) {
     sendMessage(msg,msgid,KGameMessage::calcMessageId(0,player->id()), sender);
   }
 }
 return true;
}

bool KGame::sendGroupMessage(const QDataStream &msg, int msgid, int sender, const QString& group)
{ return sendGroupMessage(((QBuffer*)msg.device())->buffer(), msgid, sender, group); }

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
 if (owner == (int)gameId()) {
   // we sent the message so the player is already available
   kdDebug(11001) << "dequeue previously added player" << endl;
   newplayer = d->mAddPlayerList.dequeue();
 } else {
   emit signalCreatePlayer(newplayer, rtti, iovalue, owner != (int)gameId(), this);
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

 if (owner != (int)gameId()) {
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
   kdDebug(11001) << "systemAddPlayer: NEW!!! player " << newplayer << " now has id " << newplayer->id() << endl;
 }

 if (findPlayer(newplayer->id())) {
   kdError(11001) << "ERROR: Double adding player !!!!! NOT GOOD !!!!!! " << newplayer->id() << "...I delete it again" << endl;
   delete newplayer;
 } else {
   // Create virtual input device
//   if (senderClient) {
//     senderClient->playerList()->append(newplayer);//AB: emm obsolete? KGameComServer...
//   }

   kdDebug(11001) << "Trying to add player " << newplayer <<" maxPlayers="<<maxPlayers()<<" playerCount="<<playerCount() << endl;
   // Add the virtual player to the game
   d->mPlayerList.append(newplayer); 
   newplayer->setGame(this);
   kdDebug() << "Player is virtual=" << newplayer->isVirtual() << endl;
   kdDebug(11001) << "@@@@@@@@@@@@ Player (id=" << newplayer->id() << ") #Players="
                  << d->mPlayerList.count() << " added " << newplayer 
                  << "  (virtual=" << newplayer->isVirtual() << ") @@@@@@@@@@@@" << endl;
   emit signalPlayerJoinedGame(newplayer);
 }
}


bool KGame::addProperty(KGamePropertyBase* data)
{ return d->mProperties.addProperty(data); }

void KGame::sendPlayerProperty(QDataStream& s, int playerId)
{ sendSystemMessage(s, KGameMessage::IdPlayerProperty, KGameMessage::calcMessageId(0,playerId)); }

void KGame::sendProperty(QDataStream& s)
{ sendSystemMessage(s, KGameMessage::IdGameProperty); }

void KGame::emitSignal(KGamePropertyBase *me)
{ emit signalPropertyChanged(me,this); }

KGamePropertyBase* KGame::findProperty(int id) const
{ return d->mProperties[id]; }

void KGame::updatePlayerIds()
{
// AB: quick hack - needs extensive testing!
// especially: what about the virtual players?
// do they exist here? this is only called if by setupGame()

 KPlayer *player;
 for ( player=d->mPlayerList.first(); player != 0; player=d->mPlayerList.next() ) {
   if (player->isVirtual()) {
     continue;
   }
   player->setId(KGameMessage::calcMessageId(gameId(),0)|(player->id()&0x3ff));
 }
 for ( player=d->mInactivePlayerList.first(); player != 0; player=d->mInactivePlayerList.next() ) {
   if (player->isVirtual()) {
     continue;
   }
   player->setId(KGameMessage::calcMessageId(gameId(),0)|(player->id()&0x3ff));
 }
}

#include "kgame.moc"
