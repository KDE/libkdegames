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
#include <qptrlist.h>
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
 * - nextPlayer and gameOver()
 * - Network connection (for KGameNetwork)
 *
 * Example:
 * <pre>
 * KGame *game=new KGame;
 * </pre>
 *
 *
 * @short The main KDE game object
 * @author Martin Heni <martin@heni-online.de>
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
     * Create a KGame object. The cookie is used to identify your
     * game in load/save and network operations. Change this between
     * games.
     */
    KGame(int cookie=42,QObject* parent=0);

    /**
    * Destructs the game
    */
    virtual ~KGame();

    /**
     * Gives debug output of the game status
     */
    virtual void Debug();

    /**
     * Game status - Use this to Control the game flow.
     * The KGame e.g. sets the status to Pause when you have
     * less player than the minimum amount
     */
	enum GameStatus
	{
		Init = 0,
		Run = 1,
		Pause = 2,
		End = 3,
		Abort = 4,
		SystemPause = 5,
		Intro = 6,
		UserStatus = 7
	};

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
     * Called by the destructor of KPlayer to remove itself from the game
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
    virtual KPlayer * nextPlayer(KPlayer *last,bool exclusive=true);

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
     * returns 0. This function should normally not be used outside KGame.
     * It could be made non-virtual,protected in a later version. At the
     * moment it is a virtual function to give you more control over KGame.
     *
     * For documentation see @ref signalPlayerInput.
     **/
    virtual bool systemPlayerInput(QDataStream &msg,KPlayer *player,Q_UINT32 sender=0);

    /**
    * This virtual function is called if the KGame needs to create a new player.
    * This happens only over a network and with load/save. Doing nothing
    * will create a default KPlayer. If you want to have your own player
    * you have to create one with the given rtti here.
    * Note: If your game uses a player class derived from KPlayer you MUST
    * overwrite this function and create your player here. Otherwise the
    * game will crash.
    * Example:
    * <pre>
    *  KPlayer *MyGame::createPlayer(int rtti,int io,bool isvirtual)
    *  {
    *    KPlayer *player=new MyPlayer;
    *    if (!isvirtual) // network player ?
    *    {
    *      // Define something like this to add the IO modules
    *      createIO(player,(KGameIO::IOMode)io);
    *    }
    *    return player;
    *    }
    * </pre>
    *
    * @param player is the player object which got created
    * @param rtti is the type of the player (0 means default KPlayer)
    * @param io is the 'or'ed rtti of the KGameIO's
    */
    virtual KPlayer *createPlayer(int rtti,int io,bool isvirtual);

    // load/save
    /**
     * Load a saved game, from file OR network. This function has
     * to be overwritten or you need to connect to the load signal
     * if you have game data other than KGameProperty.
     * For file load you should reset() the game before any load attempt
     * to make sure you load into an clear state.
     *
     * @param stream a data stream where you can stream the game from
     * @param reset - shall the game be reset before loading
     *
     * @return true?
     */
    virtual bool load(QDataStream &stream,bool reset=true);

    /**
     * Same as above function but with different parameters
     *
     * @param filename - the filename of the file to be opened
     * @param reset - shall the game be reset before loading
     *
     * @return true?
     **/
    virtual bool load(QString filename,bool reset=true);

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
    void setGameStatus(int status);

    /**
    *  docu: see @ref KPlayer
    **/
    bool addProperty(KGamePropertyBase* data);

    /**
     * This is called by @ref KPlayer::sendProperty only! Internal function!
     **/
    bool sendPlayerProperty(int msgid, QDataStream& s, Q_UINT32 playerId);

    /**
    * This function allows to find the pointer to a player
    * property when you know it's id
    */
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
     * Called by @ref KGamePropertyHandler only! Internal function!
     **/
    void sendProperty(int msgid, QDataStream& stream, bool* sent);

    /**
      * Called by @ref KGamePropertyHandler only! Internal function!
     **/
    void emitSignal(KGamePropertyBase *me);

    /**
     * Prepare the call of nextPlayer() after a QTimer event to allow QT Event processing
     */
    virtual void prepareNext();


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
     * When a client disconnects from the game usually all players from that
     * client are removed. But if you use completely the KGame structure you
     * probably don't want this. You just want to replace the KGameIO of the
     * (human) player by a computer KGameIO. So this player continues game but
     * is from this point on controlled by the computer.
     *
     * You achieve this by connecting to this signal. It is emitted as soon as a
     * client disconnects on <em>all</em> other clients. Make sure to add a new
     * KGameIO only once! you might want to use @ref isAdmin for this. If you
     * added a new KGameIO set *remove=false otherwise the player is completely
     * removed.
     * @param player The player that is about to be removed. Add your new
     * KGameIO here - but only on <em>one</em> client!
     * @param remove Set this to FALSE if you don't want this player to be
     * removed completely.
     **/
    void signalReplacePlayerIO(KPlayer* player, bool* remove);
    
    /**
     * The game will be loaded from the given stream. Load from here
     * the data which is NOT a game or player property.
     * It is not necessary to use this signal for a full property game.
     *
     * @param stream the load stream
     */
    void signalLoad(QDataStream &stream);

    /**
     * The game will be saved to the given stream. Fill this with data
     * which is NOT a game or player property.
     * It is not necessary to use this signal for a full property game.
     *
     * @param stream the save stream
     */
    void signalSave(QDataStream &stream);

    /**
     * Is emmited if a game with a different version cookie is loaded.
     * Normally this should result in an error. But maybe you do support
     * loading of older game versions. Here would be a good place to do a
     * conversion.
     *
     * @param stream - the load stream
     * @param network - true if this is a network connect. False for load game
     * @param cookie - the saved cookie. It differs from KGame::cookie()
     * @param result - set this to true if you managed to load the game
     */
    void signalLoadError(QDataStream &stream,bool network,int cookie, bool &result);

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
     * a player left the game because of a broken connection or so!
     *
     * Note that when this signal is emitted the player is not part of @ref
     * playerList anymore but the pointer is still valid. You should do some
     * final cleanups here since the player is usually deleted after the signal
     * is emitted.
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
    void signalClientJoinedGame(Q_UINT32 clientid,KGame *me);

    /**
    * This signal is emmited after a network partner left the
    * game (either by a broken connection of voluntarily).
    * All changes to the network players have already be done.
    * If there are not enough players left, the game might have
    * been paused. To check this you get the old gamestatus
    * before the disconnection as argument here. The id of the
    * client who left the game allows to distinguish who left the
    * game. If it is 0, the server disconnected and you were a client
    * which has been switched back to local play.
    * You can use this signal to, e.g. set some menues back to local
    * player when they were network before.
    *
    * @param clientId - 0:server left, otherwise the client who left
    * @parma oldgamestatus - the gamestatus before the loss
    * @param me - our game pointer
    **/
    void signalClientLeftGame(int clientID,int oldgamestatus,KGame *me);


