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
#ifndef __KGAME_H_
#define __KGAME_H_

#include <qstring.h>
#include <qlist.h>
#include <qvaluelist.h>

#include "kgamenetwork.h"

class KRandomSequence;

class KPlayer;
class KGamePropertyBase;
class KGamePropertyHandler;

class KGamePrivate;

/**
 * The KGame class is the central game object. A game basically
 * consists of following features:
 * - Player handling (add, remove,...)
 * - Game status (end,start,pause,...)
 * - load/save
 * - Move (and message) handling 
 * - Network connection (for KGameNetwork)
 *
 * Example:
 * <pre>
 * KGame *game=new KGame;
 * </pre>
 *
 * Note: I think KGame and KGameNetwork should
 *       be merged into one class
 *
 * @short The main KDE game object
 * @author Martin Heni <martin@heni-online.de>
 * @version $Id$
 *
 */
class KGame : public KGameNetwork
{
  Q_OBJECT

public:
  typedef QPtrList<KPlayer> KGamePlayerList;

	/**
	 * The policy of the property. This can be PolicyClean (@ref setVale uses
	 * @ref send), PolicyDirty (@ref setValue uses @ref changeValue) or
	 * PolicyLocal (@ref setValue uses @ref setLocal).
	 *
	 * A "clean" policy means that the property is always the same on every
	 * client. This is achieved by calling @ref send which actually changes
	 * the value only when the message from the MessageServer is received.
	 *
	 * A "dirty" policy means that as soon as @ref setValue is called the
	 * property is changed immediately. And additionally sent over network.
	 * This can sometimes lead to bugs as the other clients do not 
	 * immediately have the same value. For more information see 
	 * @ref changeValue.
	 *
	 * PolicyLocal means that a @ref KGameProperty behaves like ever
	 * "normal" variable. Whenever @ref setValue is called (e.g. using "=")
	 * the value of the property is changes immediately without sending it
	 * over network. You might want to use this if you are sure that all
	 * clients set the property at the same time.
	 **/
	enum GamePolicy
	{
    PolicyUndefined = 0,
		PolicyClean = 1,
		PolicyDirty = 2,
		PolicyLocal = 3
	};

    /** 
     * Create a KGame object
     */
    KGame(int cookie=42,QObject* parent=0);
    ~KGame();

    /**
     * Gives debug output of the game status
     */
    virtual void Debug();

    /**
     * Game status: kind of unused at the moment
     */
    enum GameStatus { Init, Run, Pause, End, Abort, SystemPause, Intro, UserStatus };

    // Properties
    /**
     * Returns a list of all active players 
     *
     * @return the list of players
     */
    KGamePlayerList *playerList();

    /**
     * The same as @ref playerList but returns a const pointer.
     **/
    const KGamePlayerList *playerList() const;

    /**
     * Returns a list of all inactive players 
     * @return the list of players
     */
    KGamePlayerList *inactivePlayerList();

    /**
     * The same as @ref inactivePlayerList but returns a const pointer.
     **/
    const KGamePlayerList *inactivePlayerList() const;
    
    /**
     * Returns a pointer to the game's @ref KRandomSequence. This sequence is
     * identical for all network players!
     * @return @ref KRandomSequence pointer
     */
    KRandomSequence *random() const;

    /**
     * Is the game running
     * @return true/false
     */
    bool isRunning() const;

    // Player handling
    /**
     * Returns the player object for a given player id
     * @param id Player id
     * @return player object
     */
    KPlayer *findPlayer(Q_UINT32 id) const;

    /**
     * Note that @ref KPlayer::save must be implemented properly, as well as
     * @ref KPlayer::rtti!
     * This will only send a message to all clients. The player is _not_ added
     * directly!
     * See also @ref signalPlayerInput which will be emitted as soon as the
     * player really has been added.
     *
     * Note that an added player will first get into a "queue" and won't be in
     * the game. It will be added to the game as soon as systemAddPlayer is
     * called what will happen as soon as IdAddPlayer is received.
     *
     * Note: you probably want to connect to @ref signalPlayerJoinedGame for
     * further initialization!
     * @param newplayer The player you want to add. KGame will send a message to
     * all clients and add the player using @ref systemAddPlayer
     **/
    void addPlayer(KPlayer* newplayer);
    
