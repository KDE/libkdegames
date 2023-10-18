/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001-2002 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamechat.h"

// own
#include "../kchatbase_p.h"
#include "kgame.h"
#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kplayer.h"
#include <kdegamesprivate_kgame_logging.h>
// KF
#include <KLocalizedString>
// Qt
#include <QMap>

// FIXME:
#define FIRST_ID 2 // first id, that is free of use, aka not defined above

class KGameChatPrivate : public KChatBasePrivate
{
public:
    KGameChatPrivate(KChatBaseModel *model, KChatBaseItemDelegate *delegate, QWidget *parent)
        : KChatBasePrivate(model, delegate, parent)
    {
    }

public:
    KGame *mGame = nullptr;
    KPlayer *mFromPlayer = nullptr;
    int mMessageId;

    QMap<int, int> mSendId2PlayerId;
    int mToMyGroup = -1; // just as the above - but for the group, not for players
};

KGameChat::KGameChat(KGame *g, int msgid, QWidget *parent, KChatBaseModel *model, KChatBaseItemDelegate *delegate)
    : KChatBase(*new KGameChatPrivate(model, delegate, parent), parent, false)
{
    init(g, msgid);
}

KGameChat::KGameChat(KGame *g, int msgid, KPlayer *fromPlayer, QWidget *parent, KChatBaseModel *model, KChatBaseItemDelegate *delegate)
    : KChatBase(*new KGameChatPrivate(model, delegate, parent), parent, false)
{
    init(g, msgid);
    setFromPlayer(fromPlayer);
}

KGameChat::KGameChat(QWidget *parent)
    : KChatBase(*new KGameChatPrivate(nullptr, nullptr, parent), parent, false)
{
    init(nullptr, -1);
}

KGameChat::~KGameChat()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG);
}

void KGameChat::init(KGame *g, int msgId)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG);
    setMessageId(msgId);

    setKGame(g);
}

void KGameChat::addMessage(int fromId, const QString &text)
{
    Q_D(KGameChat);

    if (!d->mGame) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "no KGame object has been set";
        addMessage(i18n("Player %1", fromId), text);
    } else {
        KPlayer *p = d->mGame->findPlayer(fromId);
        if (p) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "adding message of player" << p->name() << "id=" << fromId;
            addMessage(p->name(), text);
        } else {
            qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "Could not find player id" << fromId;
            addMessage(i18nc("Unknown player", "Unknown"), text);
        }
    }
}

void KGameChat::returnPressed(const QString &text)
{
    Q_D(KGameChat);

    if (!d->mFromPlayer) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": You must set a player first!";
        return;
    }
    if (!d->mGame) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": You must set a game first!";
        return;
    }

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "from:" << d->mFromPlayer->id() << "==" << d->mFromPlayer->name();

    int id = sendingEntry();

    if (isToGroupMessage(id)) {
        // note: there is currently no support for other groups than the players
        // group! It might be useful to send to other groups, too
        QString group = d->mFromPlayer->group();
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "send to group" << group;
        int sender = d->mFromPlayer->id();
        d->mGame->sendGroupMessage(text, messageId(), sender, group);

        // TODO
        // AB: this message is never received!! we need to connect to
        // KPlayer::networkData!!!
        // TODO

    } else {
        int toPlayer = 0;
        if (!isSendToAllMessage(id) && isToPlayerMessage(id)) {
            toPlayer = playerId(id);
            if (toPlayer == -1) {
                qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": don't know that player "
                                                      << "- internal ERROR";
            }
        }
        int receiver = toPlayer;
        int sender = d->mFromPlayer->id();
        d->mGame->sendMessage(text, messageId(), receiver, sender);
    }
}

void KGameChat::setMessageId(int msgid)
{
    Q_D(KGameChat);

    d->mMessageId = msgid;
}

int KGameChat::messageId() const
{
    Q_D(const KGameChat);

    return d->mMessageId;
}

bool KGameChat::isSendToAllMessage(int id) const
{
    return (id == KChatBase::SendToAll);
}

bool KGameChat::isToGroupMessage(int id) const
{
    Q_D(const KGameChat);

    return (id == d->mToMyGroup);
}

bool KGameChat::isToPlayerMessage(int id) const
{
    Q_D(const KGameChat);

    return d->mSendId2PlayerId.contains(id);
}

QString KGameChat::sendToPlayerEntry(const QString &name) const
{
    return i18n("Send to %1", name);
}

int KGameChat::playerId(int id) const
{
    Q_D(const KGameChat);

    if (!isToPlayerMessage(id)) {
        return -1;
    }

    return d->mSendId2PlayerId[id];
}

