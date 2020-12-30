/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamemessage.h"

// KF
#include <KLocalizedString>

#define MESSAGE_VERSION 2

quint32 KGameMessage::createPlayerId(int oldplayerid,quint32 gameid)
{
  int p;
  p = oldplayerid & 0x3ff; // remove game id
  p |= (gameid << 10);
  return p;
}

int KGameMessage::rawPlayerId(quint32 playerid)
{
  return playerid & 0x03ff;
}

quint32 KGameMessage::rawGameId(quint32 playerid)
{
  return (playerid & 0xfc00) >> 10;
}

bool KGameMessage::isPlayer(quint32 msgid)
{
  if (msgid & 0xfc00) {
	return true;
  } else {
	return false;
  }
}

bool KGameMessage::isGame(quint32 msgid)
{
  return !isPlayer(msgid);
}


void KGameMessage::createHeader(QDataStream &msg,quint32 sender,quint32 receiver,int msgid)
{
  msg << (qint16)sender << (qint16)receiver << (qint16)msgid;
}

void KGameMessage::extractHeader(QDataStream &msg,quint32 &sender,quint32 &receiver,int &msgid)
{
  qint16 d3,d4,d5;
  msg >> d3 >> d4 >> d5;
  sender=d3;receiver=d4;msgid=d5;
}

void KGameMessage::createPropertyHeader(QDataStream &msg,int id)
{
  msg << (qint16)id;
}

void KGameMessage::extractPropertyHeader(QDataStream &msg,int &id)
{
  qint16 d1;
  msg >> d1;
  id=d1;
}

void KGameMessage::createPropertyCommand(QDataStream &msg,int cmdid,int pid,int cmd)
{
  createPropertyHeader(msg,cmdid);
  msg << (qint16)pid ;
  msg << (qint8)cmd ;
}

void KGameMessage::extractPropertyCommand(QDataStream &msg,int &pid,int &cmd)
{
  qint16 d1;
  qint8 d2;
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
		return QString();
  }
}
