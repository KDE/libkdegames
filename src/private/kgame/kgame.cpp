/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgame.h"

// own
#include "kgamepropertyhandler.h"
#include "kgameproperty.h"
#include "kplayer.h"
#include "kgameio.h"
#include "kgameerror.h"
#include "kgamesequence.h"
#include "kgamemessage.h"
// KF
#include <KLocalizedString>
#include <KRandomSequence>
// Qt
#include <QBuffer>
#include <QTimer>
#include <QFile>
#include <QQueue>
// Std
#include <cstdio>
#include <cassert>

#define KGAME_LOAD_COOKIE 4210

Q_LOGGING_CATEGORY(GAMES_PRIVATE_KGAME, "org.kde.games.private.kgame", QtWarningMsg)

// try to place as much as possible here
// many things are *not* possible here as KGame has to use some inline function
class KGamePrivate
{
public:
    KGamePrivate()
    {
        mUniquePlayerNumber = 0;
        mPolicy=KGame::PolicyLocal;
        mGameSequence = nullptr;
    }

    int mUniquePlayerNumber;
    QQueue<KPlayer*> mAddPlayerList;// this is a list of to-be-added players. See addPlayer() docu
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
    KRandomSequence* mRandom;
#endif
    KGame::GamePolicy mPolicy;
    KGameSequence* mGameSequence;


    KGamePropertyHandler* mProperties;

    // player lists
    KGame::KGamePlayerList mPlayerList;
    KGame::KGamePlayerList mInactivePlayerList;

    //KGamePropertys
    KGamePropertyInt mMaxPlayer;
    KGamePropertyUInt mMinPlayer;
    KGamePropertyInt mGameStatus; // Game running?
    QList<int> mInactiveIdList;

};

// ------------------- GAME CLASS --------------------------
KGame::KGame(int cookie,QObject* parent)
    : KGameNetwork(cookie,parent),
      d( new KGamePrivate )
{
 qCDebug(GAMES_PRIVATE_KGAME) << " - " << this << ", sizeof(KGame)=" << sizeof(KGame);

 d->mProperties = new KGamePropertyHandler(this);

 d->mProperties->registerHandler(KGameMessage::IdGameProperty,
                                this,SLOT(sendProperty(int,QDataStream&,bool*)),
                                     SLOT(emitSignal(KGamePropertyBase*)));
 d->mMaxPlayer.registerData(KGamePropertyBase::IdMaxPlayer, this, i18n("MaxPlayers"));
 d->mMaxPlayer.setLocal(-1);  // Infinite
 d->mMinPlayer.registerData(KGamePropertyBase::IdMinPlayer, this, i18n("MinPlayers"));
 d->mMinPlayer.setLocal(0);   // Always ok
 d->mGameStatus.registerData(KGamePropertyBase::IdGameStatus, this, i18n("GameStatus"));
 d->mGameStatus.setLocal(Init);
 // d->mUniquePlayerNumber = 0;
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
 d->mRandom = new KRandomSequence;
QT_WARNING_POP
 d->mRandom->setSeed(0);
#endif

 connect(this, &KGame::signalClientConnected, this, &KGame::slotClientConnected);
 connect(this, &KGame::signalClientDisconnected, this, &KGame::slotClientDisconnected);
 connect(this, &KGame::signalConnectionBroken, this, &KGame::slotServerDisconnected);

 setGameSequence(new KGameSequence());

 // BL: FIXME This signal does no longer exist. When we are merging
 // MH: super....and how do I find out about the lost connection now?
 // KGame and KGameNetwork, this could be improved!
//  connect(this,SIGNAL(signalConnectionLost(KGameClient*)),
//          this,SLOT(slotConnectionLost(KGameClient*)));
}

KGame::~KGame()
{
 qCDebug(GAMES_PRIVATE_KGAME) ;
// Debug();
 reset();
 delete d->mGameSequence;
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
 delete d->mRandom;
#endif
 delete d;
 qCDebug(GAMES_PRIVATE_KGAME) << "done";
}

bool KGame::reset()
{
 deletePlayers();
 deleteInactivePlayers();
 return true;
}

void KGame::deletePlayers()
{
// qCDebug(GAMES_PRIVATE_KGAME) ;
 /* Bugs 303142 and 305000. KPlayer destructor removes
   * player from the list and makes iterators invalid.
   * qDeleteAll crashes in that case. */
  while (!d->mPlayerList.isEmpty()) 
  {
    delete d->mPlayerList.takeFirst();
  }
 // qDeleteAll(d->mPlayerList);
 //NOTE by majewsky: An earlier implementation copied the mPlayerList before
 //deleting the elements with a takeFirst loop. I therefore chose not to clear()
 //the list in order not to break anything. The old code had the following
 //comment: "in case of PolicyClean player=d->mPlayerList.first() is infinite"
// qCDebug(GAMES_PRIVATE_KGAME) << "done";
}

void KGame::deleteInactivePlayers()
{
 qDeleteAll(d->mInactivePlayerList);
 d->mInactivePlayerList.clear();
}

bool KGame::load(const QString& filename,bool reset)
{
  if (filename.isNull())
  {
    return false;
  }
  QFile f(filename);
  if (!f.open(QIODevice::ReadOnly))
  {
    return false;
  }
  QDataStream s( &f );
  load(s,reset);
  f.close();
  return true;
}

bool KGame::load(QDataStream &stream,bool reset)
{ return loadgame(stream, false,reset); }

