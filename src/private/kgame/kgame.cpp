/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgame.h"

// own
#include "kgameerror.h"
#include "kgameio.h"
#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"
#include "kgamesequence.h"
#include "kplayer.h"
#include <kdegamesprivate_kgame_logging.h>
// KF
#include <KLocalizedString>
// Qt
#include <QBuffer>
#include <QFile>
#include <QQueue>
#include <QTimer>
// Std
#include <cassert>
#include <cstdio>

#define KGAME_LOAD_COOKIE 4210

// try to place as much as possible here
// many things are *not* possible here as KGame has to use some inline function
class KGamePrivate
{
public:
    KGamePrivate()
    {
        mUniquePlayerNumber = 0;
        mPolicy = KGame::PolicyLocal;
        mGameSequence = nullptr;
    }

    int mUniquePlayerNumber;
    QQueue<KPlayer *> mAddPlayerList; // this is a list of to-be-added players. See addPlayer() docu
    KGame::GamePolicy mPolicy;
    KGameSequence *mGameSequence;

    KGamePropertyHandler *mProperties;

    // player lists
    KGame::KGamePlayerList mPlayerList;
    KGame::KGamePlayerList mInactivePlayerList;

    // KGamePropertys
    KGamePropertyInt mMaxPlayer;
    KGamePropertyUInt mMinPlayer;
    KGamePropertyInt mGameStatus; // Game running?
    QList<int> mInactiveIdList;
};

// ------------------- GAME CLASS --------------------------
KGame::KGame(int cookie, QObject *parent)
    : KGameNetwork(cookie, parent)
    , d(new KGamePrivate)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << " - " << this << ", sizeof(KGame)=" << sizeof(KGame);

    d->mProperties = new KGamePropertyHandler(this);

    d->mProperties->registerHandler(KGameMessage::IdGameProperty, this, SLOT(sendProperty(int, QDataStream &, bool *)), SLOT(emitSignal(KGamePropertyBase *)));
    d->mMaxPlayer.registerData(KGamePropertyBase::IdMaxPlayer, this, i18n("MaxPlayers"));
    d->mMaxPlayer.setLocal(-1); // Infinite
    d->mMinPlayer.registerData(KGamePropertyBase::IdMinPlayer, this, i18n("MinPlayers"));
    d->mMinPlayer.setLocal(0); // Always ok
    d->mGameStatus.registerData(KGamePropertyBase::IdGameStatus, this, i18n("GameStatus"));
    d->mGameStatus.setLocal(Init);
    // d->mUniquePlayerNumber = 0;

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
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG);
    // Debug();
    reset();
    delete d->mGameSequence;
    delete d;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "done";
}

bool KGame::reset()
{
    deletePlayers();
    deleteInactivePlayers();
    return true;
}

void KGame::deletePlayers()
{
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) ;
    // Bugs 303142 and 305000. KPlayer destructor removes
    // player from the list and makes iterators invalid.
    // qDeleteAll crashes in that case.
    while (!d->mPlayerList.isEmpty()) {
        delete d->mPlayerList.takeFirst();
    }
    // qDeleteAll(d->mPlayerList);
    // NOTE by majewsky: An earlier implementation copied the mPlayerList before
    // deleting the elements with a takeFirst loop. I therefore chose not to clear()
    // the list in order not to break anything. The old code had the following
    // comment: "in case of PolicyClean player=d->mPlayerList.first() is infinite"
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "done";
}

void KGame::deleteInactivePlayers()
{
    qDeleteAll(d->mInactivePlayerList);
    d->mInactivePlayerList.clear();
}

bool KGame::load(const QString &filename, bool reset)
{
    if (filename.isEmpty()) {
        return false;
    }
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    QDataStream s(&f);
    load(s, reset);
    f.close();
    return true;
}

bool KGame::load(QDataStream &stream, bool reset)
{
    return loadgame(stream, false, reset);
}

bool KGame::loadgame(QDataStream &stream, bool network, bool resetgame)
{
    // Load Game Data

    // internal data
    qint32 c;
    stream >> c; // cookie

    if (c != cookie()) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "Trying to load different game version we=" << cookie() << "saved=" << c;
        bool result = false;
        Q_EMIT signalLoadError(stream, network, (int)c, result);
        return result;
    }
    if (resetgame)
        reset();

    uint i;
    stream >> i;
    // setPolicy((GamePolicy)i);

    stream >> d->mUniquePlayerNumber;

    if (gameSequence()) {
        gameSequence()->setCurrentPlayer(nullptr); // TODO !!!
    }

    // Switch off the direct emitting of signals while
    // loading properties. This can cause inconsistencies
    // otherwise if a property emits and this emit accesses
    // a property not yet loaded
    // Note we have to have this external locking to prevent the games unlocking
    // to access the players
    dataHandler()->lockDirectEmit();

    for (KGamePlayerList::iterator it = playerList()->begin(); it != playerList()->end(); ++it) {
        (*it)->dataHandler()->lockDirectEmit();
        // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Player "<<player->id() << "to indirect emit";
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
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Loading KGame" << playercount << "KPlayer objects";
    for (i = 0; i < playercount; ++i) {
        KPlayer *newplayer = loadPlayer(stream, network);
        systemAddPlayer(newplayer);
    }

    qint16 cookie;
    stream >> cookie;
    if (cookie == KGAME_LOAD_COOKIE) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "   Game loaded properly";
    } else {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "   Game loading error. probably format error";
    }

    // Switch back on the direct emitting of signals and emit the
    // queued signals for players.
    // Note we habe to have this external locking to prevent the games unlocking
    // to access the players
    for (KGamePlayerList::iterator it = playerList()->begin(); it != playerList()->end(); ++it) {
        (*it)->dataHandler()->unlockDirectEmit();
        // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Player "<<player->id() << "to direct emit";
    }

    Q_EMIT signalLoad(stream);
    return true;
}