    /**
     * Sends a message over the network, msgid=IdRemovePlayer.
     *
     * As soon as this message is received by @ref networkTransmission @ref
     * systemRemovePlayer is called and the player is removed.
     **/
    //AB: TODO: make sendMessage to return if the message will be able to be
    //sent, eg if a socket is connected, etc. If sendMessage returns false
    //remove the player directly using systemRemovePlayer
    bool removePlayer(KPlayer * player) { return removePlayer(player, 0); }

    /**
     * Called by the desvtuctor of KPlayer to remove itself from the game
     *
     **/
    void playerDeleted(KPlayer * player);

    /**
     * sends activate player: interal use only?
     */
    bool activatePlayer(KPlayer *player);

    /**
     * sends inactivate player: interal use only?
     */
    bool inactivatePlayer(KPlayer *player);

    /**
     * Set the maximal number of players. After this is
     * reached no more players can be added. You must be ADMIN to call this (see
     * @ref isAdmin)! 
     * @param maxnumber maximal number of players
     */
    void setMaxPlayers(uint maxnumber);
    
    /**
     * What is the maximal number of players?
     * @return maximal number of players
     */
    int maxPlayers() const;
    
    /**
     * Set the minimal number of players. A game can not be started
     * with less player resp. is paused when already running. You must be ADMIN
     * to call this (see @ref isAdmin)!
     * @param minnumber minimal number of players
     */
    void setMinPlayers(uint minnumber);

    /**
     * What is the minimal number of players?
     * @return minimal number of players
     */
    uint minPlayers() const;

    /**
     * Returns how many players are plugged into the game
     * @return number of players
     */
    uint playerCount() const;

    /**
     * Select the next player in a turn based game. In an asynchronous game this
     * function has no meaning. Overwrite this function for your own game sequence.
     * Per default it selects the next player in the playerList
     */
    virtual bool nextPlayer(KPlayer *last,bool exclusive=true);

    // Input events
    /**
     * Called by @ref KPlayer to send a player input to the @ref
     * KMessageServer.
     **/
    virtual bool sendPlayerInput(QDataStream &msg,KPlayer *player,Q_UINT32 sender=0);

    /**
     * Called when a player input arrives from @ref KMessageServer.
     *
     * Calls @ref prepareNext (using @ref QTimer::singleShot) if @ref gameOver
     * returns 0
     * TODO: documentation!!
     **/
    virtual bool playerInput(QDataStream &msg,KPlayer *player,Q_UINT32 sender=0);

    // load/save
    /**
     * Load a saved game, from file OR network. This function has
     * to be overwritten or you need to connect to the load signal
     * if you have game data other than KGameProperty.
     * For file load you should reset() the game before any load attempt
     * to make sure you load into an clear state.
     *
     * @param stream a data stream where you can stream the game from
     *
     * @return true?
     */
    virtual bool load(QDataStream &stream);

    /**
     * Same as above function but with different parameters
     *
     * @param the filename of the file to be opened
     *
     * @return true?
     **/
    virtual bool load(QString filename);
    
    /**
     * Save a game to a file OR to network. Otherwise the same as 
     * the load function
     *
     * @param stream a data stream to load the game from
     * @param saveplayers If true then all players wil be saved too
     *
     * @return true?
     */
    virtual bool save(QDataStream &stream,bool saveplayers=true);

    /**
     * Same as above function but with different parameters
     *
     * @param the filename of the file to be saved
     * @param saveplayers If true then all players wil be saved too
     *
     * @return true?
     **/
    virtual bool save(QString filename,bool saveplayers=true);

    /**
     * Resets the game, i.e. puts it into a state where everything
     * can be started from, e.g. a load game
     * Right now it does only need to delete all players
     *
     * @return true on success
     */
    virtual bool reset();


    // Game sequence
    /**
     * returns the game status, ie running,pause,ended,...
     *
     * @return game status
     */
    int gameStatus() const;
    
    /**
     * sets the game status
     *
     * @param status the new status
     * @param transmit command over network
     */
    virtual void setGameStatus(int status);

    //AB: docu: see @ref KPlayer
    bool addProperty(KGamePropertyBase* data);

    /**
     * This is called by @ref KPlayer::sendProperty only! Internal function!
     **/
    bool sendPlayerProperty(QDataStream& s, Q_UINT32 playerId);
    
