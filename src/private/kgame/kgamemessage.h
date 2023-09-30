/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KGAMEMESSAGE_H_
#define __KGAMEMESSAGE_H_

// own
#include "kdegamesprivate_export.h"
// Qt
#include <QDataStream>

/**
 * \class KGameMessage kgamemessage.h <KGame/KGameMessage>
 *
 * Namespace-like class for message-related static functions.
 */
class KDEGAMESPRIVATE_EXPORT KGameMessage
{
public:
    /**
     * Creates a fully qualified player ID which contains the original
     * player id in the lower bits and the game number in the higher bits.
     * Do not rely on the exact bit positions as they are internal.
     *
     * See also @ref rawPlayerId and @ref rawGameId which are the inverse
     * operations
     *
     * @param player The player id - can include a gameid (will get removed)
     * @param game The game id (<64). 0 For broadcast.
     * @return The new player id
     */
    static quint32 createPlayerId(int player, quint32 game);

    /**
     * Returns the raw playerid, that is, a id which does not
     * contain the game number encoded in it. See also @ref createPlayerId which
     * is the inverse operation.
     *
     * @param playerid The player id
     * @return The raw player id
     */
    static int rawPlayerId(quint32 playerid);

    /**
     * Returns the raw game id, that is, the game id the player
     * belongs to. Se also @ref createPlayerId which is the inverse operation.
     *
     * @param playerid The player id
     * @return The raw game id
     */
    static quint32 rawGameId(quint32 playerid);

    /**
     * Checks whether a message receiver/sender is a player
     *
     * @param id The ID of the sender/receiver
     * @return true/false
     */
    static bool isPlayer(quint32 id);

    /**
     * Checks whether the sender/receiver of a message is a game
     *
     * @param id The ID of the sender/receiver
     * @return true/false
     */
    static bool isGame(quint32 id);

    /**
     * Creates a message header given cookie,sender,receiver,...
     *
     * Also puts "hidden" header into the stream which are used by KGameClient
     * (message length and magic cookie). If you don't need them remove them
     * with dropExternalHeader
     */
    static void createHeader(QDataStream &msg, quint32 sender, quint32 receiver, int msgid);

    /**
     * Retrieves the information like cookie,sender,receiver,... from a message header
     *
     * Note that it could be necessary to call dropExternalHeader first
     */
    static void extractHeader(QDataStream &msg, quint32 &sender, quint32 &receiver, int &msgid);

    /**
     * Creates a property header  given the property id
     */
    static void createPropertyHeader(QDataStream &msg, int id);

    /**
     * Retrieves the property id from a property message header
     */
    static void extractPropertyHeader(QDataStream &msg, int &id);

    /**
     * Creates a property header given the property id
     */
    static void createPropertyCommand(QDataStream &msg, int cmdid, int pid, int cmd);

    /**
     * Retrieves the property id from a property message header
     */
    static void extractPropertyCommand(QDataStream &msg, int &pid, int &cmd);

    /**
     * @return Version of the network library
     */
    static int version();

    /**
     * This function takes a @ref GameMessageIds as argument and returns a
     * suitable string for it. This string can't be used to identify a message
     * (as it is i18n'ed) but it can make debugging more easy.
     * @return Either a i18n'ed string (the name of the id) or QString() if
     * the msgid is unknown
     */
    static QString messageId2Text(int msgid);

    /**
     * Message Ids used inside @ref KGame.
     *
     * You can use your own custom message Id by adding @p IdUser to it.
     */
    // please document every new id with a short comment
    enum GameMessageIds {
        // game init, game load, disconnect, ...
        IdSetupGame = 1, // sent to a newly connected player
        IdSetupGameContinue = 2, // continue the setup
        IdGameLoad = 3, // load/save the game to the client
        IdGameConnected = 4, // Client successfully connected to master
        IdSyncRandom = 5, // new random seed set - sync games
        IdDisconnect = 6, // KGame object disconnects from game
        IdGameSetupDone = 7, // New game client is now operational

        // properties
        IdPlayerProperty = 20, // a player property changed
        IdGameProperty = 21, // a game property changed

        // player management
        IdAddPlayer = 30, // add a player
        IdRemovePlayer = 31, // the player will be removed
        IdActivatePlayer = 32, // Activate a player
        IdInactivatePlayer = 33, // Inactivate a player
        IdTurn = 34, // Turn to be prepared

        // to-be-categorized
        IdError = 100, // an error occurred
        IdPlayerInput = 101, // a player input occurred
        IdIOAdded = 102, // KGameIO got added to a player...init this IO

        // special ids for computer player
        IdProcessQuery = 220, // Process queries data (process only)
        IdPlayerId = 221, // PlayerId got changed (process only)

        IdUser = 256 // a user specified message
    };
};

#endif