bool KGame::save(const QString &filename, bool saveplayers)
{
    if (filename.isEmpty()) {
        return false;
    }
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }
    QDataStream s(&f);
    save(s, saveplayers);
    f.close();
    return true;
}

bool KGame::save(QDataStream &stream, bool saveplayers)
{
    return savegame(stream, false, saveplayers);
}

bool KGame::savegame(QDataStream &stream, bool /*network*/, bool saveplayers)
{
    // Save Game Data

    // internal variables
    qint32 c = cookie();
    stream << c;

    uint p = (uint)policy();
    stream << p;
    stream << d->mUniquePlayerNumber;

    // Properties
    dataHandler()->save(stream);

    // Save all data that need to be saved *before* the players are saved
    Q_EMIT signalSavePrePlayers(stream);

    if (saveplayers) {
        savePlayers(stream, playerList());
    } else {
        stream << (uint)0; // no players saved
    }

    stream << (qint16)KGAME_LOAD_COOKIE;

    Q_EMIT signalSave(stream);
    return true;
}

void KGame::savePlayer(QDataStream &stream, KPlayer *p)
{
    // this could be in KGameMessage as well
    stream << (qint32)p->rtti();
    stream << (qint32)p->id();
    stream << (qint32)p->calcIOValue();
    p->save(stream);
}

void KGame::savePlayers(QDataStream &stream, KGamePlayerList *list)
{
    if (!list) {
        list = playerList();
    }

    qint32 cnt = list->count();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Saving KGame" << cnt << "KPlayer objects";
    stream << cnt;

    for (KGamePlayerList::iterator it = playerList()->begin(); it != playerList()->end(); ++it) {
        savePlayer(stream, *it);
    }
}

KPlayer *KGame::createPlayer(int /*rtti*/, int /*io*/, bool /*isvirtual*/)
{
    qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "   No user defined player created. Creating default KPlayer. This crashes if you have overwritten KPlayer!!!! ";
    return new KPlayer;
}
KPlayer *KGame::loadPlayer(QDataStream &stream, bool isvirtual)
{
    qint32 rtti, id, iovalue;
    stream >> rtti >> id >> iovalue;
    KPlayer *newplayer = findPlayer(id);
    if (!newplayer) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Player " << id << "not found...asking user to create one";
        newplayer = createPlayer(rtti, iovalue, isvirtual);
        // Q_EMIT signalCreatePlayer(newplayer,rtti,iovalue,isvirtual,this);
    }
    /*
    if (!newplayer)
    {
      qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "   No user defined player created. Creating default KPlayer. This crashes if you have overwritten KPlayer!!!! ";
      newplayer=new KPlayer;
    }
    else
    {
      qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "   USER Player" << newplayer << "done player->rtti=" << newplayer->rtti() << "rtti=" << rtti;
    }
    */
    newplayer->load(stream);
    if (isvirtual) {
        newplayer->setVirtual(true);
    }
    return newplayer;
}

// ----------------- Player handling -----------------------

