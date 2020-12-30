/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KGAMENETWORK_H_
#define __KGAMENETWORK_H_

// own
#include "libkdegamesprivate_export.h"
// Qt
#include <QString>
#include <QObject>

class KGameIO;
class KMessageIO;
class KMessageClient;
class KMessageServer;

class KGameNetworkPrivate;

/**
 * \class KGameNetwork kgamenetwork.h <KGame/KGameNetwork>
 *
 * The KGameNetwork class is the KGame class with network
 * support. All other features are the same but they are
 * now network transparent. It is not used directly but
 * only via a KGame object. So you do not really have
 * to bother with this object.
 *
 * @short The main KDE game object
 */
class KDEGAMESPRIVATE_EXPORT KGameNetwork : public QObject
{
  Q_OBJECT

public:
    /**
     * Create a KGameNetwork object
     */
    explicit KGameNetwork(int cookie=42,QObject* parent=nullptr);
    virtual ~KGameNetwork();

    /**
     * Gives debug output of the game status
     **/
    virtual void Debug();

    /**
     * @return TRUE if this is a network game - i.e. you are either MASTER or
     * connected to a remote MASTER.
     **/
    bool isNetwork() const;

    /**
     * Is this the game MASTER (i.e. has started theKMessageServer). A
     * game has always exactly one MASTER. This is either a KGame object (i.e. a
     * Client) or an own MessageServer-process. A KGame object that has the
     * MASTER status is always admin.
     *
     * You probably don't want to use this. It is a mostly internal method which
     * will probably become protected. Better use isAdmin
     *
     * @see isAdmin
     * @return Whether this client has started the KMessageServer
     **/
    bool isMaster() const;

    /**
     * The admin of a game is the one who initializes newly connected clients
     * using  negotiateNetworkGame and is allowed to configure the game.
     * E.g. only the admin is allowed to use KGame::setMaxPlayers.
     *
     * If one KGame object in the game is MASTER then this client is the admin
     * as well. isMaster and isAdmin differ only if the KMessageServer
     * is running in an own process.
     * @return Whether this client (KGame object) is the admin
     **/
    bool isAdmin() const;

    /**
     * The unique ID of this game
     *
     * @return int id
     **/
    quint32 gameId() const;

    /**
     * Inits a network game as network MASTER. Note that if the
     * KMessageServer is not yet started it will be started here (see
     * setMaster). Any existing connection will be disconnected.
     *
     * If you already offer connections the port is changed.
     *
     * @param port The port on which the service is offered
     * @return true if it worked
     **/
    bool offerConnections (quint16 port);

    void setDiscoveryInfo(const QString& type, const QString& name=QString());

    /**
     * Inits a network game as a network CLIENT
     *
     * @param host the host to which we want to connect
     * @param port the port we want to connect to
     *
     * @return true if connected
     **/
    bool connectToServer(const QString& host, quint16 port);
    bool connectToServer(KMessageIO *connection);

    /**
     * @return The port we are listening to if offerConnections was called
     * or the port we are connected to if connectToServer was called.
     * Otherwise 0.
     **/
    quint16 port() const;

    /**
     * @return The name of the host that we are currently connected to is
     * isNetwork is TRUE and we are not the MASTER, i.e. if connectToServer
     * was called. Otherwise this will return "localhost".
     **/
    QString hostName() const;

    /**
     * Stops offering server connections - only for game MASTER
     * @return true
     **/
    bool stopServerConnection();

    /**
     * Changes the maximal connection number of the KMessageServer to max.
     * -1 Means infinite connections are possible. Note that existing
     * connections are not affected, so even if you set this to 0 in a running
     * game no client is being disconnected. You can call this only if you are
     * the ADMIN!
     *
     * @see KMessageServer::setMaxClients
     * @param max The maximal number of connections possible.
     **/
    void setMaxClients(int max);