    KGamePropertyBase* findProperty(int id) const;

    /**
     * Changes the consistency policy of a property. The @ref 
     * GamePolicy is one of PolicyClean (defaulz), PolicyDirty or PolicyLocal.
     *
     * It is up to you to decide how you want to work. 
     **/
    void setPolicy(GamePolicy p,bool recursive=true);

    /**
     * @return The default policy of the property
     **/
    GamePolicy policy() const;

    /**
     * See @ref KGameNetwork::sendMessage
     *
     * Send a network message msg with a given message ID msgid to all players of
     * a given group (see @ref KPlayer::group)
     * @param msg the message which will be send. See messages.txt for contents
     * @param msgid an id for this message
     * @param group the group of the receivers
     * @return true if worked
     */
    bool sendGroupMessage(const QByteArray& msg, int msgid, Q_UINT32 sender, const QString& group);
    bool sendGroupMessage(const QDataStream &msg, int msgid, Q_UINT32 sender, const QString& group);
    bool sendGroupMessage(int msg, int msgid, Q_UINT32 sender, const QString& group);
    bool sendGroupMessage(const QString& msg, int msgid, Q_UINT32 sender, const QString& group);

    /**
     * This will either forward an incoming message to a specified player 
     * (see @ref KPlayer::networkTransmission) or
     * handle the message directly (e.g. if msgif==IdRemovePlayer it will remove
     * the (in the stream) specified player). If both is not possible (i.e. the
     * message is user specified data) the signal @ref signalNetworkData is
     * emitted.
     *
     * This emits @ref signalMessageUpdate <em>before</em> doing anything with
     * the message. You can use this signal when you want to be notified about
     * an update/change.
     * @param msgid Specifies the kind of the message. See messages.txt for
     * further information
     * @param stream The message that is being sent
     * @param receiver The is of the player this message is for. 0 For broadcast.
     * @param sender
     * @param clientID the client from which we received the transmission - hardly used
     **/
    virtual void networkTransmission(QDataStream &stream, int msgid, Q_UINT32 receiver, Q_UINT32 sender, Q_UINT32 clientID);

    /**
     * Returns a pointer to the KGame property handler
     **/
    KGamePropertyHandler* dataHandler() const;

protected slots:
    /**
     * Called by @ref KGameProperty only! Internal function!
     **/
    void sendProperty(QDataStream& stream, bool& sent);

    /**
      * Called by @ref KGameProperty only! Internal function!
     **/
    void emitSignal(KGamePropertyBase *me);

    /**
     * Prepare the call of next() after a QTimer event to allow QT Event processing
     */
    void prepareNext();

    /**
     * Calls @ref negotiateNetworkGame
     * See @ref KGameNetwork::signalClientConnected
     **/
    void slotClientConnected(Q_UINT32 clientId);

    /**
     * This slot is called whenever the connection to a client is lost (ie the
     * signal @ref KGameNetwork::signalClientDisconnected is emitted) and will remove
     * the players from that client.
     * @param client The client the connection has been lost to
     **/
    void slotClientDisconnected(Q_UINT32 clientId,bool broken);

    /**
     * This slot is called whenever the connection to the server is lost (ie the
     * signal @ref KGameNetwork::signalConnectionBroken is emitted) and will
     * switch to local game mode
     **/
    void slotServerDisconnected();

signals:
    /**
     * A player input occured. This is the most important function
     * as the given message will contain the current move made by
     * the given player.
     * Example:
     * <pre>
     * void MyClass::slotPlayerInput(QDataStream &msg,KPlayer *player)
     * {
     *   Q_INT32 move;
     *   msg >>  move;
     *   kdDebug() << "  Player " << player->id() << " moved to " << move <<
     *   endl;
     * }
     * </pre>
     *
     * @param msg the move message
     * @param player the player who did the move
     */
    void signalPlayerInput(QDataStream &msg,KPlayer *player);
    
    /**
     * the game will be loaded from the given stream. We better
     * fill our data.
     *
     * @param stream the load stream
     */
    void signalLoad(QDataStream &stream);
    
    /**
     * the game will be saved to the given stream. We better
     * put our data.
     *
     * @param stream the save stream
     */
    void signalSave(QDataStream &stream);

