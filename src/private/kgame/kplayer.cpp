/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kplayer.h"

// own
#include "kgame.h"
#include "kgameio.h"
#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"
// KF
#include <KLocalizedString>
// Qt
#include <QBuffer>
// Std
#include <cassert>
#include <cstdio>

#define KPLAYER_LOAD_COOKIE 7285

class KPlayerPrivate
{
public:
    KPlayerPrivate()
    {
        mNetworkPlayer = nullptr;
    }

    KGame *mGame;
    bool mActive; // active player
    KPlayer::KGameIOList mInputList;

    // GameProperty
    KGamePropertyBool mAsyncInput; // async input allowed
    KGamePropertyBool mMyTurn; // Is it my turn to play (only useful if not async)?
    KGamePropertyInt mUserId; // a user defined id

    quint32 mId;
    bool mVirtual; // virtual player
    int mPriority; // tag for replacement

    KPlayer *mNetworkPlayer; // replacement player

    KGamePropertyHandler mProperties;

    // Playerdata
    KGamePropertyQString mName;
    KGamePropertyQString mGroup;
};

KPlayer::KPlayer()
    : QObject()
    , d(new KPlayerPrivate)
{
    init();
}

void KPlayer::init()
{
    // note that NO KGame object exists here! so we cannot use KGameProperty::send!
    qCDebug(GAMES_PRIVATE_KGAME) << ": this=" << this << ", sizeof(this)=" << sizeof(KPlayer);
    qCDebug(GAMES_PRIVATE_KGAME) << "sizeof(m_Group)=" << sizeof(d->mGroup);

    d->mProperties.registerHandler(KGameMessage::IdPlayerProperty, this, SLOT(sendProperty(int, QDataStream &, bool *)), SLOT(emitSignal(KGamePropertyBase *)));
    d->mVirtual = false;
    d->mActive = true;
    d->mGame = nullptr;
    d->mId = 0; // "0" is always an invalid ID!
    d->mPriority = 0;
    // I guess we cannot translate the group otherwise no
    // international connections are possible

    d->mUserId.registerData(KGamePropertyBase::IdUserId, this, i18n("UserId"));
    d->mUserId.setLocal(0);
    d->mGroup.registerData(KGamePropertyBase::IdGroup, this, i18n("Group"));
    d->mGroup.setLocal(i18n("default"));
    d->mName.registerData(KGamePropertyBase::IdName, this, i18n("Name"));
    d->mName.setLocal(i18n("default"));

    d->mAsyncInput.registerData(KGamePropertyBase::IdAsyncInput, this, i18n("AsyncInput"));
    d->mAsyncInput.setLocal(false);
    d->mMyTurn.registerData(KGamePropertyBase::IdTurn, this, i18n("myTurn"));
    d->mMyTurn.setLocal(false);
    d->mMyTurn.setEmittingSignal(true);
    d->mMyTurn.setOptimized(false);
}

KPlayer::~KPlayer()
{
    qCDebug(GAMES_PRIVATE_KGAME) << ": this=" << this << ", id=" << this->id();

    // Delete IODevices
    qDeleteAll(d->mInputList);
    d->mInputList.clear();
    if (game()) {
        game()->playerDeleted(this);
    }

    // note: mProperties does not use autoDelete or so - user must delete objects
    // himself
    d->mProperties.clear();
    qCDebug(GAMES_PRIVATE_KGAME) << "done";
}

int KPlayer::rtti() const
{
    return 0;
}

KPlayer::KGameIOList *KPlayer::ioList()
{
    return &d->mInputList;
}

void KPlayer::setGame(KGame *game)
{
    d->mGame = game;
}

KGame *KPlayer::game() const
{
    return d->mGame;
}

void KPlayer::setAsyncInput(bool a)
{
    d->mAsyncInput = a;
}

bool KPlayer::asyncInput() const
{
    return d->mAsyncInput.value();
}

bool KPlayer::isActive() const
{
    return d->mActive;
}

void KPlayer::setActive(bool v)
{
    d->mActive = v;
}

int KPlayer::userId() const
{
    return d->mUserId.value();
}

void KPlayer::setUserId(int i)
{
    d->mUserId = i;
}

bool KPlayer::myTurn() const
{
    return d->mMyTurn.value();
}

bool KPlayer::forwardMessage(QDataStream &msg, int msgid, quint32 receiver, quint32 sender)
{
    if (!isActive()) {
        return false;
    }
    if (!game()) {
        return false;
    }
    qCDebug(GAMES_PRIVATE_KGAME) << ": to game sender=" << sender << ""
                                 << "recv=" << receiver << "msgid=" << msgid;
    return game()->sendSystemMessage(msg, msgid, receiver, sender);
}