KPlayer *KGame::findPlayer(quint32 id) const
{
    for (KGamePlayerList::iterator it = d->mPlayerList.begin(); it != d->mPlayerList.end(); ++it) {
        if ((*it)->id() == id) {
            return *it;
        }
    }
    for (KGamePlayerList::iterator it = d->mInactivePlayerList.begin(); it != d->mInactivePlayerList.end(); ++it) {
        if ((*it)->id() == id) {
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
bool KGame::addPlayer(KPlayer *newplayer)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ":  "
                                       << "; maxPlayers=" << maxPlayers() << "playerCount=" << playerCount();
    if (!newplayer) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "trying to add NULL player in KGame::addPlayer()";
        return false;
    }

    if (maxPlayers() >= 0 && (int)playerCount() >= maxPlayers()) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "cannot add more than" << maxPlayers() << "players - deleting...";
        return false;
    }

    if (newplayer->id() == 0) {
        d->mUniquePlayerNumber++;
        newplayer->setId(KGameMessage::createPlayerId(d->mUniquePlayerNumber, gameId()));
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "NEW!!! player" << newplayer << "now has id" << newplayer->id();
    } else {
        // this could happen in games which use their own ID management by certain
        // reasons. that is NOT recommended
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "player" << newplayer << "already has an id:" << newplayer->id();
    }

    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    // We distinguish here what policy we have
    if (policy() == PolicyLocal || policy() == PolicyDirty) {
        if (!systemAddPlayer(newplayer))
            return false;
    }
    if (policy() == PolicyClean || policy() == PolicyDirty) {
        savePlayer(stream, newplayer);
        // Store the player for delayed clean adding
        if (policy() == PolicyClean) {
            d->mAddPlayerList.enqueue(newplayer);
        }
        sendSystemMessage(stream, (int)KGameMessage::IdAddPlayer, 0);
    }
    return true;
}

bool KGame::systemAddPlayer(KPlayer *newplayer)
{
    if (!newplayer) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "trying to add NULL player in KGame::systemAddPlayer()";
        return false;
    }
    if (newplayer->id() == 0) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "player" << newplayer << "has no ID";
    }

    if (findPlayer(newplayer->id())) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "ERROR: Double adding player !!!!! NOT GOOD !!!!!! " << newplayer->id() << "...I delete it again";
        delete newplayer;
        return false;
    } else {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Trying to add player" << newplayer << " maxPlayers=" << maxPlayers() << " playerCount=" << playerCount();
        // Add the player to the game
        d->mPlayerList.append(newplayer);
        newplayer->setGame(this);
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Player: isVirtual=" << newplayer->isVirtual();
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "        id=" << newplayer->id() << "  #Players=" << d->mPlayerList.count() << "added" << newplayer
                                           << "  (virtual=" << newplayer->isVirtual() << ")";
        Q_EMIT signalPlayerJoinedGame(newplayer);
    }
    return true;
}

// Called by the KPlayer destructor
void KGame::playerDeleted(KPlayer *player)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": id (" << player->id() << ") to be removed" << player;

    if (policy() == PolicyLocal || policy() == PolicyDirty) {
        systemRemovePlayer(player, false);
    }
    if (policy() == PolicyClean || policy() == PolicyDirty) {
        if (!player->isVirtual()) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": sending IdRemovePlayer " << player->id();
            sendSystemMessage(player->id(), KGameMessage::IdRemovePlayer, 0);
        }
    }
}

bool KGame::removePlayer(KPlayer *player, quint32 receiver)
{ // transmit to all clients, or to receiver only
    if (!player) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "trying to remove NULL player in KGame::removePlayer(  )";
        return false;
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": id (" << player->id() << ") to be removed" << player;

    if (policy() == PolicyLocal || policy() == PolicyDirty) {
        systemRemovePlayer(player, true);
        return true; // player is gone
    }
    if (policy() == PolicyClean || policy() == PolicyDirty) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": sending IdRemovePlayer " << player->id();
        sendSystemMessage(player->id(), KGameMessage::IdRemovePlayer, receiver);
    }
    return true;
    // we will receive the message in networkTransmission()
}

void KGame::systemRemovePlayer(KPlayer *player, bool deleteit)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG);
    if (!player) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "cannot remove NULL player";
        return;
    }
    systemRemove(player, deleteit);

    if (gameStatus() == (int)Run && playerCount() < minPlayers()) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": not enough players, PAUSING game\n";
        setGameStatus(Pause);
    }
}

bool KGame::systemRemove(KPlayer *p, bool deleteit)
{
    if (!p) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "cannot remove NULL player";
        return false;
    }
    bool result;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Player (" << p->id() << ") to be removed" << p;

    if (d->mPlayerList.count() == 0) {
        result = false;
    } else {
        result = d->mPlayerList.removeAll(p);
    }

    Q_EMIT signalPlayerLeftGame(p);

    p->setGame(nullptr);
    if (deleteit) {
        delete p;
    }

    return result;
}

