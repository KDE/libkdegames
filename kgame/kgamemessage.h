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
    static void createHeader(QDataStream &msg,int sender,int receiver,int msgid);

    /**
     * Retrieves the information like cookie,sender,receiver,... from a message header 
     *
     * Note that it could be necessary to call @ref dropExternalHeader first
     */
    static void extractHeader(QDataStream &msg,int &sender,int &receiver,int &msgid);

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

  enum GameMessageIds {
    IdMessage=1,
    IdSetupGame=2,         // sent to a newly connected player
    IdSetupGameContinue=3, // continue the setup
    IdGameLoad=4,          // load/save the game to the client
    IdGameReactivatePlayer=5,   // reactive the inactive players
    IdSyncRandom=6,        // new random seed set - sync games
    IdAddPlayer=7,         // add a player
    IdRemovePlayer=8,      // the player will be removed
    IdActivatePlayer=9,    // Activate a player
    IdInactivatePlayer=10, // Inactivate a player
    IdError=11,            // an error occured
    IdPlayerInput=12,      // a player input occured
    IdTurn=13,             // unused?
    IdPlayerProperty=14,   // a player property changed
    IdGameProperty=15,     // a game property changed
    IdProcessQuery=16,     // Process queries data (process only)
    IdPlayerId=17,         // PlayerId got changed (process only)
    IdUser=256          // a user specified message
  };

  protected:
    /**
     * Returns the  magic cookie for the messages
     */
};

#endif
