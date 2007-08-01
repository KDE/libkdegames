/*
    This file is part of the KDE games library
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2003 Martin Heni (kde at heni-online.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgamesequence.h"
#include "kgamesequence.moc"

#include "kplayer.h"
#include "kgame.h"

#include <kdebug.h>

class KGameSequence::KGameSequencePrivate
{
  public:
    KGameSequencePrivate()
      : mGame(0), mCurrentPlayer(0)
    {
    }

    KGame* mGame;
    KPlayer* mCurrentPlayer;
};

KGameSequence::KGameSequence()
  : QObject(), d(new KGameSequencePrivate)
{
}

KGameSequence::~KGameSequence()
{
 delete d;
}

void KGameSequence::setGame(KGame* game)
{
 d->mGame = game;
}

KGame* KGameSequence::game() const
{
 return d->mGame;
}

KPlayer* KGameSequence::currentPlayer() const
{
 return d->mCurrentPlayer;
}

void KGameSequence::setCurrentPlayer(KPlayer* player)
{
 d->mCurrentPlayer = player;
}

KPlayer *KGameSequence::nextPlayer(KPlayer *last,bool exclusive)
{
 kDebug(11001) << "=================== NEXT PLAYER =========================="<<endl;
 if (!game())
 {
   kError() << k_funcinfo << "NULL game object";
   return 0;
 }
 unsigned int minId,nextId,lastId;
 KPlayer *nextplayer, *minplayer;
 if (last)
 {
   lastId = last->id();
 }
 else
 {
   lastId = 0;
 }

 kDebug(11001) << "nextPlayer: lastId="<<lastId<<endl;

 // remove when this has been checked
 minId = 0x7fff;  // we just need a very large number...properly MAX_UINT or so would be ok...
 nextId = minId;
 nextplayer = 0;
 minplayer = 0;

 QList<KPlayer*>::iterator it = game()->playerList()->begin();
 for (;it != game()->playerList()->end(); it++ )
 {
   KPlayer* player = *it;
   // Find the first player for a cycle
   if (player->id() < minId)
   {
     minId=player->id();
     minplayer=player;
   }
   if (player==last)
   {
     continue;
   }
   // Find the next player which is bigger than the current one
   if (player->id() > lastId && player->id() < nextId)
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

 kDebug(11001) << k_funcinfo << " ##### lastId=" << lastId << "exclusive="
        << exclusive << "  minId=" << minId << "nextid=" << nextId
        << "count=" << game()->playerList()->count();
 if (nextplayer)
 {
   nextplayer->setTurn(true,exclusive);
 }
 else
 {
   return 0;
 }
 return nextplayer;
}

// Per default we do not do anything
int KGameSequence::checkGameOver(KPlayer*)
{
 return 0;
}
/*
 * vim: et sw=2
 */