bool KGame::inactivatePlayer(KPlayer *player)
{
    if (!player) {
        return false;
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Inactivate player" << player->id();

    if (policy() == PolicyLocal || policy() == PolicyDirty) {
        if (!systemInactivatePlayer(player))
            return false;
    }
    if (policy() == PolicyClean || policy() == PolicyDirty) {
        sendSystemMessage(player->id(), KGameMessage::IdInactivatePlayer);
    }

    return true;
}

bool KGame::systemInactivatePlayer(KPlayer *player)
{
    if (!player || !player->isActive()) {
        return false;
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Inactivate player" << player->id();

    int pid = player->id();
    // Virtual players cannot be deactivated. They will be removed
    if (player->isVirtual()) {
        systemRemovePlayer(player, true);
        return false; // don't touch player after this!
    } else {
        d->mPlayerList.removeAll(player);
        d->mInactivePlayerList.prepend(player);
        player->setActive(false);
    }
    Q_EMIT signalPlayerLeftGame(player);
    if (isAdmin()) {
        d->mInactiveIdList.prepend(pid);
    }
    return true;
}

bool KGame::activatePlayer(KPlayer *player)
{
    if (!player) {
        return false;
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": activate" << player->id();
    if (policy() == PolicyLocal || policy() == PolicyDirty) {
        if (!systemActivatePlayer(player))
            return false;
    }
    if (policy() == PolicyClean || policy() == PolicyDirty) {
        sendSystemMessage(player->id(), KGameMessage::IdActivatePlayer);
    }
    return true;
}

bool KGame::systemActivatePlayer(KPlayer *player)
{
    if (!player || player->isActive()) {
        return false;
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": activate" << player->id();

    d->mInactivePlayerList.removeAll(player);
    player->setActive(true);
    if (!addPlayer(player)) // player is gone
        return false;

    if (isAdmin()) {
        d->mInactiveIdList.removeAll(player->id());
    }
    return true;
}

// -------------------- Properties ---------------------------

void KGame::setMaxPlayers(uint maxnumber)
{
    if (isAdmin()) {
        d->mMaxPlayer.changeValue(maxnumber);
    }
}

void KGame::setMinPlayers(uint minnumber)
{
    if (isAdmin()) {
        d->mMinPlayer.changeValue(minnumber);
    }
}

uint KGame::minPlayers() const
{
    return d->mMinPlayer.value();
}

int KGame::maxPlayers() const
{
    return d->mMaxPlayer.value();
}

uint KGame::playerCount() const
{
    return d->mPlayerList.count();
}

int KGame::gameStatus() const
{
    return d->mGameStatus.value();
}

bool KGame::isRunning() const
{
    return d->mGameStatus.value() == Run;
}

KGamePropertyHandler *KGame::dataHandler() const
{
    return d->mProperties;
}

KGame::KGamePlayerList *KGame::inactivePlayerList()
{
    return &d->mInactivePlayerList;
}

const KGame::KGamePlayerList *KGame::inactivePlayerList() const
{
    return &d->mInactivePlayerList;
}

KGame::KGamePlayerList *KGame::playerList()
{
    return &d->mPlayerList;
}

const KGame::KGamePlayerList *KGame::playerList() const
{
    return &d->mPlayerList;
}

bool KGame::sendPlayerInput(QDataStream &msg, KPlayer *player, quint32 sender)
{
    if (!player) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": NULL player";
        return false;
    }
    if (!isRunning()) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": game not running";
        return false;
    }

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": transmitting playerInput over network";
    sendSystemMessage(msg, (int)KGameMessage::IdPlayerInput, player->id(), sender);
    return true;
}

bool KGame::systemPlayerInput(QDataStream &msg, KPlayer *player, quint32 sender)
{
    if (!player) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": NULL player";
        return false;
    }
    if (!isRunning()) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": game not running";
        return false;
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "KGame: Got playerInput from messageServer... sender:" << sender;
    if (playerInput(msg, player)) {
        playerInputFinished(player);
    } else {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": switching off player input";
        // TODO: (MH 03-2003): We need an return option from playerInput so that
        // the player's is not automatically disabled here
        if (!player->asyncInput()) {
            player->setTurn(false); // in turn based games we have to switch off input now
        }
    }
    return true;
}

KPlayer *KGame::playerInputFinished(KPlayer *player)
{
    if (!player)
        return nullptr;

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "player input finished for " << player->id();
    // Check for game over and if not allow the next player to move
    int gameOver = 0;
    if (gameSequence()) {
        gameSequence()->setCurrentPlayer(player);
    }
    gameOver = gameSequence()->checkGameOver(player);
    if (gameOver != 0) {
        player->setTurn(false);
        setGameStatus(End);
        Q_EMIT signalGameOver(gameOver, player, this);
    } else if (!player->asyncInput()) {
        player->setTurn(false); // in turn based games we have to switch off input now
        if (gameSequence()) {
            KGameSequence *gameSequence = this->gameSequence();
            QTimer::singleShot(0, gameSequence, [gameSequence]() {
                gameSequence->nextPlayer(gameSequence->currentPlayer());
            });
        }
    }
    return player;
}

void KGame::setGameSequence(KGameSequence *sequence)
{
    delete d->mGameSequence;
    d->mGameSequence = sequence;
    if (d->mGameSequence) {
        d->mGameSequence->setGame(this);
    }
}

KGameSequence *KGame::gameSequence() const
{
    return d->mGameSequence;
}

void KGame::setGameStatus(int status)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": GAMESTATUS CHANGED  to" << status;
    if (status == (int)Run && playerCount() < minPlayers()) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": not enough players, pausing game\n";
        status = Pause;
    }
    d->mGameStatus = status;
}

