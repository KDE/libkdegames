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

#ifndef KGGZMOD_MODULE_H
#define KGGZMOD_MODULE_H

#include <kggzmod/request.h>
#include <kggzmod/event.h>

#include <QObject>
#include <QList>

/**
 * @mainpage
 *
 * The \b kggzmod library is a KDE/Qt game developer's interface to the
 * GGZ Gaming Zone. It will connect the game with its corresponding game
 * server, provide information about players and spectators, allow for
 * chat and player management activities and a lot more.
 *
 * The main class to consider is \ref KGGZMod::Module. An object of
 * this class will accompany a game client for its entire lifetime, and
 * will offer signals for GGZ events and game server network events.
 *
 * Note: This is the KDE 4 version of kggzmod. There is also
 * a KDE 3 version available.
 *
 */
namespace KGGZMod
{

class ModulePrivate;
class Player;

/**
 * @short GGZ Gaming Zone game client to core client interface.
 *
 * Any game client intended to run on GGZ should instanciate exactly
 * one object of this class and connect its signals to the game
 * protocol handler. GGZ core client events are delivered by
 * \ref signalEvent but can be ignored. If needed, the \ref Event
 * class provides more information.
 * Messages from the game
 * server are emitted by \ref signalNetwork. This signal can either
 * lead to a networking routine within the game or to an instance of a
 * ggzcommgen-generated protocol handler class, which itself provides
 * signals for the game. The third signal, \ref signalError, should
 * terminate the network gaming with an appropriate error message.
 * When \ref signalError is emitted, the internal GGZ network
 * connections have already been disconnected.
 *
 * In addition to the signals, the current game state can be
 * queried, as can the list of players and game spectators
 * as \ref Player objects.
 *
 * Finally, the object of this class is used to send
 * request to GGZ, for changing the state, retrieving player
 * information and similar actions. The \ref Request class
 * has more details.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class Module : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Initialisation of online gaming through GGZ.
		 *
		 * Before online gaming is activated, the validity of
		 * the GGZ environment should be checked with the static
		 * \ref isGGZ method.
		 *
		 * @param name Name of the game client
		 */
		Module(QString name);
		~Module();

		/**
		 * The state a GGZ game can be in. These states are controlled
		 * by the GGZ server (for \ref created, \ref connected and
		 * \ref waiting) and afterwards by the game server which toggles
		 * between \ref waiting and \ref playing until finally reaching
		 * \ref done.
		 */
		enum State
		{
			created,	/**< The initial state. */
			connected,	/**< The GGZ core client could be contacted successfully. */
			waiting,	/**< The connection to the game server has been established. */
			playing,	/**< The game client is now playing. */
			done		/**< The game is over. */
		};

		/**
		 * Sends a request to the GGZ core client.
		 *
		 * The request is then forwarded to the GGZ server if necessary.
		 * In most cases, an event will be delivered back to the game client.
		 *
		 * @param request The request to the GGZ core client
		 */
		void sendRequest(Request request);

		/**
		 * Returns the list of seats on the table.
		 *
		 * This includes all active players, bots, open seats
		 * and abandoned/reserved seats.
		 */
		QList<Player*> players() const;

		/**
		 * Returns the list of game spectators.
		 */
		QList<Player*> spectators() const;

		/**
		 * Returns the current state the game is in.
		 */
		State state() const;

		/**
		 * Checks if the game is started in a GGZ environment.
		 *
		 * Calling \ref Module should only be done in case a
		 * GGZ environment has been detected.
		 *
		 * @return \b true if the game runs on GGZ, \b false otherwise
		 */
		static bool isGGZ();

		/**
		 * Returns information about the player who is running the
		 * game client.
		 *
		 * @return player information, or \b null if not available yet
		 */
		Player *self() const;

		/**
		 * Returns the single instance of this class. If no instance
		 * exists yet, \b null is returned. However, if multiple
		 * instances exist, this method might return one of the
		 * existing instances, but also \b null, i.e. the behaviour
		 * is undefined.
		 * In most scenarios, there will be exactly one instance.
		 *
		 * @return Module instance, or \b null if not applicable
		 */
		static Module *instance();

	signals:
		/**
		 * An event from the GGZ core client has happened.
		 *
		 * Such events can be ignored but are still useful for many
		 * games to know.
		 *
		 * @param event The event from the core client
		 */
 		void signalEvent(const KGGZMod::Event& event);

		/**
		 * An error has occurred.
		 *
		 * In such a case, the game client should terminate its
		 * multiplayer mode and depending on the situation also
		 * terminate itself.
		 */
		void signalError();

		/**
		 * Messages from the game server are available.
		 *
		 * If the connection to the game server is active and
		 * the game server writes out a message, the file descriptor
		 * contained in this event can be used to communicate with
		 * the game server.
		 * The file descriptor is initially reported in a
		 * \ref signalEvent as well as a \ref ServerEvent.
		 *
		 * @param fd File descriptor from which to read the message
		 */
		void signalNetwork(int fd);

	private:
		ModulePrivate *d;
};

}

#endif

