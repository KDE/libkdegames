/* **************************************************************************
                           KGameNetwork Class
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
#ifndef __KGAMENETWORK_H_
#define __KGAMENETWORK_H_

#include <qstring.h>
#include <qobject.h>
#include <qptrdict.h>


class KGameIO;

class KGameNetworkPrivate;

/**
 * The KGameNetwork class is the KGame class with network
 * support. All other features are the same but they are
 * now network transparent. It is not used directly but
 * only via a KGame
 *
 * @short The main KDE game object
 * @author Martin Heni <martin@heni-online.de>
 * @version $Id$
 *
 */
class KGameNetwork : public QObject
{
  Q_OBJECT

public:
    /**
     * Create a KGameNetwork object
     */
    KGameNetwork(int cookie=42,QObject* parent=0);
    ~KGameNetwork();

    //enum PlayerId {IdBroadcast=-1,IdClient=0};

    /**
     * Gives debug output of the game status
     **/
    virtual void Debug();

    /**
     * Is this the game MASTER (i.e. has started the @ref KMessageServer). A
     * game has always exactly one MASTER. This is either a KGame object (i.e. a
     * Client) or an own MessageServer-process. A KGame object that has the
     * MASTER status is always admin.
     *
     * You probably don't want to use this. It is a mostly internal method which
     * will probably become protected. Better use @ref isAdmin
     *
     * See also @ref isAdmin
     * @return Whether this client has started the @ref KMessageServer
     **/
    bool isMaster() const;

    /**
     * The admin of a game is the one who initializes newly connected clients
     * using @ref negotiateNetworkGame and is allowed to configure the game.
     * E.g. only the admin is allowed to use @ref KGame::setMaxPlayers.
     *
     * If one KGame object in the game is MASTER then this client is the admin
     * as well. @ref isMaster and isAdmin differ only if the @ref KMessageServer
     * is running in an own process.
     * @ref Whether this client (KGame object) is the admin
     **/
    bool isAdmin() const;

    /**
     * The unique ID of this game
     *
     * @return int id
     **/
    Q_UINT32 gameId() const;

    /**
     * Inits a network game as network MASTER
     *
     * @param The port on which the service is offered
     * @return true if it worked
     **/
    bool offerConnections (Q_UINT16 port);

    /**
     * Inits a network game as a network CLIENT
     *
     * @param host the host to which we want to connect
     * @param port the port we want to connect to
     *
     * @return true if connected
     **/
    bool connectToServer(const QString& host, Q_UINT16 port);

    /**
     * Stops offering server connections - only for game MASTER
     * @return true
     **/
    bool stopServerConnection();

    /**
     * Sends an integer command  over the network. This is used to synchronise
     * function calls between the clients
     *
     * @param msgid to identify the data
     * @param data the integer data
     * @param client=0 if given then send only to a given client
     *
     * @return error status
     **/
    bool sendSystemMessage(int data, int msgid, int receiver=0,int sender=0);

    /**
     * Sends a network message msg with a given msg id msgid to either all clients
     * or a given client or all clients but a given client.
     * Use this to communicate with KGame (e.g. to add a player ot to cunfigure
     * the game - usually not necessary). For your own messages use @ref
     * sendMessage instead!
     *
     * @param msg the message which will be send. See messages.txt for contents
     * @param msgid an id for this message
     * @param receiver 0:broadcast, lower 9 bits=0 then to game<<10, otherwise to player
     * @param sender same as receiver
     * @param playerid send from or to this player id ... has to be improved
     * @return true if worked
     */
    bool sendSystemMessage(const QDataStream &msg, int msgid, int receiver=0, int sender=0);