bool KPlayer::forwardInput(QDataStream &msg, bool transmit, quint32 sender)
{
    if (!isActive()) {
        return false;
    }
    if (!game()) {
        return false;
    }

    qCDebug(GAMES_PRIVATE_KGAME) << ": to game playerInput(sender=" << sender << ")";
    if (!asyncInput() && !myTurn()) {
        qCDebug(GAMES_PRIVATE_KGAME) << ": rejected cause it is not our turn";
        return false;
    }

    // AB: I hope I remember the usage correctly:
    // this function is called twice (on sender side) - once with transmit = true
    // where it sends the input to the comserver and once with transmit = false
    // where it really looks at the input
    if (transmit) {
        qCDebug(GAMES_PRIVATE_KGAME) << "indirect playerInput";
        return game()->sendPlayerInput(msg, this, sender);
    } else {
        qCDebug(GAMES_PRIVATE_KGAME) << "direct playerInput";
        return game()->systemPlayerInput(msg, this, sender);
    }
}

void KPlayer::setId(quint32 newid)
{
    // Needs to be after the sendProcess
    d->mId = newid;
}

void KPlayer::setGroup(const QString &group)
{
    d->mGroup = group;
}

const QString &KPlayer::group() const
{
    return d->mGroup.value();
}

void KPlayer::setName(const QString &name)
{
    d->mName = name;
}

const QString &KPlayer::name() const
{
    return d->mName.value();
}

quint32 KPlayer::id() const
{
    return d->mId;
}

KGamePropertyHandler *KPlayer::dataHandler()
{
    return &d->mProperties;
}

void KPlayer::setVirtual(bool v)
{
    d->mVirtual = v;
}

bool KPlayer::isVirtual() const
{
    return d->mVirtual;
}

void KPlayer::setNetworkPlayer(KPlayer *p)
{
    d->mNetworkPlayer = p;
}

KPlayer *KPlayer::networkPlayer() const
{
    return d->mNetworkPlayer;
}

int KPlayer::networkPriority() const
{
    return d->mPriority;
}

void KPlayer::setNetworkPriority(int p)
{
    d->mPriority = p;
}

bool KPlayer::addGameIO(KGameIO *input)
{
    if (!input) {
        return false;
    }
    d->mInputList.append(input);
    input->initIO(this); // set player and init device
    return true;
}

// input=0, remove all
bool KPlayer::removeGameIO(KGameIO *targetinput, bool deleteit)
{
    qCDebug(GAMES_PRIVATE_KGAME) << ":" << targetinput << "delete=" << deleteit;
    bool result = true;
    if (!targetinput) // delete all
    {
        KGameIO *input;
        while (!d->mInputList.isEmpty()) {
            input = d->mInputList.first();
            if (input)
                removeGameIO(input, deleteit);
        }
    } else {
        //    qCDebug(GAMES_PRIVATE_KGAME) << "remove IO" << targetinput;
        if (deleteit) {
            delete targetinput;
        } else {
            targetinput->setPlayer(nullptr);
            result = d->mInputList.removeAll(targetinput);
        }
    }
    return result;
}

bool KPlayer::hasRtti(int rtti) const
{
    return findRttiIO(rtti) != nullptr;
}

KGameIO *KPlayer::findRttiIO(int rtti) const
{
    QListIterator<KGameIO *> it(d->mInputList);
    while (it.hasNext()) {
        KGameIO *curGameIO = it.next();
        if (curGameIO->rtti() == rtti) {
            return curGameIO;
        }
    }
    return nullptr;
}

int KPlayer::calcIOValue()
{
    int value = 0;
    QListIterator<KGameIO *> it(d->mInputList);
    while (it.hasNext()) {
        value |= it.next()->rtti();
    }
    return value;
}

bool KPlayer::setTurn(bool b, bool exclusive)
{
    qCDebug(GAMES_PRIVATE_KGAME) << ":" << id() << " (" << this << ") to" << b;
    if (!isActive()) {
        return false;
    }

    // if we get to do an exclusive turn all other players are disallowed
    if (exclusive && b && game()) {
        for (KGame::KGamePlayerList::iterator it = game()->playerList()->begin(); it != game()->playerList()->end(); ++it) {
            if ((*it) == this) {
                continue;
            }
            (*it)->setTurn(false, false);
        }
    }

    // Return if nothing changed
    d->mMyTurn = b;

    return true;
}