    //AB: is this now internal only? Can we make it protected (maybe with
    //friends)? sendSystemMessage AND sendMessage is very confusing to the
    //user.
    /**
     * Sends a network message msg with a given msg id msgid to all clients.
     * Use this to communicate with KGame (e.g. to add a player ot to configure
     * the game - usually not necessary).
     *
     * For your own messages use  sendMessage instead! This is mostly
     * internal!
     *
     * @param buffer the message which will be send. See messages.txt for contents
     * @param msgid an id for this message. See
     * KGameMessage::GameMessageIds
     * @param receiver the KGame / KPlayer this message is for.
     * @param sender The KGame / KPlayer this message is from (i.e.
     * you). You
     * probably want to leave this 0, then KGameNetwork will create the correct
     * value for you. You might want to use this if you send a message from a
     * specific player.
     * @return true if worked
     */
    // AB: TODO: doc on how "receiver" and "sender" should be created!
    bool sendSystemMessage(const QByteArray& buffer, int msgid, quint32 receiver=0, quint32 sender=0);

    /**
     * @overload
     **/
    bool sendSystemMessage(int data, int msgid, quint32 receiver=0, quint32 sender=0);

    /**
     * @overload
     **/
    bool sendSystemMessage(const QDataStream &msg, int msgid, quint32 receiver=0, quint32 sender=0);

    /**
     * @overload
     **/
    bool sendSystemMessage(const QString& msg, int msgid, quint32 receiver=0, quint32 sender=0);

    /**
     * Sends a network message
     * @param error The error code
     * @param message The error message - use KGameError
     * @param receiver the KGame / KPlayer this message is for. 0 For
     * all
     * @param sender The KGame / KPlayer this message is from (i.e.
     * you). You probably want to leave this 0, then KGameNetwork will create
     * the correct value for you. You might want to use this if you send a
     * message from a specific player.
     **/
    void sendError(int error, const QByteArray& message, quint32 receiver=0, quint32 sender=0);

    /**
     * Are we still offer offering server connections - only for game MASTER
     * @return true/false
     **/
    bool isOfferingConnections() const;

    /**
     * Application cookie. this identifies the game application. It
     * help to distinguish between e.g. KPoker and KWin4
     * @return the application cookie
     **/
    int cookie() const;