protected:
    /**
     * A player input occured. This is the most important function
     * as the given message will contain the current move made by
     * the given player.
     * Note that you HAVE to overwrite this function. Otherwise your
     * game makes no sense at all.
     * Generally you have to return TRUE in this function. Only then
     * the game sequence is proceeded by calling @ref playerInputFinished
     * which in turn will check for game over or the next player
     * However, if you have a delayed move, because you e.g. move a
     * card or a piece you want to return FALSE to pause the game sequence
     * and then manually call @ref playerInputFinished to resume it.
     * Example:
     * <pre>
     * bool MyClass::playerInput(QDataStream &msg,KPlayer *player)
     * {
     *   Q_INT32 move;
     *   msg >>  move;
     *   kdDebug() << "  Player " << player->id() << " moved to " << move <<
     *   endl;
     *   return true;
     * }
     * </pre>
     *
     * @param msg the move message
     * @param player the player who did the move
     * @return true - input ready, false: input manual
     */
    virtual bool playerInput(QDataStream &msg,KPlayer *player)=0;


    /**
    * Called after the player input is processed by the game. Here the
    * checks for game over and nextPlayer (in the case of turn base games)
    * are processed.
    * Call this manually if you have a delayed move, i.e. your playerInput
    * function returns FALSE. If it returns true you need not do anything
    * here.
    *
    * @return the current player
    *
    **/
    KPlayer *playerInputFinished(KPlayer *player);


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
    virtual int checkGameOver(KPlayer *player);

    /**
     * Load a saved game, from file OR network. Internal. 
     * Warning: loadgame must not rely that all players all already
     * activated. Actually the network will activate a player AFTER
     * the loadgame only. This is not true anymore. But be careful 
     * anyway.
     *
     * @param stream a data stream where you can stream the game from
     * @param is it a network call -> make players virtual
     * @param reset - shall the game be reset before loading
     *
     * @return true?
     */
    virtual bool loadgame(QDataStream &stream, bool network,bool reset);

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