bool KGame::loadgame(QDataStream &stream, bool network,bool resetgame)
{
 // Load Game Data

 // internal data
 qint32 c;
 stream >> c; // cookie

 if (c!=cookie())
 {
   qCWarning(GAMES_PRIVATE_KGAME) << "Trying to load different game version we="<<cookie() << "saved=" << c;
   bool result=false;
   Q_EMIT signalLoadError(stream,network,(int)c,result);
   return result;
 }
 if (resetgame) reset();

 uint i;
 stream >> i;
// setPolicy((GamePolicy)i);

 stream >> d->mUniquePlayerNumber;

 if (gameSequence())
 {
   gameSequence()->setCurrentPlayer(nullptr);  // TODO !!!
 }

#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
 int newseed;
 stream >> newseed;
 d->mRandom->setSeed(newseed);
#endif

 // Switch off the direct emitting of signals while
 // loading properties. This can cause inconsistencies
 // otherwise if a property emits and this emit accesses
 // a property not yet loaded
 // Note we have to have this external locking to prevent the games unlocking
 // to access the players
 dataHandler()->lockDirectEmit();

 for ( KGamePlayerList::iterator it = playerList()->begin(); it!=playerList()->end();++it )
 {
   (*it)->dataHandler()->lockDirectEmit();
   // qCDebug(GAMES_PRIVATE_KGAME) << "Player "<<player->id() << "to indirect emit";
 }

 // Properties
 dataHandler()->load(stream);

 // If there is additional data to be loaded before players are loaded then do
 // this here.
 Q_EMIT signalLoadPrePlayers(stream);

 // Switch back on the direct emitting of signals and emit the
 // queued signals for properties.
 // Unlocks properties before loading players in order to make game
 // initializations related to properties before using them in players
 // initialization
 dataHandler()->unlockDirectEmit();

 // Load Playerobjects
 uint playercount;
 stream >> playercount;
 qCDebug(GAMES_PRIVATE_KGAME) << "Loading KGame" << playercount << "KPlayer objects";
 for (i=0;i<playercount;++i)
 {
   KPlayer *newplayer=loadPlayer(stream,network);
   systemAddPlayer(newplayer);
 }

 qint16 cookie;
 stream >> cookie;
 if (cookie==KGAME_LOAD_COOKIE) {
   qCDebug(GAMES_PRIVATE_KGAME) << "   Game loaded properly";
 } else {
   qCCritical(GAMES_PRIVATE_KGAME) << "   Game loading error. probably format error";
 }

 // Switch back on the direct emitting of signals and emit the
 // queued signals for players.
 // Note we habe to have this external locking to prevent the games unlocking
 // to access the players
 for ( KGamePlayerList::iterator it = playerList()->begin(); it!=playerList()->end();++it )
 {
   (*it)->dataHandler()->unlockDirectEmit();
   // qCDebug(GAMES_PRIVATE_KGAME) << "Player "<<player->id() << "to direct emit";
 }

 Q_EMIT signalLoad(stream);
 return true;
}

bool KGame::save(const QString& filename,bool saveplayers)
{
 if (filename.isNull())
 {
   return false;
 }
 QFile f(filename);
 if (!f.open(QIODevice::WriteOnly))
 {
   return false;
 }
 QDataStream s( &f );
 save(s,saveplayers);
 f.close();
 return true;
}

bool KGame::save(QDataStream &stream,bool saveplayers)
{ return savegame(stream, false,saveplayers); }

bool KGame::savegame(QDataStream &stream,bool /*network*/,bool saveplayers)
{
  // Save Game Data

  // internal variables
  qint32 c=cookie();
  stream << c;

  uint p=(uint)policy();
  stream << p;
  stream << d->mUniquePlayerNumber;
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
  int newseed=(int)d->mRandom->getLong(65535);
  stream << newseed;
  d->mRandom->setSeed(newseed);
#endif

 // Properties
 dataHandler()->save(stream);

 // Save all data that need to be saved *before* the players are saved
 Q_EMIT signalSavePrePlayers(stream);

 if (saveplayers)
 {
   savePlayers(stream,playerList());
 }
 else
 {
   stream << (uint)0; // no players saved
 }

 stream << (qint16)KGAME_LOAD_COOKIE;

 Q_EMIT signalSave(stream);
 return true;
}

void KGame::savePlayer(QDataStream &stream,KPlayer* p)
{
// this could be in KGameMessage as well
 stream << (qint32)p->rtti();
 stream << (qint32)p->id();
 stream << (qint32)p->calcIOValue();
 p->save(stream);
}

void KGame::savePlayers(QDataStream &stream, KGamePlayerList *list)
{
 if (!list)
 {
   list=playerList();
 }

 qint32 cnt=list->count();
 qCDebug(GAMES_PRIVATE_KGAME) << "Saving KGame" << cnt << "KPlayer objects";
 stream << cnt;

 for ( KGamePlayerList::iterator it = playerList()->begin(); it!=playerList()->end();++it )
 {
   savePlayer(stream,*it);
 }
}

KPlayer *KGame::createPlayer(int /*rtti*/,int /*io*/,bool /*isvirtual*/)
{
  qCWarning(GAMES_PRIVATE_KGAME) << "   No user defined player created. Creating default KPlayer. This crashes if you have overwritten KPlayer!!!! ";
  return new KPlayer;
}
KPlayer *KGame::loadPlayer(QDataStream& stream,bool isvirtual)
{
  qint32 rtti,id,iovalue;
  stream >> rtti >> id >> iovalue;
  KPlayer *newplayer=findPlayer(id);
  if (!newplayer)
  {
    qCDebug(GAMES_PRIVATE_KGAME) << "Player "<< id << "not found...asking user to create one";
    newplayer=createPlayer(rtti,iovalue,isvirtual);
    //Q_EMIT signalCreatePlayer(newplayer,rtti,iovalue,isvirtual,this);
  }
  /*
  if (!newplayer)
  {
    qCWarning(GAMES_PRIVATE_KGAME) << "   No user defined player created. Creating default KPlayer. This crashes if you have overwritten KPlayer!!!! ";
    newplayer=new KPlayer;
  }
  else
  {
    qCDebug(GAMES_PRIVATE_KGAME) << "   USER Player" << newplayer << "done player->rtti=" << newplayer->rtti() << "rtti=" << rtti;
  }
  */
  newplayer->load(stream);
  if (isvirtual)
  {
    newplayer->setVirtual(true);
  }
  return newplayer;
}

// ----------------- Player handling -----------------------

KPlayer * KGame::findPlayer(quint32 id) const
{
  for ( KGamePlayerList::iterator it = d->mPlayerList.begin(); it!=d->mPlayerList.end();++it )
 {
   if ((*it)->id() == id)
   {
     return *it;
   }
 }
 for ( KGamePlayerList::iterator it = d->mInactivePlayerList.begin(); it!=d->mInactivePlayerList.end();++it )
 {
   if ((*it)->id() == id)
   {
     return *it;
   }
 }
 return nullptr;
}

