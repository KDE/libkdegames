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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
/*
    $Id$
*/

#include "kgamemessage.h"

#include <klocale.h>

#define MESSAGE_VERSION 2

Q_UINT32 KGameMessage::createPlayerId(int oldplayerid,Q_UINT32 gameid)
{
  int p;
  p = oldplayerid & 0x3ff; // remove game id
  p |= (gameid << 10);
  return p;
}

int KGameMessage::rawPlayerId(Q_UINT32 playerid)
{
  return playerid & 0x03ff;
}

Q_UINT32 KGameMessage::rawGameId(Q_UINT32 playerid)
{
  return (playerid & 0xfc00) >> 10;
}

bool KGameMessage::isPlayer(Q_UINT32 msgid)
{
  if (msgid & 0xfc00) {
	return true;
  } else {
	return false;
  }
}

bool KGameMessage::isGame(Q_UINT32 msgid)
{
  return !isPlayer(msgid);
}


void KGameMessage::createHeader(QDataStream &msg,Q_UINT32 sender,Q_UINT32 receiver,int msgid)
{
  msg << (Q_INT16)sender << (Q_INT16)receiver << (Q_INT16)msgid;
}

void KGameMessage::extractHeader(QDataStream &msg,Q_UINT32 &sender,Q_UINT32 &receiver,int &msgid)
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

QString KGameMessage::messageId2Text(int msgid)
{
// this should contain all KGameMessage::GameMessageIds
// feel free to add missing ones, to remove obsolete one and even feel free to
// let it be ;-)
  switch (msgid) {
	case KGameMessage::IdSetupGame:
		return i18n("Setup Game");
	case KGameMessage::IdSetupGameContinue:
		return i18n("Setup Game Continue");
	case KGameMessage::IdGameLoad:
		return i18n("Load Game");
	case KGameMessage::IdGameConnected:
		return i18n("Client game connected");
	case KGameMessage::IdGameSetupDone:
		return i18n("Game setup done");
	case KGameMessage::IdSyncRandom:
		return i18n("Synchronize Random");
	case KGameMessage::IdDisconnect:
		return i18n("Disconnect");
	case KGameMessage::IdPlayerProperty:
		return i18n("Player Property");
	case KGameMessage::IdGameProperty:
		return i18n("Game Property");
	case KGameMessage::IdAddPlayer:
		return i18n("Add Player");
	case KGameMessage::IdRemovePlayer:
		return i18n("Remove Player");
	case KGameMessage::IdActivatePlayer:
		return i18n("Activate Player");
	case KGameMessage::IdInactivatePlayer:
		return i18n("Inactivate Player");
	case KGameMessage::IdTurn:
		return i18n("Id Turn");
	case KGameMessage::IdError:
		return i18n("Error Message");
	case KGameMessage::IdPlayerInput:
		return i18n("Player Input");
	case KGameMessage::IdIOAdded:
		return i18n("An IO was added");
	case KGameMessage::IdProcessQuery:
		return i18n("Process Query");
	case KGameMessage::IdPlayerId:
		return i18n("Player ID");
	case KGameMessage::IdUser: // IdUser must be unknown for use, too!
	default:
		return QString::null;
  }
}