void KGame::networkTransmission(QDataStream &stream, int msgid, quint32 receiver, quint32 sender, quint32 /*clientID*/)
{ // clientID is unused
    // message targets a playerobject. If we find it we forward the message to the
    // player. Otherwise we proceed here and hope the best that the user processes
    // the message

    //  qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": we="<<(int)gameId()<<" id="<<msgid<<" recv=" << receiver << "sender=" << sender;

    // *first* notice the game that something has changed - so no return prevents
    // this
    Q_EMIT signalMessageUpdate(msgid, receiver, sender);
    if (KGameMessage::isPlayer(receiver)) {
        // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "message id" << msgid << "seems to be for a player ("<<active=p->isActive()<<" recv="<< receiver;
        KPlayer *p = findPlayer(receiver);
        if (p && p->isActive()) {
            p->networkTransmission(stream, msgid, sender);
            return;
        }
        if (p) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "player is here but not active";
        } else {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "no player found";
        }
    }
    // If it is not for a player it is meant for us!!!! Otherwise the
    // gamenetwork would not have passed the message to us!

    // GameProperties processed
    if (d->mProperties->processMessage(stream, msgid, sender == gameId())) {
        //   qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "KGame: message taken by property - returning";
        return;
    }

    switch (msgid) {
    case KGameMessage::IdSetupGame: // Client: First step in setup game
    {
        qint16 v;
        qint32 c;
        stream >> v >> c;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << " ===================> (Client) "
                                           << ": Got IdSetupGame ==================";
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "our game id is" << gameId() << "Lib version=" << v << "App Cookie=" << c;
        // Verify identity of the network partners
        if (c != cookie()) {
            qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "IdGameSetup: Negotiate Game: cookie mismatch I'am=" << cookie() << " master=" << c;
            sendError(KGameError::Cookie, KGameError::errCookie(cookie(), c));
            disconnect(); // disconnect from master
        } else if (v != KGameMessage::version()) {
            sendError(KGameError::Version, KGameError::errVersion(v));
            disconnect(); // disconnect from master
        } else {
            setupGame(sender);
        }
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "========== (Client) Setup game done\n";
    } break;
    case KGameMessage::IdSetupGameContinue: // Master: second step in game setup
    {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "=====>(Master) "
                                           << " - IdSetupGameContinue";
        setupGameContinue(stream, sender);
    } break;
    case KGameMessage::IdActivatePlayer: // Activate Player
    {
        int id;
        stream >> id;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Got IdActivatePlayer id=" << id;
        if (sender != gameId() || policy() != PolicyDirty) {
            systemActivatePlayer(findPlayer(id));
        }
    } break;
    case KGameMessage::IdInactivatePlayer: // Inactivate Player
    {
        int id;
        stream >> id;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Got IdInactivatePlayer id=" << id;
        if (sender != gameId() || policy() != PolicyDirty) {
            systemInactivatePlayer(findPlayer(id));
        }
    } break;
    case KGameMessage::IdAddPlayer: {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Got IdAddPlayer";
        if (sender != gameId() || policy() != PolicyDirty) {
            KPlayer *newplayer = nullptr;
            // We sent the message so the player is already available
            if (sender == gameId()) {
                qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "dequeue previously added player";
                newplayer = d->mAddPlayerList.dequeue();
            } else {
                newplayer = loadPlayer(stream, true);
            }
            systemAddPlayer(newplayer); // the final, local, adding
            // systemAddPlayer(stream);
        }
    } break;
    case KGameMessage::IdRemovePlayer: // Client should delete player id
    {
        int id;
        stream >> id;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Got IdRemovePlayer" << id;
        KPlayer *p = findPlayer(id);
        if (p) {
            // Otherwise the player is already removed
            if (sender != gameId() || policy() != PolicyDirty) {
                systemRemovePlayer(p, true);
            }
        } else {
            qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "Cannot find player" << id;
        }
    } break;
    case KGameMessage::IdGameLoad: {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "====> (Client) "
                                           << ": Got IdGameLoad";
        loadgame(stream, true, false);
    } break;
    case KGameMessage::IdGameSetupDone: {
        int cid;
        stream >> cid;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "====> (CLIENT) "
                                           << ": Got IdGameSetupDone for client " << cid << "we are =" << gameId();
        sendSystemMessage(gameId(), KGameMessage::IdGameConnected, 0);
    } break;
    case KGameMessage::IdGameConnected: {
        int cid;
        stream >> cid;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "====> (ALL) "
                                           << ": Got IdGameConnected for client " << cid << "we are =" << gameId();
        Q_EMIT signalClientJoinedGame(cid, this);
    } break;

    case KGameMessage::IdDisconnect: {
        // if we disconnect we *always* start a local game.
        // this could lead into problems if we just change the message server
        if (sender != gameId()) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "client" << sender << "leaves game";
            return;
        }
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "leaving the game";
        // start a new local game
        // no other client is by default connected to this so this call should be
        // enough
        setMaster();
    } break;
    default: {
        if (msgid < KGameMessage::IdUser) {
            qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "incorrect message id" << msgid << " - emit anyway";
        }
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": User data msgid" << msgid;
        Q_EMIT signalNetworkData(msgid - KGameMessage::IdUser, ((QBuffer *)stream.device())->readAll(), receiver, sender);
    } break;
    }
}