// it is necessary that addPlayer and systemAddPlayer are called in the same
// order. Ie if addPlayer(foo) followed by addPlayer(bar) is called, you must
// not call systemAddPlayer(bar) followed by systemAddPlayer(foo), as the
// mAddPlayerList would get confused. Should be no problem as long as comServer
// and the clients are working correctly.
// BUT: if addPlayer(foo) does not arrive by any reason while addPlayer(bar)
// does, we would be in trouble...
bool KGame::addPlayer(KPlayer* newplayer)
{
 qCDebug(GAMES_PRIVATE_KGAME) << ":  " << "; maxPlayers=" << maxPlayers() << "playerCount=" << playerCount();
 if (!newplayer)
 {
  qCWarning(GAMES_PRIVATE_KGAME) << "trying to add NULL player in KGame::addPlayer()";
  return false;
 }

 if (maxPlayers() >= 0 && (int)playerCount() >= maxPlayers())
 {
   qCWarning(GAMES_PRIVATE_KGAME) << "cannot add more than" << maxPlayers() << "players - deleting...";
   return false;
 }

 if (newplayer->id() == 0)
 {
   d->mUniquePlayerNumber++;
   newplayer->setId(KGameMessage::createPlayerId(d->mUniquePlayerNumber, gameId()));
   qCDebug(GAMES_PRIVATE_KGAME) << "NEW!!! player" << newplayer << "now has id" << newplayer->id();
 }
 else
 {
   // this could happen in games which use their own ID management by certain
   // reasons. that is NOT recommended
   qCDebug(GAMES_PRIVATE_KGAME) << "player" << newplayer << "already has an id:" << newplayer->id();
 }

 QByteArray buffer;
 QDataStream stream(&buffer,QIODevice::WriteOnly);
 // We distinguish here what policy we have
 if (policy()==PolicyLocal || policy()==PolicyDirty)
 {
   if ( !systemAddPlayer(newplayer) )
       return false;
 }
 if (policy()==PolicyClean || policy()==PolicyDirty)
 {
   savePlayer(stream,newplayer);
   // Store the player for delayed clean adding
   if (policy()==PolicyClean)
   {
     d->mAddPlayerList.enqueue(newplayer);
   }
   sendSystemMessage(stream,(int)KGameMessage::IdAddPlayer, 0);
 }
 return true;
}

bool KGame::systemAddPlayer(KPlayer* newplayer)
{
 if (!newplayer)
 {
   qCWarning(GAMES_PRIVATE_KGAME) << "trying to add NULL player in KGame::systemAddPlayer()";
   return false ;
 }
 if (newplayer->id() == 0)
 {
   qCWarning(GAMES_PRIVATE_KGAME) << "player" << newplayer << "has no ID";
 }

 if (findPlayer(newplayer->id()))
 {
   qCCritical(GAMES_PRIVATE_KGAME) << "ERROR: Double adding player !!!!! NOT GOOD !!!!!! " << newplayer->id() << "...I delete it again";
   delete newplayer;
   return false;
 }
 else
 {
   qCDebug(GAMES_PRIVATE_KGAME) << "Trying to add player" << newplayer <<" maxPlayers="<<maxPlayers()<<" playerCount="<<playerCount();
   // Add the player to the game
   d->mPlayerList.append(newplayer);
   newplayer->setGame(this);
   qCDebug(GAMES_PRIVATE_KGAME) << "Player: isVirtual=" << newplayer->isVirtual();
   qCDebug(GAMES_PRIVATE_KGAME) << "        id=" << newplayer->id() << "  #Players="
                  << d->mPlayerList.count() << "added" << newplayer
                  << "  (virtual=" << newplayer->isVirtual() << ")";
   Q_EMIT signalPlayerJoinedGame(newplayer);
 }
 return true;
}

// Called by the KPlayer destructor
void KGame::playerDeleted(KPlayer *player)
{
 qCDebug(GAMES_PRIVATE_KGAME) << ": id (" << player->id() << ") to be removed" << player;

 if (policy()==PolicyLocal || policy()==PolicyDirty)
 {
   systemRemovePlayer(player,false);
 }
 if (policy()==PolicyClean || policy()==PolicyDirty)
 {
   if (!player->isVirtual())
   {
     qCDebug(GAMES_PRIVATE_KGAME) << ": sending IdRemovePlayer "<<player->id();
     sendSystemMessage(player->id(), KGameMessage::IdRemovePlayer, 0);
   }
 }
}

bool KGame::removePlayer(KPlayer * player, quint32 receiver)
{//transmit to all clients, or to receiver only
 if (!player)
 {
   qCWarning(GAMES_PRIVATE_KGAME) << "trying to remove NULL player in KGame::removePlayer(  )" ;
   return false;
 }
 qCDebug(GAMES_PRIVATE_KGAME) << ": id (" << player->id() << ") to be removed" << player;

 if (policy()==PolicyLocal || policy()==PolicyDirty)
 {
     systemRemovePlayer(player,true);
     return true; // player is gone
 }
 if (policy()==PolicyClean || policy()==PolicyDirty)
 {
   qCDebug(GAMES_PRIVATE_KGAME) << ": sending IdRemovePlayer "<<player->id();
   sendSystemMessage(player->id(),KGameMessage::IdRemovePlayer, receiver);
 }
 return true;
 // we will receive the message in networkTransmission()
}

void KGame::systemRemovePlayer(KPlayer* player,bool deleteit)
{
 qCDebug(GAMES_PRIVATE_KGAME) ;
 if (!player)
 {
   qCWarning(GAMES_PRIVATE_KGAME) << "cannot remove NULL player";
   return;
 }
 systemRemove(player,deleteit);

 if (gameStatus()==(int)Run && playerCount()<minPlayers())
 {
   qCWarning(GAMES_PRIVATE_KGAME) << ": not enough players, PAUSING game\n";
   setGameStatus(Pause);
 }
}

bool KGame::systemRemove(KPlayer* p,bool deleteit)
{
 if (!p)
 {
   qCWarning(GAMES_PRIVATE_KGAME) << "cannot remove NULL player";
   return false;
 }
 bool result;
 qCDebug(GAMES_PRIVATE_KGAME) << ": Player (" << p->id() << ") to be removed" << p;

 if (d->mPlayerList.count() == 0)
 {
   result = false;
 }
 else
 {

   result = d->mPlayerList.removeAll(p);
 }

 Q_EMIT signalPlayerLeftGame(p);

 p->setGame(nullptr);
 if (deleteit)
 {
   delete p;
 }

 return result;
}

