/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/*
    $Id$
*/
#include "kgamemessage.h"

#define MESSAGE_VERSION 1


int KGameMessage::calcMessageId(int gameid,int playerid)
{
  return playerid?(playerid&0x3ff): ((gameid&0xff)<<10);
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