    /**
     * We got an user defined update message. This is usually done
     * by a sendData in a inherited KGame Object which defines its
     * own methods and has to syncronise them over the network.
     * Reaction to this is usually a call to a KGame function.
     */
    void signalNetworkData(int msgid,const QByteArray& buffer, Q_UINT32 receiver, Q_UINT32 sender);

    /**
     * We got an network message. this can be used to notify us that something
     * changed. What changed can be seen in the message id. Whether this is
     * the best possible method to do this is unclear...
     */
    void signalMessageUpdate(int msgid,Q_UINT32 receiver,Q_UINT32 sender);
    
    /**
     * a player left the game because of a broken connection!
     *
     * @param player the player who left the game
     */
    void signalPlayerLeftGame(KPlayer *player);
    
    /**
     * a player joined the game
     *
     * @param player the player who joined the game
     */
    void signalPlayerJoinedGame(KPlayer *player);

    /**
    * This signal is emmited if the KGame needs to create a new player.
    * Currently this happens only over a network connection. Doing nothing
    * will create a default KPlayer. If you want to have your own player
    * you have to create one with the given rtti here.
    *
    * @param player is the player object which got created
    * @param rtti is the type of the player (0 means default KPlayer)
    * @param io is the 'or'ed rtti of the KGameIO's
    */
    void signalCreatePlayer(KPlayer *&player,int rtti,int io,bool isvirtual,KGame *me);

    /**
    * This signal is emmited if a player property changes its value and
    * the property is set to notify this change
    */
    void signalPropertyChanged(KGamePropertyBase *property, KGame *me);

    /**
     * Is emitted after a call to gameOver() returns a non zero
     * return code. This code is forwarded to this signal as 'status'.
     *
     * @param status the return code of gameOver()
     * @param current the player who did the last move
     * @param me a pointer to the KGame object
     */
    void signalGameOver(int status, KPlayer *current, KGame *me);
     
    /**
    * Is emmited after a client is sucessfully connected to the game.
    * The client id is the id of the new game client. An easy way to
    * check whether that's us is 
    * <pre>
    *   if (clientid==gameid()) .. // we joined
    *   else ... // someone joined the game
    * </pre>
    * @param clientid - The id of the new client
    * @param me - our game pointer
    */
    void signalClientConnected(Q_UINT32 clientid,KGame *me);


protected:

    /**
    * This virtual function can be overwritten for your own player managment.
    * It is called when a new game connects to an existing network game or
    * to the network master. In case you do not want all players of both games
    * to be present in the new network game, you can deactivate players here.
    * This is of particular importance if you have a game with fixed number of
    * player like e.g. chess. A network connect needs to disable one player of
    * each game to make sense.
    * 
    * Not overwriting this function will activate a default behaviour which
    * will deactivate players until the @ref maxPlayers() numebr is reached
    * according to the KPlayer::networkPriority() value. Players with a low
    * value will be kicked out first. With equal priority players of the new
    * client will leave first. This means, not setting this value and not
    * overwriting this function will never allow a chess game to add client
    * players!!! 
    * On the other hand setting one player of each game to a networkPriorty of
    * say 10, already does most of the work for you.
    *
    * The parameters of this functin are the playerlist of the network game,
    * which is @ref playerList(). The second argument is the player list of
    * the new client who wants to join and the third argument serves as return
    * parameter. All <em>player ID's</em> which are written into this list
    * will be <em>removed</em> from the created game. You do this by an
    * <pre>
    * inactivate.append(player->id());
    * </pre>
    * 
    * @param oldplayer - the list of the network players
    * @param newplayer - the list of the client players
    * @param inactivate - the value list of ids to be deactivated
    *
    **/ 
    virtual void newPlayersJoin(KGamePlayerList *,KGamePlayerList *,QValueList<int> &) {};

    /**
    * Save the player list to a stream. Used for network game and load/save.
    * Can be overwritten if you know what you are doing
    *
    * @param stream is the stream to save the player to
    * @param the optional list is the player list to be saved, default is playerList()
    *
    **/
    void savePlayers(QDataStream &stream,KGamePlayerList *list=0);