bool KGame::inactivatePlayer(KPlayer* player)
{
 if (!player)
 {
   return false;
 }
 qCDebug(GAMES_PRIVATE_KGAME) << "Inactivate player" << player->id();

 if (policy()==PolicyLocal || policy()==PolicyDirty)
 {
   if ( !systemInactivatePlayer(player) )
       return false;
 }
 if (policy()==PolicyClean || policy()==PolicyDirty)
 {
   sendSystemMessage(player->id(), KGameMessage::IdInactivatePlayer);
 }

 return true;
}

bool KGame::systemInactivatePlayer(KPlayer* player)
{
 if (!player || !player->isActive())
 {
   return false;
 }
 qCDebug(GAMES_PRIVATE_KGAME) << "Inactivate player" << player->id();

 int pid=player->id();
 // Virtual players cannot be deactivated. They will be removed
 if (player->isVirtual())
 {
   systemRemovePlayer(player,true);
   return false; // don't touch player after this!
 }
 else
 {
   d->mPlayerList.removeAll(player);
   d->mInactivePlayerList.prepend(player);
   player->setActive(false);
 }
 Q_EMIT signalPlayerLeftGame(player);
 if (isAdmin())
 {
   d->mInactiveIdList.prepend(pid);
 }
 return true;
}

bool KGame::activatePlayer(KPlayer * player)
{
  if (!player)
  {
    return false;
  }
  qCDebug(GAMES_PRIVATE_KGAME) << ": activate" << player->id();
  if (policy()==PolicyLocal || policy()==PolicyDirty)
  {
      if ( !systemActivatePlayer(player) )
          return false;
  }
  if (policy()==PolicyClean || policy()==PolicyDirty )
  {
    sendSystemMessage(player->id(), KGameMessage::IdActivatePlayer);
  }
 return true;
}

bool KGame::systemActivatePlayer(KPlayer* player)
{
 if (!player || player->isActive())
 {
   return false;
 }
 qCDebug(GAMES_PRIVATE_KGAME) << ": activate" << player->id();

 d->mInactivePlayerList.removeAll(player);
 player->setActive(true);
 if ( !addPlayer(player) ) // player is gone
     return false;

 if (isAdmin())
 {
   d->mInactiveIdList.removeAll(player->id());
 }
 return true;
}

// -------------------- Properties ---------------------------

void KGame::setMaxPlayers(uint maxnumber)
{ if (isAdmin()) { d->mMaxPlayer.changeValue(maxnumber); } }

void KGame::setMinPlayers(uint minnumber)
{ if (isAdmin()) { d->mMinPlayer.changeValue(minnumber); } }

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

KGamePropertyHandler* KGame::dataHandler() const
{ return d->mProperties; }


KGame::KGamePlayerList* KGame::inactivePlayerList()
{ return &d->mInactivePlayerList; }

const KGame::KGamePlayerList* KGame::inactivePlayerList() const
{ return &d->mInactivePlayerList; }

KGame::KGamePlayerList* KGame::playerList()
{ return &d->mPlayerList; }

const KGame::KGamePlayerList* KGame::playerList() const
{ return &d->mPlayerList; }

#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
KRandomSequence* KGame::random() const
{ return d->mRandom; }
#endif

bool KGame::sendPlayerInput(QDataStream &msg, KPlayer *player, quint32 sender)
{
 if (!player)
 {
   qCCritical(GAMES_PRIVATE_KGAME) << ": NULL player";
   return false;
 }
 if (!isRunning())
 {
   qCCritical(GAMES_PRIVATE_KGAME) << ": game not running";
   return false;
 }

 qCDebug(GAMES_PRIVATE_KGAME) << ": transmitting playerInput over network";
 sendSystemMessage(msg, (int)KGameMessage::IdPlayerInput, player->id(), sender);
 return true;
}

bool KGame::systemPlayerInput(QDataStream &msg, KPlayer *player, quint32 sender)
{
 if (!player)
 {
   qCCritical(GAMES_PRIVATE_KGAME) << ": NULL player";
   return false;
 }
 if (!isRunning())
 {
   qCCritical(GAMES_PRIVATE_KGAME) << ": game not running";
   return false;
 }
 qCDebug(GAMES_PRIVATE_KGAME) << "KGame: Got playerInput from messageServer... sender:" << sender;
 if (playerInput(msg,player))
 {
   playerInputFinished(player);
 }
 else
 {
   qCDebug(GAMES_PRIVATE_KGAME) <<": switching off player input";
   // TODO: (MH 03-2003): We need an return option from playerInput so that
   // the player's is not automatically disabled here
   if (!player->asyncInput())
   {
     player->setTurn(false); // in turn based games we have to switch off input now
   }
 }
 return true;
}


KPlayer * KGame::playerInputFinished(KPlayer *player)
{
    if ( !player )
        return nullptr;

 qCDebug(GAMES_PRIVATE_KGAME) <<"player input finished for "<<player->id();
 // Check for game over and if not allow the next player to move
 int gameOver = 0;
 if (gameSequence())
 {
   gameSequence()->setCurrentPlayer(player);
 }
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(3, 2)
 gameOver = checkGameOver(player);
#else
 gameOver = gameSequence()->checkGameOver(player);
#endif
if (gameOver!=0)
 {
     player->setTurn(false);
     setGameStatus(End);
     Q_EMIT signalGameOver(gameOver,player,this);
 }
 else if (!player->asyncInput())
 {
   player->setTurn(false); // in turn based games we have to switch off input now
   if (gameSequence())
   {
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(3, 2)
     QTimer::singleShot(0,this,&KGame::prepareNext);
#else
     KGameSequence* gameSequence = this->gameSequence();
     QTimer::singleShot(0, gameSequence, [gameSequence]() {
         gameSequence->nextPlayer(gameSequence->currentPlayer());
    });
#endif
   }
 }
 return player;
}

#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(3, 2)
// Per default we do not do anything
int KGame::checkGameOver(KPlayer *player)
{
 if (gameSequence())
 {
   return gameSequence()->checkGameOver(player);
 }
 return 0;
}
#endif

void KGame::setGameSequence(KGameSequence* sequence)
{
 delete d->mGameSequence;
 d->mGameSequence = sequence;
 if (d->mGameSequence)
 {
   d->mGameSequence->setGame(this);
 }
}

KGameSequence* KGame::gameSequence() const
{
  return d->mGameSequence;
}

