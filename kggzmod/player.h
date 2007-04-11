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

#ifndef KGGZMOD_PLAYER_H
#define KGGZMOD_PLAYER_H

#include <QtCore/QString>

namespace KGGZMod
{

class PlayerPrivate;
class Statistics;

/**
 * @short Player representation of an online game.
 *
 * Every participant in an online game is represented by an object
 * of this class. This includes active players, but also passive
 * spectators, bots and even open seats. The \ref Type attribute
 * differentiates those objects.
 *
 * While a player's name is always present, the statistics and
 * extended information such as a photo or the real name are not.
 * If they're needed, requests of type \ref InfoRequest must be
 * sent out, and events of type \ref InfoEvent and \ref StatsEvent
 * must have been received.
 *
 * Seat assignments can be changed by sending further requests.
 * The \b KGGZSeatsDialog can be used to avoid the manual usage
 * of all of those requests.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class Player
{
	friend class ModulePrivate;

	public:
		/**
		 * The type of which a seat can be.
		 * While spectators sit on the sidelines, players
		 * and bots along with open/reserved/abandoned seats
		 * are the ones which participate in a game.
		 */
		enum Type
		{
			unknown,	/**< The type of a seat is not yet known. */
			open,		/**< A seat is open and also not reserved. */
			bot,		/**< This is a bot player (a server-side AI player). */
			player,		/**< This is a regular player. */
			reserved,	/**< The seat is reserved for someone. */
			abandoned,	/**< The seat has been abandoned by a player. */
			spectator	/**< This is a spectator who does only watch the game. */
		};

		/**
		 * The type a seat has.
		 *
		 * @return seat type
		 */
		Type type() const;

		/**
		 * The name of a player.
		 *
		 * This is the actual name for normal players and for bot
		 * players. It refers to a name for reserved and abandoned
		 * seats. Open seats do not have a name associated with them.
		 *
		 * @return name of the player or bot
		 */
		QString name() const;

		/**
		 * The seat number.
		 *
		 * This number is used to associate a player object uniquely
		 * with other information.
		 *
		 * @return number of the seat
		 */
		int seat() const;

		/**
		 * Statistics for a given player.
		 *
		 * In most cases this will not be present. Only for normal players
		 * and only if a \ref StatsEvent has been received, the statistics
		 * for the players become available.
		 *
		 * @return player statistics, or \b null if not present
		 */
		Statistics *stats() const;

		/**
		 * Returns the hostname of the player.
		 *
		 * If the game requires revealing the players' addresses and the
		 * player has agreed to do so, this returns the address as a
		 * hostname or IP address.
		 * This information depends on a previously sent \ref InfoRequest.
		 *
		 * @return player hostname
		 */
		QString hostname() const;

		/**
		 * Returns the photo of the player.
		 *
		 * If the player has added a photo of him/herself or an avatar
		 * picture is available in the database, this returns an
		 * URL to the image.
		 * This information depends on a previously sent \ref InfoRequest.
		 *
		 * @return player photo
		 */
		QString photo() const;

		/**
		 * Returns the realname of the player.
		 *
		 * If the player has identified him/herself with a real name
		 * in the database, this can be retrieved for display purposes.
		 * This information depends on a previously sent \ref InfoRequest.
		 *
		 * @return player realname
		 */
		QString realname() const;

	private:
		Player();
		~Player();
		PlayerPrivate *d;
		void init(PlayerPrivate *x);
};

}

#endif