int KGameChat::sendingId(int playerId) const
{
    Q_D(const KGameChat);

    QMap<int, int>::ConstIterator it;
    for (it = d->mSendId2PlayerId.begin(); it != d->mSendId2PlayerId.end(); ++it) {
        if (it.value() == playerId) {
            return it.key();
        }
    }
    return -1;
}

QString KGameChat::fromName() const
{
    Q_D(const KGameChat);

    return d->mFromPlayer ? d->mFromPlayer->name() : QString();
}

bool KGameChat::hasPlayer(int id) const
{
    return (sendingId(id) != -1);
}

void KGameChat::setFromPlayer(KPlayer *p)
{
    Q_D(KGameChat);

    if (!p) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": NULL player";
        removeSendingEntry(d->mToMyGroup);
        d->mFromPlayer = nullptr;
        return;
    }
    if (d->mFromPlayer) {
        changeSendingEntry(p->group(), d->mToMyGroup);
    } else {
        if (d->mToMyGroup != -1) {
            qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "send to my group exists already - removing";
            removeSendingEntry(d->mToMyGroup);
        }
        d->mToMyGroup = nextId();
        addSendingEntry(i18n("Send to My Group (\"%1\")", p->group()), d->mToMyGroup);
    }
    d->mFromPlayer = p;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "player=" << p;
}

void KGameChat::setKGame(KGame *g)
{
    Q_D(KGameChat);

    if (d->mGame) {
        slotUnsetKGame();
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "game=" << g;
    d->mGame = g;

    if (d->mGame) {
        connect(d->mGame, &KGame::signalPlayerJoinedGame, this, &KGameChat::slotAddPlayer);
        connect(d->mGame, &KGame::signalPlayerLeftGame, this, &KGameChat::slotRemovePlayer);
        connect(d->mGame, &KGame::signalNetworkData, this, &KGameChat::slotReceiveMessage);
        connect(d->mGame, &KGame::destroyed, this, &KGameChat::slotUnsetKGame);

        const QList<KPlayer *> playerList = *d->mGame->playerList();
        for (KPlayer *player : playerList) {
            slotAddPlayer(player);
        }
    }
}

KGame *KGameChat::game() const
{
    Q_D(const KGameChat);

    return d->mGame;
}

KPlayer *KGameChat::fromPlayer() const
{
    Q_D(const KGameChat);

    return d->mFromPlayer;
}

void KGameChat::slotUnsetKGame()
{
    // TODO: test this method!
    Q_D(KGameChat);

    if (!d->mGame) {
        return;
    }
    disconnect(d->mGame, nullptr, this, nullptr);
    removeSendingEntry(d->mToMyGroup);
    QMap<int, int>::Iterator it;
    for (it = d->mSendId2PlayerId.begin(); it != d->mSendId2PlayerId.end(); ++it) {
        removeSendingEntry(it.value());
    }
}

void KGameChat::slotAddPlayer(KPlayer *p)
{
    Q_D(KGameChat);

    if (!p) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": cannot add NULL player";
        return;
    }
    if (hasPlayer(p->id())) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": player was added before";
        return;
    }

    int sendingId = nextId();
    addSendingEntry(comboBoxItem(p->name()), sendingId);
    d->mSendId2PlayerId.insert(sendingId, p->id());
    connect(p, &KPlayer::signalPropertyChanged, this, &KGameChat::slotPropertyChanged);
    connect(p, &KPlayer::signalNetworkData, this, &KGameChat::slotReceivePrivateMessage);
}

void KGameChat::slotRemovePlayer(KPlayer *p)
{
    Q_D(KGameChat);

    if (!p) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": NULL player";
        return;
    }
    if (!hasPlayer(p->id())) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": cannot remove non-existent player";
        return;
    }

    int id = sendingId(p->id());
    removeSendingEntry(id);
    p->disconnect(this);
    d->mSendId2PlayerId.remove(id);
}

void KGameChat::slotPropertyChanged(KGamePropertyBase *prop, KPlayer *player)
{
    if (prop->id() == KGamePropertyBase::IdName) {
        //	qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "new Name";
        changeSendingEntry(player->name(), sendingId(player->id()));
        /*
            mCombo->changeItem(comboBoxItem(player->name()), index);
         */
    } else if (prop->id() == KGamePropertyBase::IdGroup) {
        // TODO
    }
}

void KGameChat::slotReceivePrivateMessage(int msgid, const QByteArray &buffer, quint32 sender, KPlayer *me)
{
    if (!me || me != fromPlayer()) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "nope - not for us!";
        return;
    }
    slotReceiveMessage(msgid, buffer, me->id(), sender);
}

void KGameChat::slotReceiveMessage(int msgid, const QByteArray &buffer, quint32, quint32 sender)
{
    QDataStream msg(buffer);
    if (msgid != messageId()) {
        return;
    }

    QString text;
    msg >> text;

    addMessage(sender, text);
}

#include "moc_kgamechat.cpp"
