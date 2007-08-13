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

#ifndef KGGZMOD_EVENT_H
#define KGGZMOD_EVENT_H

#include <QtCore/QMap>
#include <QtCore/QString>

#include "kggzmod_export.h"

namespace KGGZMod
{

class Player;

/**
 * @short Events from the GGZ core client.
 *
 * When there is some important information from the GGZ core client
 * to the game client, an event is submitted in \ref Module::signalEvent.
 * In most cases the event is originating from the GGZ server.
 *
 * Depending on the type of the event, the more general \ref Event object
 * can be casted to specific objects of type \ref LaunchEvent, \ref ServerEvent,
 * \ref SelfEvent, \ref SeatEvent, \ref ChatEvent, \ref StatsEvent and
 * \ref InfoEvent. However, this is not strictly needed, as all data
 * is available from the public \ref data hashtable, should a game
 * wish to use this instead.
 *
 * None of the events must be handled by the game client, especially when
 * using the \b KGGZSeatsDialog class for player management.
 * However, it is often convenient to catch the \ref ServerEvent to get
 * the file descriptor which refers to the connection to the game server.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class KGGZMOD_EXPORT Event
{
	friend class ModulePrivate;

	public:
		/**
		 * The type of the event. Depending on this type,
		 * the object might be casted to a more specific
		 * class.
		 */
		enum Type
		{
			launch,	/**< Game was launched. */
			server,	/**< Connected to server. */
			self,	/**< Your seat was assigned. */
			seat,	/**< Someone's seat changed. */
			chat,	/**< A chat message was received */
			stats,	/**< Statistics have been received for a player. */
			info,	/**< Information about a player has arrived. */
			rankings /**< Experimental: Rankings information for this game/room */
		};

		/**
		 * Creates an event of a specific type.
		 *
		 * The event will not be valid until all \ref data fields
		 * are properly filled out. Therefore, only the \ref Module
		 * is supposed to create event objects.
		 *
		 * @param type Type of the event
		 *
		 * @internal
		 */
		Event(Type type);

		/**
		 * Returns the type of an event.
		 *
		 * @return type of an event
		 */
		Type type() const;

		/**
		 * Returns the pointer to the player associated with the event.
		 *
		 * If no player is associated, this returns \b null.
		 *
		 * @return player associated with an event
		 */
		Player *player() const;

		/**
		 * Data storage for all events.
		 *
		 * Independent of the actual type of the event,
		 * all important data is stored in this map.
		 */
		QMap<QString, QString> data;

	protected:
		Player *m_player;

	private:
		Type m_type;
};

/**
 * @short Launch event from the GGZ core client.
 *
 * An event of this type describes that a game server has just
 * been launched. This is the first event a game client ever gets.
 *
 * Refer to the \ref Event documentation for everything else.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class LaunchEvent : public Event
{
	public:
		LaunchEvent(const Event& event);
};

/**
 * @short Server event from the GGZ core client.
 *
 * An event of this type describes that the game client is connected
 * to the game server. The file descriptor can be retrieved through
 * the \ref fd method.
 *
 * Refer to the \ref Event documentation for everything else.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class KGGZMOD_EXPORT ServerEvent : public Event
{
	public:
		ServerEvent(const Event& event);
		int fd() const;
};

/**
 * @short Event about oneself from the GGZ core client.
 *
 * This event occurs once and informs the game client about its own
 * position in the game. The \ref self method returns the
 * player object which the game client is linked to.
 *
 * Refer to the \ref Event documentation for everything else.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class SelfEvent : public Event
{
	public:
		SelfEvent(const Event& event);
		Player *self() const;
};

/**
 * @short Event about seat changes from the GGZ core client.
 *
 * Whenever a seat assignment changes, the list of players is
 * updated and information about the new assignment is available
 * through this event's \ref player method.
 *
 * Refer to the \ref Event documentation for everything else.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class SeatEvent : public Event
{
	public:
		SeatEvent(const Event& event);
		Player *player() const;
};

/**
 * @short Chat event from the GGZ core client.
 *
 * Chat messages from a player to others on the same table are
 * displayed as table chat in GGZ core clients, but are also
 * delivered to the game clients.
 *
 * Refer to the \ref Event documentation for everything else.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class ChatEvent : public Event
{
	public:
		ChatEvent(const Event& event);
		Player *player() const;
		QString message() const;
};

/**
 * @short Statistics from the GGZ core client.
 *
 * After the table configuration is known, that is, after all
 * seats have been assigned, for both the player seats and
 * the spectator seats statistics are received. The \ref
 * Statistics are available through the returned \ref player
 * objects.
 *
 * Refer to the \ref Event documentation for everything else.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class StatsEvent : public Event
{
	public:
		StatsEvent(const Event& event);
		Player *player() const;
};

/**
 * @short Player information from the GGZ core client.
 *
 * Extended player information, if requested with a
 * \ref InfoRequest, has arrived when an event of this
 * type occurs.
 *
 * Refer to the \ref Event documentation for everything else.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class InfoEvent : public Event
{
	public:
		InfoEvent(const Event& event);
		Player *player() const;
};

/**
 * @short Rankings information for this game/room.
 *
 * After a game has been played, the statistics usually
 * change regarding player placements, highscores, ratings
 * and wins/losses. This event can be requested during the
 * game to report on the current statistics, and it will
 * arrive automatically at the end of the game to report
 * on the new one.
 *
 * Refer to the \ref Event documentation for everything else.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class KGGZMOD_EXPORT RankingsEvent : public Event
{
	public:
		RankingsEvent(const Event& event);
		int count() const;
		QString name(int i) const;
		int score(int i) const;
};

}

#endif