// called by the IdSetupGameContinue Message - MASTER SIDE
// Here the master needs to decide which players can take part at the game
// and which will be deactivated
void KGame::setupGameContinue(QDataStream &stream, quint32 sender)
{
    KPlayer *player;
    qint32 cnt;
    int i;
    stream >> cnt;

    QList<int> inactivateIds;

    KGamePlayerList newPlayerList;
    for (i = 0; i < cnt; ++i) {
        player = loadPlayer(stream, true);
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Master got player" << player->id() << " rawgame=" << KGameMessage::rawGameId(player->id()) << "from sender"
                                           << sender;
        if (KGameMessage::rawGameId(player->id()) != sender) {
            qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "Client tries to add player with wrong game id - cheat possible";
        } else {
            newPlayerList.append(player);
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "newplayerlist appended" << player->id();
        }
    }

    newPlayersJoin(playerList(), &newPlayerList, inactivateIds);

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Master calculates how many players to activate client has cnt=" << cnt;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "The game has" << playerCount() << "active players";
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "The user deactivated " << inactivateIds.count() << "player already";
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "MaxPlayers for this game is" << maxPlayers();

    // Do we have too many players? (After the programmer disabled some?)
    // MH: We cannot use have player here as it CHANGES in the loop
    // int havePlayers = cnt+playerCount()-inactivateIds.count();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "havePlayers" << cnt + playerCount() - inactivateIds.count();
    while (maxPlayers() > 0 && maxPlayers() < (int)(cnt + playerCount() - inactivateIds.count())) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "  Still to deactivate " << (int)(cnt + playerCount() - inactivateIds.count()) - (int)maxPlayers();
        KPlayer *currentPlayer = nullptr;
        int currentPriority = 0x7fff; // MAX_UINT (16bit?) to get the maximum of the list
        // find lowest network priority which is not yet in the newPlayerList
        // do this for the new players
        for (KGamePlayerList::iterator it = newPlayerList.begin(); it != newPlayerList.end(); ++it) {
            KPlayer *player = *it;
            // Already in the list
            if (inactivateIds.indexOf(player->id()) != -1) {
                continue;
            }
            if (player->networkPriority() < currentPriority) {
                currentPriority = player->networkPriority();
                currentPlayer = player;
            }
        }

        // find lowest network priority which is not yet in the newPlayerList
        // Do this for the network players
        for (KGamePlayerList::iterator it = d->mPlayerList.begin(); it != d->mPlayerList.end(); ++it) {
            KPlayer *player = *it;
            // Already in the list
            if (inactivateIds.indexOf(player->id()) != -1) {
                continue;
            }
            if (player->networkPriority() < currentPriority) {
                currentPriority = player->networkPriority();
                currentPlayer = player;
            }
        }

        // add it to inactivateIds
        if (currentPlayer) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Marking player" << currentPlayer->id() << "for inactivation";
            inactivateIds.append(currentPlayer->id());
        } else {
            qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "Couldn't find a player to deactivate. That is not so good...";
            break;
        }
    }

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Altogether deactivated" << inactivateIds.count() << "players";

    QList<int>::Iterator it;
    for (it = inactivateIds.begin(); it != inactivateIds.end(); ++it) {
        int pid = *it;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "pid=" << pid;
    }

    // Now deactivate the network players from the inactivateId list
    // QValueList<int>::Iterator it;
    for (it = inactivateIds.begin(); it != inactivateIds.end(); ++it) {
        int pid = *it;
        if (KGameMessage::rawGameId(pid) == sender) {
            continue; // client's player
        }
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << " -> the network needs to deactivate" << pid;
        player = findPlayer(pid);
        if (player) {
            // We have to make REALLY sure that the player is gone. With any policy
            if (systemInactivatePlayer(player) && policy() != PolicyLocal) {
                sendSystemMessage(player->id(), KGameMessage::IdInactivatePlayer);
            } else
                player = nullptr;
        } else {
            qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "We should deactivate a player, but cannot find it...not good.";
        }
    }

    // Now send out the player list which the client can activate
    for (KGamePlayerList::iterator it = newPlayerList.begin(); it != newPlayerList.end(); ++it) {
        KPlayer *player = *it;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "newplayerlist contains" << player->id();
        // Only activate what is not in the list
        if (inactivateIds.indexOf(player->id()) != -1) {
            continue;
        }
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << " -> the client can ******** reactivate ********  " << player->id();
        sendSystemMessage(player->id(), KGameMessage::IdActivatePlayer, sender);
    }

    // Save the game over the network
    QByteArray bufferS;
    QDataStream streamS(&bufferS, QIODevice::WriteOnly);
    // Save game over netowrk and save players
    savegame(streamS, true, true);
    sendSystemMessage(streamS, KGameMessage::IdGameLoad, sender);

    // Only to the client first , as the client will add players
    sendSystemMessage(sender, KGameMessage::IdGameSetupDone, sender);

    // Finally delete content of the newPlayerList
    qDeleteAll(newPlayerList);
    newPlayerList.clear();
}

