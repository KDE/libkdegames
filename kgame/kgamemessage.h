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
#ifndef __KGAMEMSG_H_
#define __KGAMEMSG_H_

#include <qdatastream.h>

class KGameMessage
{
  public:
    /**
     * Calculates a sender or receiver message id from a given game and player
     * id.
     *
     * @param gameid The game id (<64)
     * @param playerid the player id (<1024)
     * @return The message id
     */
    static int calcMessageId(int gameid,int playerid);
    /**
     * Calculates the player id from a given message id
     * 
     *
     * @param msgid the message id
     * @return The player id
     */
    static int calcPlayerId(int msgid);
    /**
     * Calculates the gmae id from a given message id
     * 
     *
     * @param msgid the message id
     * @return The game id
     */
    static int calcGameId(int msgid);

    /**
     * Creates a message header given cookie,sender,receiver,...
     *
     * Also puts "hidden" header into the stream which are used by KGameClient
     * (message length and magic cookie). If you don't need them remove them
     * with @ref dropExternalHeader
     */
    static void createHeader(QDataStream &msg,int cookie,int msgversion,int sender,int receiver,int msgid);

    /**
     * Retrieves the information like cookie,sender,receiver,... from a message header 
     *
     * Note that it could be necessary to call @ref dropExternalHeader first
     */
    static void extractHeader(QDataStream &msg,int &cookie,int &msgversion,int &sender,int &receiver,int &msgid);

    /**
     * Creates a property header  given the property id
     */
    static void createPropertyHeader(QDataStream &msg,int id);

    /**
     * Retrieves the property id from a property message header
     */
    static void extractPropertyHeader(QDataStream &msg,int &id);

    /**
     * @return Version of the network library
     */
    static int version();

    /**
     * Returns the size of the magiccookie
     */
    static size_t systemHeaderSize();

    /**
     * inserts the magiccookie and the len into the mesage
     */
    static void createSystemHeader(char *buffer, size_t size);

    /** 
     * retrieves the length of the data block from the message
     */
    static bool extractMessageLength(const char *buffer,size_t &len);

    /**
     * Creates basic information for the setupGame code
     * @param isOffering Whether connections are offered
     **/
    static bool createSetupGame(QDataStream& stream, bool isOffering, int maxPlayers, int clientId, int gameId, uint playerCount);

    /**
     * Extracts the information created by @ref createSetupGame
     **/
    static bool extractSetupGame(QDataStream& stream,Q_INT32& isOffering, Q_INT32& maxPlayers, Q_INT32& clientId, Q_INT32& gameId, Q_INT32& playerCount);



  enum GameMessageIds {
    IdMessage,
    IdSetupGame,        // sent to a newly connected player
    IdSetupGameContinue,// continue the setup
    IdGameLoad,         // load/save the game to the client
    IdGameReactivatePlayer,   // reactive the inactive players
    IdSyncRandom,       // new random seed set - sync games
    IdAddPlayer,        // add a player
    IdRemovePlayer,     // the player will be removed
    IdActivatePlayer,   // Activate a player
    IdInactivatePlayer, // Inactivate a player
    IdError,            // an error occured
    IdPlayerInput,      // a player input occured
    IdTurn,             // unused?
    IdPlayerProperty,   // a player property changed
    IdGameProperty,     // a game property changed
    IdProcessQuery,     // Process queries data (process only)
    IdPlayerId,         // PlayerId got changed (process only)
//    IdChat=64,          // Chat - unused
    IdUser=256          // a user specified message
  };

  protected:
    /**
     * Returns the  magic cookie for the messages
     */
    static Q_INT32 magicCookie();
};

#endif