#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(3, 2)
void KGame::prepareNext()
{
 if (gameSequence())
 {
   // we don't call gameSequence->nextPlayer() to keep old code working
   nextPlayer(gameSequence()->currentPlayer());
 }
}
#endif

#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(3, 2)
KPlayer *KGame::nextPlayer(KPlayer *last,bool exclusive)
{
 if (gameSequence())
 {
   return gameSequence()->nextPlayer(last, exclusive);
 }
 return nullptr;
}
#endif

void KGame::setGameStatus(int status)
{
 qCDebug(GAMES_PRIVATE_KGAME) << ": GAMESTATUS CHANGED  to" << status;
 if (status==(int)Run && playerCount()<minPlayers())
 {
   qCDebug(GAMES_PRIVATE_KGAME) << ": not enough players, pausing game\n";
   status=Pause;
 }
 d->mGameStatus = status;
}

void KGame::networkTransmission(QDataStream &stream, int msgid, quint32 receiver, quint32 sender, quint32 /*clientID*/)
{//clientID is unused
 // message targets a playerobject. If we find it we forward the message to the
 // player. Otherwise we proceed here and hope the best that the user processes
 // the message

//  qCDebug(GAMES_PRIVATE_KGAME) << ": we="<<(int)gameId()<<" id="<<msgid<<" recv=" << receiver << "sender=" << sender;


 // *first* notice the game that something has changed - so no return prevents
 // this
 Q_EMIT signalMessageUpdate(msgid, receiver, sender);
 if (KGameMessage::isPlayer(receiver))
 {
   //qCDebug(GAMES_PRIVATE_KGAME) << "message id" << msgid << "seems to be for a player ("<<active=p->isActive()<<" recv="<< receiver;
   KPlayer *p=findPlayer(receiver);
   if (p && p->isActive())
   {
     p->networkTransmission(stream,msgid,sender);
     return;
   }
   if (p)
   {
      qCDebug(GAMES_PRIVATE_KGAME) << "player is here but not active";
   }
   else
   {
      qCDebug(GAMES_PRIVATE_KGAME) << "no player found";
   }
 }
 // If it is not for a player it is meant for us!!!! Otherwise the
 // gamenetwork would not have passed the message to us!

 // GameProperties processed
 if (d->mProperties->processMessage(stream, msgid, sender == gameId()))
 {
//   qCDebug(GAMES_PRIVATE_KGAME) << "KGame: message taken by property - returning";
   return ;
 }

 switch(msgid)
 {
   case KGameMessage::IdSetupGame:  // Client: First step in setup game
   {
     qint16 v;
     qint32 c;
     stream >> v >> c;
     qCDebug(GAMES_PRIVATE_KGAME) << " ===================> (Client) " << ": Got IdSetupGame ==================";
     qCDebug(GAMES_PRIVATE_KGAME) << "our game id is" << gameId() << "Lib version=" << v << "App Cookie=" << c;
     // Verify identity of the network partners
     if (c!=cookie())
     {
       qCCritical(GAMES_PRIVATE_KGAME) << "IdGameSetup: Negotiate Game: cookie mismatch I'am="<<cookie()<<" master="<<c;
       sendError(KGameError::Cookie, KGameError::errCookie(cookie(), c));
       disconnect(); // disconnect from master
     }
     else if (v!=KGameMessage::version())
     {
       sendError(KGameError::Version, KGameError::errVersion(v));
       disconnect(); // disconnect from master
     }
     else
     {
       setupGame(sender);
     }
     qCDebug(GAMES_PRIVATE_KGAME) << "========== (Client) Setup game done\n";
   }
   break;
   case KGameMessage::IdSetupGameContinue:  // Master: second step in game setup
   {
     qCDebug(GAMES_PRIVATE_KGAME) << "=====>(Master) " << " - IdSetupGameContinue";
     setupGameContinue(stream, sender);
   }
   break;
   case KGameMessage::IdActivatePlayer:  // Activate Player
   {
     int id;
     stream >> id;
     qCDebug(GAMES_PRIVATE_KGAME) << "Got IdActivatePlayer id=" << id;
     if (sender!=gameId()  || policy()!=PolicyDirty)
     {
       systemActivatePlayer(findPlayer(id));
     }
   }
   break;
   case KGameMessage::IdInactivatePlayer:  // Inactivate Player
   {
     int id;
     stream >> id;
     qCDebug(GAMES_PRIVATE_KGAME) << "Got IdInactivatePlayer id=" << id;
     if (sender!=gameId()  || policy()!=PolicyDirty)
     {
       systemInactivatePlayer(findPlayer(id));
     }
   }
   break;
   case KGameMessage::IdAddPlayer:
   {
     qCDebug(GAMES_PRIVATE_KGAME) << ": Got IdAddPlayer";
     if (sender!=gameId()  || policy()!=PolicyDirty)
     {
       KPlayer *newplayer=nullptr;
       // We sent the message so the player is already available
       if (sender==gameId())
       {
          qCDebug(GAMES_PRIVATE_KGAME) << "dequeue previously added player";
          newplayer = d->mAddPlayerList.dequeue();
       }
       else
       {
         newplayer=loadPlayer(stream,true);
       }
       systemAddPlayer(newplayer);// the final, local, adding
       //systemAddPlayer(stream);
     }
   }
   break;
   case KGameMessage::IdRemovePlayer: // Client should delete player id
   {
     int id;
     stream >> id;
     qCDebug(GAMES_PRIVATE_KGAME) << ": Got IdRemovePlayer" << id;
     KPlayer *p=findPlayer(id);
     if (p)
     {
       // Otherwise the player is already removed
       if (sender!=gameId()  || policy()!=PolicyDirty)
       {
         systemRemovePlayer(p,true);
       }
     }
     else
     {
       qCWarning(GAMES_PRIVATE_KGAME) << "Cannot find player" << id;
     }
   }
   break;
   case KGameMessage::IdGameLoad:
   {
     qCDebug(GAMES_PRIVATE_KGAME) << "====> (Client) " << ": Got IdGameLoad";
     loadgame(stream,true,false);
   }
   break;
   case KGameMessage::IdGameSetupDone:
   {
     int cid;
     stream >> cid;
     qCDebug(GAMES_PRIVATE_KGAME) << "====> (CLIENT) " << ": Got IdGameSetupDone for client "
             << cid << "we are =" << gameId();
     sendSystemMessage(gameId(), KGameMessage::IdGameConnected, 0);
   }
   break;
   case KGameMessage::IdGameConnected:
   {
     int cid;
     stream >> cid;
     qCDebug(GAMES_PRIVATE_KGAME) << "====> (ALL) " << ": Got IdGameConnected for client "<< cid << "we are =" << gameId();
     Q_EMIT signalClientJoinedGame(cid,this);
   }
   break;

#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
   case KGameMessage::IdSyncRandom:  // Master forces a new random seed on us
   {
     int newseed;
     stream >> newseed;
     qCDebug(GAMES_PRIVATE_KGAME) << "CLIENT: setting random seed to" << newseed;
     d->mRandom->setSeed(newseed);
   }
   break;
#endif

   case KGameMessage::IdDisconnect:
   {
   // if we disconnect we *always* start a local game.
   // this could lead into problems if we just change the message server
     if (sender != gameId())
     {
         qCDebug(GAMES_PRIVATE_KGAME) << "client" << sender << "leaves game";
         return;
     }
     qCDebug(GAMES_PRIVATE_KGAME) << "leaving the game";
     // start a new local game
     // no other client is by default connected to this so this call should be
     // enough
     setMaster();
   }
   break;
   default:
    {
     if (msgid < KGameMessage::IdUser)
     {
       qCCritical(GAMES_PRIVATE_KGAME) << "incorrect message id" << msgid << " - emit anyway";
     }
     qCDebug(GAMES_PRIVATE_KGAME) << ": User data msgid" << msgid;
     Q_EMIT signalNetworkData(msgid - KGameMessage::IdUser,((QBuffer*)stream.device())->readAll(),receiver,sender);
   }
   break;
 }

}