    /**
     * Send a network message msg with a given message ID msgid to all clients.
     * You want to use this to send a message to the clients.
     *
     * Note that a message is always sent to ALL clients! This is necessary so
     * that all clients always have the same data and can easily be changed from
     * network to non-network without restarting the game. If you want a
     * specific KGame / KPlayer to react to the message use the
     * receiver and sender parameters. See KGameMessage::calsMessageId
     *
     * SendMessage differs from sendSystemMessage only by the msgid parameter.
     * sendSystemMessage is thought as a KGame only method while
     * sendMessage is for public use. The msgid parameter will be
     * +=KGameMessage::IdUser and in KGame::signalNetworkData msgid will
     * be -= KGameMessage::IdUser again, so that one can easily distinguish
     * between system and user messages.
     *
     * Use sendSystemMessage to communicate with KGame (e.g. by adding a
     * player) and sendMessage for your own user message.
     *
     * Note: a player should send messages through a KGameIO!
     *
     * @param buffer the message which will be send. See messages.txt for contents
     * @param msgid an id for this message. See KGameMessage::GameMessageIds
     * @param receiver the KGame / KPlayer this message is for.
     * @param sender The KGame / KPlayer this message is from (i.e.
     * you). You
     * probably want to leave this 0, then KGameNetwork will create the correct
     * value for you. You might want to use this if you send a message from a
     * specific player.
     * @return true if worked
     **/
    // AB: TODO: doc on how "receiver" and "sender" should be created!
    bool sendMessage(const QByteArray& buffer, int msgid, quint32 receiver=0, quint32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendMessage(const QDataStream &msg, int msgid, quint32 receiver=0, quint32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendMessage(const QString& msg, int msgid, quint32 receiver=0, quint32 sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendMessage(int data, int msgid, quint32 receiver=0, quint32 sender=0);


    /**
     * Called by ReceiveNetworkTransmission(). Will be overwritten by
     * KGame and handle the incoming message.
     **/
    virtual void networkTransmission(QDataStream&, int, quint32, quint32, quint32 clientID) = 0;


    /**
     * Disconnect the current connection and establish a new local one.
     **/
    void disconnect();


    /**
     * If you are the ADMIN of the game you can give the ADMIN status away to
     * another client. Use this e.g. if you want to quit the game or if you want
     * another client to administrate the game (note that disconnect calls
     * this automatically).
     * @param clientID the ID of the new ADMIN (note: this is the _client_ID
     * which has nothing to do with the player IDs. See KMessageServer)
     **/
    void electAdmin(quint32 clientID);

    /**
     * Don't use this unless you really know what you're doing! You might
     * experience some strange behaviour if you send your messages directly
     * through the KMessageClient!
     *
     * @return a pointer to the KMessageClient used internally to send the
     * messages. You should rather use one of the send functions!
     **/
    KMessageClient* messageClient() const;

    /**
     * Don't use this unless you really know what you are doing! You might
     * experience some strange behaviour if you use the message server directly!
     *
     * @return a pointer to the message server if this is the MASTER KGame
     * object. Note that it might be possible that no KGame object contains
     * the KMessageServer at all! It might even run stand alone!
     **/
    KMessageServer* messageServer() const;

    /**
     * You should call this before doing thigs like, e.g. qApp->processEvents().
     * Don't forget to call unlock once you are done!
     *
     * @see KMessageClient::lock
     **/
    virtual void lock();

    /**
     * @see KMessageClient::unlock
     **/
    virtual void unlock();

Q_SIGNALS:
    /**
     * A network error occurred
     * @param error the error code
     * @param text the error text
     */
    void signalNetworkErrorMessage(int error, const QString &text);

    /**
     * Our connection to the KMessageServer has broken.
     * See KMessageClient::connectionBroken
     **/
    void signalConnectionBroken();

    /**
     * This signal is emitted whenever the KMessageServer sends us a message that a
     * new client connected. KGame uses this to call KGame::negotiateNetworkGame
     * for the newly connected client if we are admin (see isAdmin)
     *
     * @see KMessageClient::eventClientConnected
     *
     * @param clientID the ID of the newly connected client
     **/
    void signalClientConnected(quint32 clientID);

    /**
     * This signal is emitted whenever the KMessageServer sends us a message
     * that a connection to a client was detached. The second parameter can be used
     * to distinguish between network errors or removing on purpose.
     *
     * @see KMessageClient::eventClientDisconnected
     *
     * @param clientID the client that has disconnected
     * @param broken true if the connection was lost because of a network error, false
     *        if the connection was closed by the message server admin.
     */
    void signalClientDisconnected(quint32 clientID, bool broken);

    /**
     * This client gets or loses the admin status.
     * @see KMessageClient::adminStatusChanged
     * @param isAdmin True if this client gets the ADMIN status otherwise FALSE
     **/
    void signalAdminStatusChanged(bool isAdmin);

protected:
    /**
     * @internal
     * Start a KMessageServer object and use it as the MASTER of the game.
     * Note that you must not call this if there is already another master
     * running!
     **/
    void setMaster();

protected Q_SLOTS:
    /**
     * Called by KMessageClient::broadcastReceived() and will check if the
     * message format is valid. If it is not, it will generate an error (see
     * signalNetworkVersionError and signalNetworkErorrMessage).
     * If it is valid, the pure virtual method networkTransmission() is called.
     * (This one is overwritten in KGame.)
     **/
    void receiveNetworkTransmission(const QByteArray& a, quint32 clientID);

    /**
     * This KGame object receives or loses the admin status.
     * @param isAdmin Whether we are admin or not
     **/
    void slotAdminStatusChanged(bool isAdmin);

    /**
     * Called when the network connection is about to terminate. Is used
     * to store the network parameter like the game id
     */
     void aboutToLoseConnection(quint32 id);

    /**
     * Called when the network connection is terminated. Used to clean
     * up the disconnect parameter
     */
     void slotResetConnection();


private:
     void tryPublish();
     void tryStopPublishing();
     KGameNetworkPrivate* const d;
};

#endif