    bool sendSystemMessage(const QByteArray& a, int msgid, int receiver=0, int sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendSystemMessage(const QString& msg, int msgid, int receiver=0, int sender=0);

    /**
     * sends a network message with an error text
     **/
    void sendError(int error, const QString& text,int receiver=0,int sender=0);


    /**
     * Are we still offer offering server connections - only for game MASTER
     *
     * @return true/false
     **/
    bool isOfferingConnections() const;


    /**
     * Application cookie. this idendifies the game application. It
     * help to distinguish between e.g. KPoker and KWin4
     *
     * @return the application cookie
     **/
    int cookie() const;


    /**
     * Sends a network message msg with a given msg id msgid to either all clients
     * or a given client or all clients but a given client. You want to use this
     * to send a message to the clients.
     *
     * SendMessage differs from @ref sendSystemMessage only by the msgid parameter.
     * @ref sendSystemMessage is thought as a KGame only mehtod while
     * sendMessage is for public use. The msgid paramter will be
     * +=KGameMessages::IdUser and in @ref KGame::signalNetworkData msgid will
     * be -= KGameMessages::IdUser again, so that one can easily distinguish
     * between system and user messages.
     *
     * Use @ref sendSystemMessage to comunicate with KGame (e.g. by adding a
     * player) and sendMessage for your own user message
     *
     * Note: a player should send messages through a @ref KGameIO!
     *
     * @param msg the message which will be send. See messages.txt for contents
     * @param msgid an id for this message
     * @param receiver broadcast to all clients if 0 otherwise just send to one client
     * @param sender The sender of the message
     * @return true if worked
     **/
    bool sendMessage(const QDataStream &msg, int msgid, int receiver=0, int sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendMessage(const QString& msg, int msgid, int receiver=0, int sender=0);

    /**
     * This is an overloaded member function, provided for convenience.
     **/
    bool sendMessage(int data, int msgid, int receiver=0, int sender=0);

    /**
     * Register a KGameIO devices as message listener. This means this
     * device will get all messages to this game client forwarded
     *
     * @param The client
     */
    bool registerListener(KGameIO *l);

    /**
     * Unregister a KGameIO device as messager listner.
     */
    bool unregisterListener(KGameIO *l);

    /**
     * Called by @ref ReceiveNetworkTransmission(). Will be overwritten by
     * @ref KGame and handle the incoming message.
     **/
    virtual void networkTransmission(QDataStream&, int, int, int, Q_UINT32 clientID) = 0;

signals:
    /**
     * A network error occured
     * @param error the error code
     * @param text the error text
     */
    void signalNetworkErrorMessage(int error, QString text);

    /**
     * Network message received but with wrong checks, version
     * cookie, etc
     */
    void signalNetworkVersionError(Q_UINT32 clientID);

    /**
     * Our connection to the @ref KMessageServer has broken.
     * See @ref KMessageClient::connectionBroken
     **/
    void signalConnectionBroken();

    /**
     * This signal is emitted whenever the @ref KMessageServer sends us a message that a
     * new client connected. @ref KGame uses this to call KGame::negotiateNetworkGame
     * for the newly connected client if we are admin (see @ref isAdmin)
     *
     * See @ref KMessageClient::eventClientConnected
     *
     * @param clientID the ID of the newly connected client
     **/
    void signalClientConnected(Q_UINT32 clientID);

    /**
     * This signal is emitted whenever the @ref KMessageServer sends us a message
     * that a connection to a client was detached. The second parameter can be used
     * to distinguish between network errors or removing on purpose.
     *
     * See @ref KMessageClient::eventClientDisconnected
     *
     * @param clientID the client that has disconnected
     * @param broken true if the connection was lost because of a network error, false
     *        if the connection was closed by the message server admin.
     */
    void signalClientDisconnected(Q_UINT32 clientID, bool broken);

protected slots:
    /**
     * Called by @ref KMessageClient::broadcastReceived() and will check if the
     * message format is valid. If it is not, it will generate an error (see
     * @ref signalNetworkVersionError and @ref signalNetworkErorrMessage).
     * If it is valid, the pure virtual method @ref networkTransmission() is called.
     * (This one is overwritten in KGame.)
     **/
    void receiveNetworkTransmission(const QByteArray& a, Q_UINT32 clientID);

    /**
     * This KGame object receives or looses the admin status.
     * @param isAdmin Whether we are admin or not
     **/
    void slotAdminStatusChanged(bool isAdmin);

private:
    /**
     * @internal
     * Same then the other transmitGameNetworkMessage except for different parameters. It is internally
     * called by the other functions.
     */
    bool sendGameIOMessage(const QByteArray &msg, int msgid, KGameIO *client, int receiver=0, int sender=0 );

private:
    KGameNetworkPrivate* d;
};

#endif
