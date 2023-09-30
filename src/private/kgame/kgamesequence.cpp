/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2003 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2003 Martin Heni <kde at heni-online.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamesequence.h"

// own
#include "kgame.h"
#include "kplayer.h"
#include <kdegamesprivate_kgame_logging.h>

class KGameSequencePrivate
{
public:
    KGameSequencePrivate()
        : mGame(nullptr)
        , mCurrentPlayer(nullptr)
    {
    }

    KGame *mGame;
    KPlayer *mCurrentPlayer;
};

KGameSequence::KGameSequence()
    : QObject()
    , d(new KGameSequencePrivate)
{
}

KGameSequence::~KGameSequence() = default;

void KGameSequence::setGame(KGame *game)
{
    d->mGame = game;
}

KGame *KGameSequence::game() const
{
    return d->mGame;
}

KPlayer *KGameSequence::currentPlayer() const
{
    return d->mCurrentPlayer;
}

void KGameSequence::setCurrentPlayer(KPlayer *player)
{
    d->mCurrentPlayer = player;
}

KPlayer *KGameSequence::nextPlayer(KPlayer *last, bool exclusive)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "=================== NEXT PLAYER ==========================";
    if (!game()) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "NULL game object";
        return nullptr;
    }
    unsigned int minId, nextId, lastId;
    KPlayer *nextplayer, *minplayer;
    if (last) {
        lastId = last->id();
    } else {
        lastId = 0;
    }

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "nextPlayer: lastId=" << lastId;

    // remove when this has been checked
    minId = 0x7fff; // we just need a very large number...properly MAX_UINT or so would be ok...
    nextId = minId;
    nextplayer = nullptr;
    minplayer = nullptr;

    QList<KPlayer *>::iterator it = game()->playerList()->begin();
    for (; it != game()->playerList()->end(); it++) {
        KPlayer *player = *it;
        // Find the first player for a cycle
        if (player->id() < minId) {
            minId = player->id();
            minplayer = player;
        }
        if (player == last) {
            continue;
        }
        // Find the next player which is bigger than the current one
        if (player->id() > lastId && player->id() < nextId) {
            nextId = player->id();
            nextplayer = player;
        }
    }

    // Cycle to the beginning
    if (!nextplayer) {
        nextplayer = minplayer;
    }

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << " ##### lastId=" << lastId << "exclusive=" << exclusive << "  minId=" << minId << "nextid=" << nextId
                                       << "count=" << game()->playerList()->count();
    if (nextplayer) {
        nextplayer->setTurn(true, exclusive);
    } else {
        return nullptr;
    }
    return nextplayer;
}

// Per default we do not do anything
int KGameSequence::checkGameOver(KPlayer *)
{
    return 0;
}
/*
 * vim: et sw=2
 */

#include "moc_kgamesequence.cpp"
