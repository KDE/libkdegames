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
     * Creates a property header  given the property id
     */
    static void createPropertyCommand(QDataStream &msg,int cmdid,int pid,int cmd);

    /**
     * Retrieves the property id from a property message header
     */
    static void extractPropertyCommand(QDataStream &msg,int &pid,int &cmd);

    /**
     * @return Version of the network library
     */
    static int version();


// please document every new id with a short comment
  enum GameMessageIds {
// game init, game load, disconnect, ...
    IdSetupGame=1,         // sent to a newly connected player
    IdSetupGameContinue=2, // continue the setup
    IdGameLoad=3,          // load/save the game to the client
    IdGameReactivatePlayer=4,   // reactive the inactive players
    IdSyncRandom=5,        // new random seed set - sync games
    IdDisconnect=6,        // KGame object disconnects from game

// properties
    IdPlayerProperty=20,   // a player property changed
    IdGameProperty=21,     // a game property changed

// player management
    IdAddPlayer=30,         // add a player
    IdRemovePlayer=31,      // the player will be removed
    IdActivatePlayer=32,    // Activate a player
    IdInactivatePlayer=33,  // Inactivate a player
    IdTurn=34,              // Turn to be prepared
    
// to-be-categorized
    IdError=100,            // an error occured
    IdPlayerInput=101,      // a player input occured
    IdIOAdded=102,          // KGameIO got added to a player...init this IO

// special ids for computer player
    IdProcessQuery=220,     // Process queries data (process only)
    IdPlayerId=221,         // PlayerId got changed (process only)

    IdUser=256          // a user specified message
  };

  protected:
    /**
     * Returns the  magic cookie for the messages
     */
};

#endif