// called by the IdSetupGame Message - CLIENT SIDE
// Client needs to prepare for network transfer
void KGame::setupGame(quint32 sender)
{
    QByteArray bufferS;
    QDataStream streamS(&bufferS, QIODevice::WriteOnly);

    // Deactivate all players
    KGamePlayerList mTmpList(d->mPlayerList); // we need copy otherwise the removal crashes
    qint32 cnt = mTmpList.count();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Client: playerlistcount=" << d->mPlayerList.count() << "tmplistcout=" << cnt;

    streamS << cnt;

    KGamePlayerList::iterator it = mTmpList.begin();
    KPlayer *player;
    while (it != mTmpList.end()) {
        player = *it;
        ++it;
        --cnt;

        if (!systemInactivatePlayer(player))
            continue; // player is gone

        // Give the new game id to all players (which are inactivated now)
        player->setId(KGameMessage::createPlayerId(player->id(), gameId()));

        // Save it for the master to decide what to do
        savePlayer(streamS, player);
    }
    if (d->mPlayerList.count() > 0 || cnt != 0) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "KGame::setupGame(): Player list is not empty! or cnt!=0=" << cnt;
        abort();
    }

    sendSystemMessage(streamS, KGameMessage::IdSetupGameContinue, sender);
}

void KGame::Debug()
{
    KGameNetwork::Debug();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "------------------- KGAME -------------------------";
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "this:          " << this;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "uniquePlayer   " << d->mUniquePlayerNumber;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "gameStatus     " << gameStatus();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "MaxPlayers :   " << maxPlayers();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "NoOfPlayers :  " << playerCount();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "NoOfInactive:  " << d->mInactivePlayerList.count();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "---------------------------------------------------";
}

void KGame::slotClientConnected(quint32 clientID)
{
    if (isAdmin()) {
        negotiateNetworkGame(clientID);
    }
}

void KGame::slotServerDisconnected() // Client side
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "======= SERVER DISCONNECT =======";
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "+++ (CLIENT)++++++++"
                                       << ": our GameID=" << gameId();

    int oldgamestatus = gameStatus();

    KGamePlayerList removeList;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Playerlist of client=" << d->mPlayerList.count() << "count";
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Inactive Playerlist of client=" << d->mInactivePlayerList.count() << "count";
    for (KGamePlayerList::iterator it = d->mPlayerList.begin(); it != d->mPlayerList.end(); ++it) {
        KPlayer *player = *it;
        // TODO: CHECK: id=0, could not connect to server in the first place??
        if (KGameMessage::rawGameId(player->id()) != gameId() && gameId() != 0) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Player" << player->id() << "belongs to a removed game";
            removeList.append(player);
        }
    }

    for (KGamePlayerList::iterator it = removeList.begin(); it != removeList.end(); ++it) {
        KPlayer *player = *it;
        bool remove = true;
        Q_EMIT signalReplacePlayerIO(player, &remove);
        if (remove) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << " ---> Removing player" << player->id();
            systemRemovePlayer(player, true); // no network necessary
        }
    }

    setMaster();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "our game id is after setMaster" << gameId();

    KGamePlayerList mReList(d->mInactivePlayerList);
    for (KGamePlayerList::iterator it = mReList.begin(); it != mReList.end(); ++it) {
        KPlayer *player = *it;
        // TODO ?check for priority? Sequence should be ok
        if ((int)playerCount() < maxPlayers() || maxPlayers() < 0) {
            systemActivatePlayer(player);
        }
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Players activated player-cnt=" << playerCount();

    for (KGamePlayerList::iterator it = d->mPlayerList.begin(); it != d->mPlayerList.end(); ++it) {
        KPlayer *player = *it;
        int oldid = player->id();
        d->mUniquePlayerNumber++;
        player->setId(KGameMessage::createPlayerId(d->mUniquePlayerNumber, gameId()));
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Player id" << oldid << " changed to" << player->id() << "as we are now local";
    }
    // TODO clear inactive lists ?
    Debug();
    for (KGamePlayerList::iterator it = d->mPlayerList.begin(); it != d->mPlayerList.end(); ++it) {
        KPlayer *player = *it;
        player->Debug();
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "+++++++++++"
                                       << "DONE=";
    Q_EMIT signalClientLeftGame(0, oldgamestatus, this);
}

void KGame::slotClientDisconnected(quint32 clientID, bool /*broken*/) // server side
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "++++(SERVER)+++++++"
                                       << "clientId=" << clientID;

    int oldgamestatus = gameStatus();

    KPlayer *player;
    KGamePlayerList removeList;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Playerlist of client=" << d->mPlayerList.count() << "count";
    for (KGamePlayerList::iterator it = d->mPlayerList.begin(); it != d->mPlayerList.end(); ++it) {
        KPlayer *player = *it;
        if (KGameMessage::rawGameId(player->id()) == clientID) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Player" << player->id() << "belongs to the removed game";
            removeList.append(player);
        }
    }

    for (KGamePlayerList::iterator it = removeList.begin(); it != removeList.end(); ++it) {
        KPlayer *player = *it;
        // try to replace the KGameIO first
        bool remove = true;
        Q_EMIT signalReplacePlayerIO(player, &remove);
        if (remove) {
            // otherwise (no new KGameIO) remove the player
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << " ---> Removing player" << player->id();
            removePlayer(player, 0);
        }
    }

    // Now add inactive players - sequence should be ok
    // TODO remove players from removed game
    for (int idx = 0; idx < d->mInactiveIdList.count(); idx++) {
        int it1 = d->mInactiveIdList.at(idx);
        player = findPlayer(it1);
        if (((int)playerCount() < maxPlayers() || maxPlayers() < 0) && player && KGameMessage::rawGameId(it1) != clientID) {
            activatePlayer(player);
        }
    }
    Q_EMIT signalClientLeftGame(clientID, oldgamestatus, this);
}

