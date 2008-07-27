/*
    This file is part of the kggzmod library.
    Copyright (c) 2005 - 2007 Josef Spillner <josef@ggzgamingzone.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KGGZMOD_MODULE_PRIVATE_H
#define KGGZMOD_MODULE_PRIVATE_H

#include <kggzmod/module.h>
#include <kggzmod/player.h>

#include <QtCore/QSocketNotifier>
#include <QtCore/QList>

class KGGZRaw;

namespace KGGZMod
{

class ModulePrivate : public QObject
{
	Q_OBJECT
	public:
		enum GGZEvents
		{
			msglaunch = 0,
			msgserver = 1,
			msgserverfd = 2,
			msgplayer = 3,
			msgseat = 4,
			msgspectatorseat = 5,
			msgchat = 6,
			msgstats = 7,
			msginfo = 8,
			msgrankings = 9
		};

		void connect();
		void disconnect();
		void sendRequest(Request request);
		void insertPlayer(Player::Type seattype, const QString &name, int seat);
		Player *findPlayer(Player::Type seattype, const QString &name);
		Player *self() const;
		QString opcodeString(GGZEvents opcode);
		QString requestString(Request::Type requestcode);

		QString m_name;
		int m_fd;
		Module::State m_state;
		QList<Player*> m_players;
		QList<Player*> m_spectators;

		QSocketNotifier *m_notifier;
		QSocketNotifier *m_gnotifier;
		KGGZRaw *m_net;

		int m_playerseats;
		int m_spectatorseats;

		int m_myseat;
		bool m_myspectator;

	public slots:
		void slotGGZEvent();
		void slotGGZError();

	signals:
		void signalEvent(const KGGZMod::Event& event);
		void signalError();
		void signalNetwork(int fd);
};

}

#endif

