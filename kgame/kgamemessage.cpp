/* **************************************************************************
                           KGameMessages
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
#include "kgamemessage.h"

#define MESSAGE_VERSION 1
#define MESSAGE_MAGIC_COOKIE 0x02905723;


int KGameMessage::calcMessageId(int gameid,int playerid)
{
  return playerid?playerid: (gameid<<10);
}
int KGameMessage::calcPlayerId(int msgid)
{
  return (msgid & 0x3ff) ? msgid : 0;
}
int KGameMessage::calcGameId(int msgid)
{
  return calcPlayerId(msgid) ? 0 : (msgid >> 10);
}

void KGameMessage::createHeader(QDataStream &msg,int cookie,int msgversion,int sender,int receiver,int msgid)
{
  msg << (Q_INT16)cookie << (Q_INT8)msgversion << (Q_INT16)sender << (Q_INT16)receiver << (Q_INT16)msgid;
}

void KGameMessage::extractHeader(QDataStream &msg,int &cookie,int &msgversion,int &sender,int &receiver,int &msgid)
{
  Q_INT16 d1,d3,d4,d5;
  Q_INT8 d2;
  msg >> d1 >> d2 >> d3 >> d4 >> d5;
  cookie=d1;msgversion=d2;sender=d3;receiver=d4;msgid=d5;
}

void KGameMessage::createPropertyHeader(QDataStream &msg,int id)
{
  msg << (Q_INT16)id;
}

void KGameMessage::extractPropertyHeader(QDataStream &msg,int &id)
{
  Q_INT16 d1;
  msg >> d1;
  id=d1;
}

void KGameMessage::createSystemHeader(char *buffer,size_t size)
{
  Q_INT32 *cookie = (Q_INT32 *)buffer;
  Q_INT32 *len = (Q_INT32 *)(buffer + sizeof(Q_INT32));
  *cookie = magicCookie();
  *len = size;// - sizeof(Q_INT32) - sizeof(Q_INT32);// count cookie and len, too as we use it everywhere, now
}

bool KGameMessage::extractMessageLength(const char *buffer,size_t &len)
{
  Q_INT32 cookie=magicCookie();
  if (0==memcmp(buffer,&cookie,sizeof(Q_INT32)))
  {
     Q_INT32 *pL=(Q_INT32 *)(buffer+sizeof(Q_INT32));
     len=(size_t)(*pL);
     return true;
  }
  else return false;
}

int KGameMessage::version()
{
  return MESSAGE_VERSION;
}

size_t KGameMessage::systemHeaderSize()
{
    return sizeof(Q_INT32)+sizeof(Q_INT32);
}

Q_INT32 KGameMessage::magicCookie()
{
    return MESSAGE_MAGIC_COOKIE;
}

bool KGameMessage::createSetupGame(QDataStream& stream, bool isOffering, int maxPlayers, int clientId, int gameId, uint playerCount)
{
 stream << (Q_INT32)isOffering;
 stream << (Q_INT32)maxPlayers;
 stream << (Q_INT32)clientId; // hmm maybe obsolete
 stream << (Q_INT32)gameId;
 stream << (Q_INT32)playerCount;
 return true;
}

bool KGameMessage::extractSetupGame(QDataStream& stream, Q_INT32& isOffering, Q_INT32& maxPlayers, Q_INT32& clientId, Q_INT32& gameId, Q_INT32& playerCount)
{
 stream >> isOffering;
 stream >> maxPlayers;
 stream >>clientId;
 stream >> gameId;
 stream >> playerCount;
 return true;
}
