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

void KGameMessage::createHeader(QDataStream &msg,int sender,int receiver,int msgid)
{
  msg << (Q_INT16)sender << (Q_INT16)receiver << (Q_INT16)msgid;
}

void KGameMessage::extractHeader(QDataStream &msg,int &sender,int &receiver,int &msgid)
{
  Q_INT16 d3,d4,d5;
  msg >> d3 >> d4 >> d5;
  sender=d3;receiver=d4;msgid=d5;
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

void KGameMessage::createPropertyCommand(QDataStream &msg,int cmdid,int pid,int cmd)
{
  createPropertyHeader(msg,cmdid);
  msg << (Q_INT16)pid ;
  msg << (Q_INT8)cmd ;
}

void KGameMessage::extractPropertyCommand(QDataStream &msg,int &pid,int &cmd)
{
  Q_INT16 d1;
  Q_INT8 d2;
  msg >> d1 >> d2;
  pid=d1;
  cmd=d2;
}

int KGameMessage::version()
{
  return MESSAGE_VERSION;
}