bool KPlayer::load(QDataStream &stream)
{
    qint32 id, priority;
    stream >> id >> priority;
    setId(id);
    setNetworkPriority(priority);

    // Load Player Data
    // FIXME: maybe set all properties setEmitSignal(false) before?
    d->mProperties.load(stream);

    qint16 cookie;
    stream >> cookie;
    if (cookie == KPLAYER_LOAD_COOKIE) {
        qCDebug(GAMES_PRIVATE_KGAME) << "   Player loaded properly";
    } else {
        qCCritical(GAMES_PRIVATE_KGAME) << "   Player loading error. probably format error";
    }

    // Q_EMIT signalLoad(stream);
    return true;
}

bool KPlayer::save(QDataStream &stream)
{
    stream << (qint32)id() << (qint32)networkPriority();

    d->mProperties.save(stream);

    stream << (qint16)KPLAYER_LOAD_COOKIE;

    // Q_EMIT signalSave(stream);
    return true;
}

void KPlayer::networkTransmission(QDataStream &stream, int msgid, quint32 sender)
{
    // qCDebug(GAMES_PRIVATE_KGAME) ": msgid=" << msgid << "sender=" << sender << "we are=" << id();
    //  PlayerProperties processed
    bool issender;
    if (game()) {
        issender = sender == game()->gameId();
    } else {
        issender = true;
    }
    if (d->mProperties.processMessage(stream, msgid, issender)) {
        return;
    }
    switch (msgid) {
    case KGameMessage::IdPlayerInput: {
        qCDebug(GAMES_PRIVATE_KGAME) << ": Got player move "
                                     << "KPlayer (virtual) forwards it to the game object";
        forwardInput(stream, false);
    } break;
    default:
        Q_EMIT signalNetworkData(msgid - KGameMessage::IdUser, ((QBuffer *)stream.device())->readAll(), sender, this);
        qCDebug(GAMES_PRIVATE_KGAME) << ": "
                                     << "User data msgid" << msgid;
        break;
    }
}

KGamePropertyBase *KPlayer::findProperty(int id) const
{
    return d->mProperties.find(id);
}

bool KPlayer::addProperty(KGamePropertyBase *data)
{
    return d->mProperties.addProperty(data);
}

void KPlayer::sendProperty(int msgid, QDataStream &stream, bool *sent)
{
    if (game()) {
        bool s = game()->sendPlayerProperty(msgid, stream, id());
        if (s) {
            *sent = true;
        }
    }
}

void KPlayer::emitSignal(KGamePropertyBase *me)
{
    // Notify KGameIO (Process) for a new turn
    if (me->id() == KGamePropertyBase::IdTurn) {
        // qCDebug(GAMES_PRIVATE_KGAME) << ": for KGamePropertyBase::IdTurn";
        QListIterator<KGameIO *> it(d->mInputList);
        while (it.hasNext()) {
            it.next()->notifyTurn(d->mMyTurn.value());
        }
    }
    Q_EMIT signalPropertyChanged(me, this);
}

// --------------------- DEBUG --------------------
void KPlayer::Debug()
{
    qCDebug(GAMES_PRIVATE_KGAME) << "------------------- KPLAYER -----------------------";
    qCDebug(GAMES_PRIVATE_KGAME) << "this:    " << this;
    qCDebug(GAMES_PRIVATE_KGAME) << "rtti:    " << rtti();
    qCDebug(GAMES_PRIVATE_KGAME) << "id  :    " << id();
    qCDebug(GAMES_PRIVATE_KGAME) << "Name :   " << name();
    qCDebug(GAMES_PRIVATE_KGAME) << "Group:   " << group();
    qCDebug(GAMES_PRIVATE_KGAME) << "Async:   " << asyncInput();
    qCDebug(GAMES_PRIVATE_KGAME) << "myTurn:  " << myTurn();
    qCDebug(GAMES_PRIVATE_KGAME) << "Virtual:" << isVirtual();
    qCDebug(GAMES_PRIVATE_KGAME) << "Active:  " << isActive();
    qCDebug(GAMES_PRIVATE_KGAME) << "Priority:" << networkPriority();
    qCDebug(GAMES_PRIVATE_KGAME) << "Game   :" << game();
    qCDebug(GAMES_PRIVATE_KGAME) << "#IOs:    " << d->mInputList.count();
    qCDebug(GAMES_PRIVATE_KGAME) << "---------------------------------------------------";
}

#include "moc_kplayer.cpp"