// called by the IdSetupGameContinue Message - MASTER SIDE
// Here the master needs to decide which players can take part at the game
// and which will be deactivated
void KGame::setupGameContinue(QDataStream& stream, quint32 sender)
{
  KPlayer *player;
  qint32 cnt;
  int i;
  stream >> cnt;

  QList<int> inactivateIds;

  KGamePlayerList newPlayerList;
  for (i=0;i<cnt;++i)
  {
    player=loadPlayer(stream,true);
    qCDebug(GAMES_PRIVATE_KGAME) << "Master got player" << player->id() <<" rawgame=" << KGameMessage::rawGameId(player->id())  << "from sender" << sender;
    if (KGameMessage::rawGameId(player->id()) != sender)
    {
      qCCritical(GAMES_PRIVATE_KGAME) << "Client tries to add player with wrong game id - cheat possible";
    }
    else
    {
      newPlayerList.append(player);
      qCDebug(GAMES_PRIVATE_KGAME) << "newplayerlist appended" << player->id();
    }
  }

  newPlayersJoin(playerList(),&newPlayerList,inactivateIds);


  qCDebug(GAMES_PRIVATE_KGAME) << "Master calculates how many players to activate client has cnt=" << cnt;
  qCDebug(GAMES_PRIVATE_KGAME) << "The game has" << playerCount() << "active players";
  qCDebug(GAMES_PRIVATE_KGAME) << "The user deactivated "<< inactivateIds.count() << "player already";
  qCDebug(GAMES_PRIVATE_KGAME) << "MaxPlayers for this game is" << maxPlayers();

  // Do we have too many players? (After the programmer disabled some?)
  // MH: We cannot use have player here as it CHANGES in the loop
  // int havePlayers = cnt+playerCount()-inactivateIds.count();
  qCDebug(GAMES_PRIVATE_KGAME) << "havePlayers" << cnt+playerCount()-inactivateIds.count();
  while (maxPlayers() > 0 && maxPlayers() < (int)(cnt+playerCount() - inactivateIds.count()))
  {
    qCDebug(GAMES_PRIVATE_KGAME) << "  Still to deactivate "
            << (int)(cnt+playerCount()-inactivateIds.count())-(int)maxPlayers()
;
    KPlayer *currentPlayer=nullptr;
    int currentPriority=0x7fff; // MAX_UINT (16bit?) to get the maximum of the list
    // find lowest network priority which is not yet in the newPlayerList
    // do this for the new players
    for ( KGamePlayerList::iterator it = newPlayerList.begin(); it!=newPlayerList.end();++it )
    {
      KPlayer* player = *it;
      // Already in the list
      if (inactivateIds.indexOf(player->id())!=-1)
      {
        continue;
      }
      if (player->networkPriority()<currentPriority)
      {
        currentPriority=player->networkPriority();
        currentPlayer=player;
      }
    }

    // find lowest network priority which is not yet in the newPlayerList
    // Do this for the network players
    for ( KGamePlayerList::iterator it = d->mPlayerList.begin(); it!=d->mPlayerList.end();++it )
    {
      KPlayer* player = *it;
      // Already in the list
      if (inactivateIds.indexOf(player->id())!=-1)
      {
        continue;
      }
      if (player->networkPriority()<currentPriority)
      {
        currentPriority=player->networkPriority();
        currentPlayer=player;
      }
    }

    // add it to inactivateIds
    if (currentPlayer)
    {
      qCDebug(GAMES_PRIVATE_KGAME) << "Marking player" << currentPlayer->id() << "for inactivation";
      inactivateIds.append(currentPlayer->id());
    }
    else
    {
      qCCritical(GAMES_PRIVATE_KGAME) << "Couldn't find a player to deactivate. That is not so good...";
      break;
    }
  }

  qCDebug(GAMES_PRIVATE_KGAME) << "Altogether deactivated" << inactivateIds.count() << "players";

  QList<int>::Iterator it;
  for ( it = inactivateIds.begin(); it != inactivateIds.end(); ++it )
  {
    int pid=*it;
    qCDebug(GAMES_PRIVATE_KGAME) << "pid=" << pid;
  }

  // Now deactivate the network players from the inactivateId list
  //QValueList<int>::Iterator it;
  for ( it = inactivateIds.begin(); it != inactivateIds.end(); ++it )
  {
    int pid=*it;
    if (KGameMessage::rawGameId(pid) == sender)
    {
      continue; // client's player
    }
    qCDebug(GAMES_PRIVATE_KGAME) << " -> the network needs to deactivate" << pid;
    player=findPlayer(pid);
    if (player)
    {
      // We have to make REALLY sure that the player is gone. With any policy
      if (systemInactivatePlayer(player) && policy()!=PolicyLocal)
      {
        sendSystemMessage(player->id(), KGameMessage::IdInactivatePlayer);
      } else
      player = nullptr;
    }
    else
    {
      qCCritical(GAMES_PRIVATE_KGAME) << "We should deactivate a player, but cannot find it...not good.";
    }
  }

  // Now send out the player list which the client can activate
  for ( KGamePlayerList::iterator it = newPlayerList.begin(); it!=newPlayerList.end();++it )
  {
    KPlayer* player = *it;
    qCDebug(GAMES_PRIVATE_KGAME) << "newplayerlist contains" << player->id();
    // Only activate what is not in the list
    if (inactivateIds.indexOf(player->id())!=-1)
    {
      continue;
    }
    qCDebug(GAMES_PRIVATE_KGAME) << " -> the client can ******** reactivate ********  " << player->id();
    sendSystemMessage(player->id(), KGameMessage::IdActivatePlayer, sender);
  }

  // Save the game over the network
  QByteArray bufferS;
  QDataStream streamS(&bufferS,QIODevice::WriteOnly);
  // Save game over netowrk and save players
  savegame(streamS,true,true);
  sendSystemMessage(streamS,KGameMessage::IdGameLoad,sender);


  // Only to the client first , as the client will add players
  sendSystemMessage(sender, KGameMessage::IdGameSetupDone, sender);

  //Finally delete content of the newPlayerList
  qDeleteAll(newPlayerList);
  newPlayerList.clear();
}