// -------------------- Synchronization -----------------------

// this initializes a newly connected client.
// we send the number of players (including type) as well as game status and
// properties to the client. After the initialization has been completed both
// clients should have the same status (ie players, properties, etc)
void KGame::negotiateNetworkGame(quint32 clientID)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "==========================="
                                       << ": clientID=" << clientID << " =========================== ";
    if (!isAdmin()) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": Serious WARNING..only gameAdmin should call this";
        return;
    }

    QByteArray buffer;
    QDataStream streamGS(&buffer, QIODevice::WriteOnly);

    // write Game setup specific data
    // streamGS << (qint32)maxPlayers();
    // streamGS << (qint32)minPlayers();

    // send to the newly connected client *only*
    qint16 v = KGameMessage::version();
    qint32 c = cookie();
    streamGS << v << c;
    sendSystemMessage(streamGS, KGameMessage::IdSetupGame, clientID);
}

bool KGame::sendGroupMessage(const QByteArray &msg, int msgid, quint32 sender, const QString &group)
{
    // AB: group must not be i18n'ed!! we should better use an id for group and use
    // a groupName() for the name // FIXME

    for (KGamePlayerList::iterator it = d->mPlayerList.begin(); it != d->mPlayerList.end(); ++it) {
        KPlayer *player = *it;
        if (player && player->group() == group) {
            sendMessage(msg, msgid, player->id(), sender);
        }
    }
    return true;
}

bool KGame::sendGroupMessage(const QDataStream &msg, int msgid, quint32 sender, const QString &group)
{
    return sendGroupMessage(((QBuffer *)msg.device())->buffer(), msgid, sender, group);
}

bool KGame::sendGroupMessage(const QString &msg, int msgid, quint32 sender, const QString &group)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << msg;
    return sendGroupMessage(stream, msgid, sender, group);
}

bool KGame::addProperty(KGamePropertyBase *data)
{
    return dataHandler()->addProperty(data);
}

bool KGame::sendPlayerProperty(int msgid, QDataStream &s, quint32 playerId)
{
    return sendSystemMessage(s, msgid, playerId);
}

void KGame::sendProperty(int msgid, QDataStream &stream, bool *sent)
{
    bool s = sendSystemMessage(stream, msgid);
    if (s) {
        *sent = true;
    }
}

void KGame::emitSignal(KGamePropertyBase *me)
{
    Q_EMIT signalPropertyChanged(me, this);
}

KGamePropertyBase *KGame::findProperty(int id) const
{
    return d->mProperties->find(id);
}

KGame::GamePolicy KGame::policy() const
{
    return d->mPolicy;
}
void KGame::setPolicy(GamePolicy p, bool recursive)
{
    // Set KGame policy
    d->mPolicy = p;
    if (recursive) {
        // Set all KGame property policy
        dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p, false);

        // Set all KPLayer (active or inactive) property policy
        for (KGamePlayerList::iterator it = d->mPlayerList.begin(); it != d->mPlayerList.end(); ++it) {
            (*it)->dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p, false);
        }
        for (KGamePlayerList::iterator it = d->mInactivePlayerList.begin(); it != d->mInactivePlayerList.end(); ++it) {
            (*it)->dataHandler()->setPolicy((KGamePropertyBase::PropertyPolicy)p, false);
        }
    }
}

#include "moc_kgame.cpp"

/*
 * vim: et sw=2
 */