    /**
     * Prepare a player for being added. Put all data about a player into the
     * stream so that it can be sent to the @ref KGameCommunicationServer using
     * @ref addPlayer (e.g.)
     *
     * This function ensures that the code for adding a player is the same in
     * @ref addPlayer as well as in @ref negotiateNetworkGame
     * @param receiver The owner of the player
     **/
    void savePlayer(QDataStream& stream,KPlayer* player);

    /**
    * Load the player list from a stream. Used for network game and load/save.
    * Can be overwritten if you know what you are doing
    *
    * @param stream is the stream to save the player to
    * @param isvirtual will set the virtual flag true/false
    *
    **/
    KPlayer *loadPlayer(QDataStream& stream,bool isvirtual=false);
    

    /**
     * inactivates player. Use @ref inactivatePlayer instead!
     */
    bool systemInactivatePlayer(KPlayer *player);

    /**
     * activates player. Use @ref activatePlayer instead!
     */
    bool systemActivatePlayer(KPlayer *player);

    /**
     * Adds a player to the game
     *
     * Use @ref addPlayer to send @ref KGameMessage::IdAddPlayer. As soon as
     * this Id is received this function is called, where the player (see @ref
     * KPlayer::rtti) is added as well as its properties (see @ref KPlayer::save
     * and @ref KPlayer::load)
     *
     * This method calls the overloaded @ref systemAddPlayer with the created
     * player as argument. That method will really add the player.
     * If you need to do some changes to your newly added player just connect to
     * @ref signalPlayerJoinedGame
     */

    /**
     * Finally adds a player to the game and therefore to the list.
     **/
    void systemAddPlayer(KPlayer* newplayer);

    /**
     * Removes a player from the game
     *
     * Use @ref removePlayer to send @ref KGameMessage::IdRemovePlayer. As soon
     * as this Id is received systemRemovePlayer is called and the player is
     * removed directly.
     *
     **/
    void systemRemovePlayer(KPlayer* player,bool deleteit);
    
    /**
     * This member function will transmit e.g. all players to that client, as well as
     * all properties of these players (at least if they have been added by
     * @ref KPlayer::addProperty) so that the client will finally have the same
     * status as the master. You want to overwrite this function if you expand
     * KGame by any properties which have to be known by all clients.
     *
     * Only the ADMIN is allowed to call this.
     * @param clientID The ID of the message client which has connected
     **/
    virtual void negotiateNetworkGame(Q_UINT32 clientID);

    /**
     * syncronise the random numbers with all network clients
     * not used by KGame - if it should be kept then as public method
     */
    void syncRandom();

    void deletePlayers();
    void deleteInactivePlayers();

    /**
     * Check whether the game is over.
     *
     * @param player the player who made the last move
     * @return anything else but 0 is considered as game over
     */
    virtual int gameOver(KPlayer *player);

    /**
     * Load a saved game, from file OR network. Internal
     *
     * @param stream a data stream where you can stream the game from
     * @param is it a network call -> make players virtual
     *
     * @return true?
     */
    virtual bool loadgame(QDataStream &stream, bool network);

private:
    //AB: this is to hide the "receiver" parameter from the user. It shouldn't be
    //used if possible (except for init).
    /**
     * This is an overloaded function. Id differs from the public one only in
     * its parameters:
     *
     * @param receiver The Client that will receive the message. You will hardly
     * ever need this. It it internally used to initialize a newly connected
     * client.
     **/
    //void addPlayer(KPlayer* newplayer, Q_UINT32 receiver);

    /**
     * Just the same as the public one except receiver:
     * @param receiver 0 for broadcast, otherwise the receiver. Should only be
     * used in special circumstances and not outside KGame.
     **/
    bool removePlayer(KPlayer * player, Q_UINT32 receiver);

    /**
     * Helping function - game negotiation
     **/
    void setupGame(Q_UINT32 sender);

    /**
     * Helping function - game negotiation
     **/
    void setupGameContinue(QDataStream& msg, Q_UINT32 sender);

    /**
     * Removes a player from all lists, removes the @ref KGame pointer from the
     * @ref KPlayer and deletes the player. Used by (e.g.) @ref
     * systemRemovePlayer
     * @return True if the player has been removed, false if the current is not
     * found
     **/
    bool systemRemove(KPlayer* player,bool deleteit);

    
private:
    KGamePrivate* d;
};

#endif