// called by the IdSetupGame Message - CLIENT SIDE
// Client needs to prepare for network transfer
void KGame::setupGame(quint32 sender)
{
  QByteArray bufferS;
  QDataStream streamS(&bufferS,QIODevice::WriteOnly);

  // Deactivate all players
  KGamePlayerList mTmpList(d->mPlayerList); // we need copy otherwise the removal crashes
  qint32 cnt=mTmpList.count();
  qCDebug(GAMES_PRIVATE_KGAME) << "Client: playerlistcount=" << d->mPlayerList.count() << "tmplistcout=" << cnt;

  streamS << cnt;

  KGamePlayerList::iterator it = mTmpList.begin();
  KPlayer *player;
  while (it!=mTmpList.end())
  {
    player=*it;
    ++it;
    --cnt;

    if (!systemInactivatePlayer(player))
	continue; // player is gone

    // Give the new game id to all players (which are inactivated now)
    player->setId(KGameMessage::createPlayerId(player->id(),gameId()));

    // Save it for the master to decide what to do
    savePlayer(streamS,player);
  }
  if (d->mPlayerList.count() > 0 || cnt!=0)
  {
    qCWarning(GAMES_PRIVATE_KGAME) << "KGame::setupGame(): Player list is not empty! or cnt!=0=" <<cnt;
    abort();
  }

  sendSystemMessage(streamS,KGameMessage::IdSetupGameContinue,sender);
}

#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
// unused by KGame
void KGame::syncRandom()
{
 int newseed=(int)d->mRandom->getLong(65535);
 sendSystemMessage(newseed,KGameMessage::IdSyncRandom); // Broadcast
 d->mRandom->setSeed(newseed);
}
#endif

void KGame::Debug()
{
 KGameNetwork::Debug();
 qCDebug(GAMES_PRIVATE_KGAME) << "------------------- KGAME -------------------------";
 qCDebug(GAMES_PRIVATE_KGAME) << "this:          " << this;
 qCDebug(GAMES_PRIVATE_KGAME) << "uniquePlayer   " << d->mUniquePlayerNumber;
 qCDebug(GAMES_PRIVATE_KGAME) << "gameStatus     " << gameStatus();
 qCDebug(GAMES_PRIVATE_KGAME) << "MaxPlayers :   " << maxPlayers();
 qCDebug(GAMES_PRIVATE_KGAME) << "NoOfPlayers :  " << playerCount();
 qCDebug(GAMES_PRIVATE_KGAME) << "NoOfInactive:  " << d->mInactivePlayerList.count();
 qCDebug(GAMES_PRIVATE_KGAME) << "---------------------------------------------------";
}

void KGame::slotClientConnected(quint32 clientID)
{
 if (isAdmin())
 {
   negotiateNetworkGame(clientID);
 }
}

void KGame::slotServerDisconnected() // Client side
{
  qCDebug(GAMES_PRIVATE_KGAME) << "======= SERVER DISCONNECT =======";
  qCDebug(GAMES_PRIVATE_KGAME) << "+++ (CLIENT)++++++++" << ": our GameID="<<gameId();

  int oldgamestatus=gameStatus();


  KGamePlayerList removeList;
  qCDebug(GAMES_PRIVATE_KGAME) << "Playerlist of client=" << d->mPlayerList.count() << "count";
  qCDebug(GAMES_PRIVATE_KGAME) << "Inactive Playerlist of client=" << d->mInactivePlayerList.count() << "count";
  for ( KGamePlayerList::iterator it = d->mPlayerList.begin(); it!=d->mPlayerList.end();++it )
  {
    KPlayer* player = *it;
    // TODO: CHECK: id=0, could not connect to server in the first place??
    if (KGameMessage::rawGameId(player->id()) != gameId() && gameId()!=0)
    {
      qCDebug(GAMES_PRIVATE_KGAME) << "Player" << player->id() << "belongs to a removed game";
      removeList.append(player);
    }
  }

  for ( KGamePlayerList::iterator it = removeList.begin(); it!=removeList.end();++it )
  {
    KPlayer* player = *it;
    bool remove = true;
    Q_EMIT signalReplacePlayerIO(player, &remove);
    if (remove)
    {
      qCDebug(GAMES_PRIVATE_KGAME) << " ---> Removing player" << player->id();
      systemRemovePlayer(player,true); // no network necessary
    }
  }

  setMaster();
  qCDebug(GAMES_PRIVATE_KGAME) << "our game id is after setMaster" << gameId();

  KGamePlayerList mReList(d->mInactivePlayerList);
  for ( KGamePlayerList::iterator it = mReList.begin(); it!=mReList.end();++it )
  {
    KPlayer* player = *it;
    // TODO ?check for priority? Sequence should be ok
    if ((int)playerCount()<maxPlayers() || maxPlayers()<0)
    {
      systemActivatePlayer(player);
    }
  }
  qCDebug(GAMES_PRIVATE_KGAME) << "Players activated player-cnt=" << playerCount();

  for ( KGamePlayerList::iterator it = d->mPlayerList.begin(); it!=d->mPlayerList.end();++it )
  {
    KPlayer* player = *it;
    int oldid=player->id();
    d->mUniquePlayerNumber++;
    player->setId(KGameMessage::createPlayerId(d->mUniquePlayerNumber,gameId()));
    qCDebug(GAMES_PRIVATE_KGAME) << "Player id" << oldid <<" changed to" << player->id() << "as we are now local";
  }
  // TODO clear inactive lists ?
  Debug();
  for ( KGamePlayerList::iterator it = d->mPlayerList.begin(); it!=d->mPlayerList.end();++it )
  {
    KPlayer* player = *it;
    player->Debug();
  }
  qCDebug(GAMES_PRIVATE_KGAME) << "+++++++++++" << "DONE=";
  Q_EMIT signalClientLeftGame(0,oldgamestatus,this);
}

void KGame::slotClientDisconnected(quint32 clientID,bool /*broken*/) // server side
{
 qCDebug(GAMES_PRIVATE_KGAME) << "++++(SERVER)+++++++" << "clientId=" << clientID;

 int oldgamestatus=gameStatus();

 KPlayer *player;
 KGamePlayerList removeList;
 qCDebug(GAMES_PRIVATE_KGAME) << "Playerlist of client=" << d->mPlayerList.count() << "count";
 for ( KGamePlayerList::iterator it = d->mPlayerList.begin(); it!=d->mPlayerList.end();++it )
 {
   KPlayer* player = *it;
   if (KGameMessage::rawGameId(player->id())==clientID)
   {
     qCDebug(GAMES_PRIVATE_KGAME) << "Player" << player->id() << "belongs to the removed game";
     removeList.append(player);
   }
 }

 for ( KGamePlayerList::iterator it = removeList.begin(); it!=removeList.end();++it )
 {
   KPlayer* player = *it;
   // try to replace the KGameIO first
   bool remove = true;
   Q_EMIT signalReplacePlayerIO(player, &remove);
   if (remove) {
     // otherwise (no new KGameIO) remove the player
     qCDebug(GAMES_PRIVATE_KGAME) << " ---> Removing player" << player->id();
     removePlayer(player,0);
   }
 }

 // Now add inactive players - sequence should be ok
 // TODO remove players from removed game
 for (int idx=0;idx<d->mInactiveIdList.count();idx++)
 {
   int it1 = d->mInactiveIdList.at(idx);
   player = findPlayer(it1);
   if (((int)playerCount() < maxPlayers() || maxPlayers() < 0) && player && KGameMessage::rawGameId(it1) != clientID)
   {
     activatePlayer(player);
   }
 }
  Q_EMIT signalClientLeftGame(clientID,oldgamestatus,this);
}


// -------------------- Synchronization -----------------------

// this initializes a newly connected client.
// we send the number of players (including type) as well as game status and
// properties to the client. After the initialization has been completed both
// clients should have the same status (ie players, properties, etc)
void KGame::negotiateNetworkGame(quint32 clientID)
{
 qCDebug(GAMES_PRIVATE_KGAME) << "===========================" << ": clientID=" << clientID << " =========================== ";
 if (!isAdmin())
 {
   qCCritical(GAMES_PRIVATE_KGAME) << ": Serious WARNING..only gameAdmin should call this";
   return ;
 }

 QByteArray buffer;
 QDataStream streamGS(&buffer,QIODevice::WriteOnly);

 // write Game setup specific data
 //streamGS << (qint32)maxPlayers();
 //streamGS << (qint32)minPlayers();

 // send to the newly connected client *only*
 qint16 v=KGameMessage::version();
 qint32 c=cookie();
 streamGS << v << c;
 sendSystemMessage(streamGS, KGameMessage::IdSetupGame, clientID);
}

bool KGame::sendGroupMessage(const QByteArray &msg, int msgid, quint32 sender, const QString& group)
{
// AB: group must not be i18n'ed!! we should better use an id for group and use
// a groupName() for the name // FIXME

 for ( KGamePlayerList::iterator it = d->mPlayerList.begin(); it!=d->mPlayerList.end();++it )
 {
   KPlayer* player = *it;
   if (player && player->group()==group)
   {
     sendMessage(msg,msgid,player->id(), sender);
   }
 }
 return true;
}

bool KGame::sendGroupMessage(const QDataStream &msg, int msgid, quint32 sender, const QString& group)
{ return sendGroupMessage(((QBuffer*)msg.device())->buffer(), msgid, sender, group); }

bool KGame::sendGroupMessage(const QString& msg, int msgid, quint32 sender, const QString& group)
{
 QByteArray buffer;
 QDataStream stream(&buffer, QIODevice::WriteOnly);
 stream << msg;
 return sendGroupMessage(stream, msgid, sender, group);
}

bool KGame::addProperty(KGamePropertyBase* data)
{ return dataHandler()->addProperty(data); }

bool KGame::sendPlayerProperty(int msgid, QDataStream& s, quint32 playerId)
{ return sendSystemMessage(s, msgid, playerId); }

void KGame::sendProperty(int msgid, QDataStream& stream, bool* sent)
{
  bool s = sendSystemMessage(stream, msgid);
  if (s)
  {
    *sent = true;
  }
}

void KGame::emitSignal(KGamePropertyBase *me)
{
 Q_EMIT signalPropertyChanged(me,this);
}

KGamePropertyBase* KGame::findProperty(int id) const
{ return d->mProperties->find(id); }

KGame::GamePolicy KGame::policy() const
{
 return d->mPolicy;
}
void KGame::setPolicy(GamePolicy p,bool recursive)
{
 // Set KGame policy
 d->mPolicy=p;
 if (recursive)
 {
   // Set all KGame property policy
   dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p,false);

   // Set all KPLayer (active or inactive) property policy
   for ( KGamePlayerList::iterator it = d->mPlayerList.begin(); it!=d->mPlayerList.end();++it )
   {
     (*it)->dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p,false);
   }
   for ( KGamePlayerList::iterator it = d->mInactivePlayerList.begin(); it!=d->mInactivePlayerList.end();++it )
   {
     (*it)->dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p,false);
   }
 }
}

/*
 * vim: et sw=2
 */
