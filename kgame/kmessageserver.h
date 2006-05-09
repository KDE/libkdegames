/*
    This file is part of the KDE games library
    Copyright (C) 2001 Burkhard Lehner (Burkhard.Lehner@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KMESSAGESERVER_H__
#define __KMESSAGESERVER_H__

#include <qobject.h>
#include <q3serversocket.h>
#include <QString>


class KMessageIO;
class KMessageServerPrivate;

/**
  @short A server for message sending and broadcasting, using TCP/IP connections.

  An object of this class listens for incoming connections via TCP/IP sockets and
  creates KMessageSocket objects for every established connection. It receives
  messages from the "clients", analyses them and processes an appropriate
  reaction.

  You can also use other KMessageIO objects with KMessageServer, not only
  TCP/IP socket based ones. Use addClient to connect via an object of any
  KMessageIO subclass. (For clients within the same process, you can e.g. use
  KMessageDirect.) This object already has to be connected.

  The messages are always packages of an arbitrary length. The format of the messages
  is given below. All the data is stored and received with QDataStream, to be
  platform independant.

  Setting up a KMessageServer can be done like this:

  \code
    KMessageServer *server = new KMessageServer ();
    server->initNetwork (TCP/IP-Portnumber);
  \endcode

  Usually that is everything you will do. There are a lot of public methods to
  administrate the object (maximum number of clients, finding clients, removing
  clients, setting the admin client, ...), but this functionality can also
  be done by messages from the clients. So you can administrate the object completely
  on remote.

  If you want to extend the Server for your own needs (e.g. additional message types),
  you can either create a subclass and overwrite the method processOneMessage.
  (But don't forget to call the method of the superclass!) Or you can connect to
  the signal messageReceived, and analyse the messages there.

  Every client has a unique ID, so that messages can be sent to another dedicated
  client or a list of clients.

  One of the clients (the admin) has a special administration right. Some of the
  administration messages can only be used with him. The admin can give the admin
  status to another client. You can send a message to the admin by using clientID 0.
  This is always interpreted as the admin client, independant of its real clientID.

  Here is a list of the messages the KMessageServer understands:
  &lt;&lt; means, the value is inserted into the QByteArray using QDataStream. The
  messageIDs (REQ_BROADCAST, ...) are of type quint32.

  - QByteArray << static_cast&lt;quint32>( REQ_BROADCAST ) << raw_data

    When the server receives this message, it sends the following message to
    ALL connected clients (a broadcast), where the raw_data is left unchanged:
       QByteArray << static_cast &lt;quint32>( MSG_BROADCAST ) << clientID << raw_data
       quint32 clientID; // the ID of the client that sent the broadcast request

  - QByteArray << static_cast&lt;quint32>( REQ_FORWARD ) << client_list << raw_data
    QValueList &lt;quint32> client_list; // list of receivers

    When the server receives this message, it sends the following message to
    the clients in client_list:
        QByteArray << static_cast&lt;quint32>( MSG_FORWARD ) << senderID << client_list << raw_data
        quint32 senderID;  // the sender of the forward request
        QValueList &lt;quint32> client_list; // a copy of the receiver list

    Note: Every client receives the message as many times as he is in the client_list.
    Note: Since the client_list is sent to all the clients, every client can see who else
          got the message. If you want to prevent this, send a single REQ_FORWARD
          message for every receiver.

  - QByteArray << static_cast&lt;quint32>( REQ_CLIENT_ID )

    When the server receives this message, it sends the following message to
    the asking client:
        QByteArray << static_cast&lt;quint32>( ANS_CLIENT_ID ) << clientID
        quint32 clientID;  // The ID of the client who asked for it

    Note: This answer is also automatically sent to a new connected client, so that he
          can store his ID. The ID of a client doesn't change during his lifetime, and is
          unique for this KMessageServer.

  - QByteArray << static_cast&lt;quint32>( REQ_ADMIN_ID )

    When the server receives this message, it sends the following message to
    the asking client:
        QByteArray << ANS_ADMIN_ID << adminID
        quint32 adminID;  // The ID of the admin

    Note: This answer is also automatically sent to a new connected client, so that he
          can see if he is the admin or not. It will also be sent to all connected clients
          when a new admin is set (see REQ_ADMIN_CHANGE).

  - QByteArray << static_cast&lt;quint32>( REQ_ADMIN_CHANGE ) << new_admin
    quint32 new_admin;  // the ID of the new admin, or 0 for no admin

    When the server receives this message, it sets the admin to the new ID. If no client
    with that ID exists, nothing happens. With new_admin == 0 no client is a admin.
    ONLY THE ADMIN ITSELF CAN USE THIS MESSAGE!

    Note: The server sends a ANS_ADMIN_ID message to every connected client.

  - QByteArray << static_cast&lt;quint32>( REQ_REMOVE_CLIENT ) << client_list
    QValueList &lt;quint32> client_list; // The list of clients to be removed

    When the server receives this message, it removes the clients with the ids stored in
    client_list, disconnecting the connection to them.
    ONLY THE ADMIN CAN USE THIS MESSAGE!

    Note: If one of the clients is the admin himself, he will also be deleted.
          Another client (if any left) will become the new admin.

  - QByteArray << static_cast&lt;quint32>( REQ_MAX_NUM_CLIENTS ) << maximum_clients
    qint32 maximum_clients; // The maximum of clients connected, or infinite if -1

    When the server receives this message, it limits the number of clients to the number given,
    or sets it unlimited for maximum_clients == -1.
    ONLY THE ADMIN CAN USE THIS MESSAGE!

    Note: If there are already more clients, they are not affected. It only prevents new Clients
          to be added. To assure this limit, remove clients afterwards (REQ_REMOVE_CLIENT)

  - QByteArray  << static_cast&lt;quint32>( REQ_CLIENT_LIST )

    When the server receives this message, it answers by sending a list of IDs of all the clients
    that are connected at the moment. So it sends the following message to the asking client:
        QByteArray << static_cast&lt;quint32>( ANS_CLIENT_LIST ) << clientList
        QValueList &lt;quint32> clientList;  // The IDs of the connected clients

    Note: This message is also sent to every new connected client, so that he knows the other
          clients.

  There are two more messages that are sent from the server to the every client automatically
  when a new client connects or a connection to a client is lost:

        QByteArray << static_cast&lt;quint32>( EVNT_CLIENT_CONNECTED ) << clientID;
        quint32 clientID;   // the ID of the new connected client

        QByteArray << static_cast&lt;quint32>( EVNT_CLIENT_DISCONNECTED ) << clientID;
        quint32 clientID;   // the ID of the client that lost the connection
        quint8 broken;      // 1 if the network connection was closed, 0 if it was disconnected
                             // on purpose


  @author Andreas Beckermann <b_mann@gmx.de>, Burkhard Lehner <Burkhard.Lehner@gmx.de>
  @version $Id$
*/
class KMessageServer : public QObject
{
  Q_OBJECT

public:
    /**
      MessageIDs for messages from a client to the message server.
    */
    enum { 
            REQ_BROADCAST = 1, 
            REQ_FORWARD,
            REQ_CLIENT_ID,
            REQ_ADMIN_ID,
            REQ_ADMIN_CHANGE,
            REQ_REMOVE_CLIENT,
            REQ_MAX_NUM_CLIENTS,
            REQ_CLIENT_LIST,
            REQ_MAX_REQ = 0xffff };

    /**
     * MessageIDs for messages from the message server to a client.
     **/
    enum {
            MSG_BROADCAST = 101, 
            MSG_FORWARD, 
            ANS_CLIENT_ID, 
            ANS_ADMIN_ID, 
            ANS_CLIENT_LIST,
            EVNT_CLIENT_CONNECTED, 
            EVNT_CLIENT_DISCONNECTED,
            EVNT_MAX_EVNT = 0xffff
    };

    /**
     * Create a KGameNetwork object
     **/
    KMessageServer(quint16 cookie = 42, QObject* parent = 0);

    ~KMessageServer();

    /**
     * Gives debug output of the game status
     **/
    virtual void Debug();

//---------------------------------- TCP/IP server stuff

    /**
     * Starts the Communication server to listen for incoming TCP/IP connections.
     *
     * @param port The port on which the service is offered, or 0 to let the
     * system pick a free port
     * @return true if it worked
    */
    bool initNetwork (quint16 port = 0);

    /**
     * Returns the TCP/IP port number we are listening to for incoming connections.
     * (This has to be known by other clients so that they can connect to us. It's
     * especially necessary if you used 0 as port number in initNetwork().
     * @return the port number
     **/
    quint16 serverPort () const;

    /**
     * Stops listening for connections. The already running connections are
     * not affected.
     * To listen for connections again call initNetwork again.
     **/
    void stopNetwork();

    /**
     * Are we still offer offering server connections?
     * @return true, if we are still listening to connections requests
     **/
    bool isOfferingConnections() const;

//---------------------------------- adding / removing clients

public slots:
    /**
     * Adds a new @ref KMessageIO object to the communication server. This "client"
     * gets a unique ID.
     *
     * This slot method is automatically called for any incoming TCP/IP
     * connection. You can use it to add other types of connections, e.g.
     * local connections (KMessageDirect) to the server manually.
     *
     * NOTE: The @ref KMessageIO object gets owned by the KMessageServer,
     * so don't delete or manipulate it afterwards. It is automatically deleted
     * when the connection is broken or the communication server is deleted.
     * So, add a @ref KMessageIO object to just ONE KMessageServer.
     **/
    void addClient (KMessageIO *);

    /**
     * Removes the KMessageIO object from the client list and deletes it.
     * This destroys the connection, if it already was up.
     * Does NOT emit connectionLost.
     * Sends an info message to the other clients, that contains the ID of
     * the removed client and the value of the parameter broken.
     *
     * @param io the object to delete and to remove from the client list
     * @param broken true if the client has lost connection
     * Mostly used internally. You will probably not need this.
     **/
    void removeClient (KMessageIO *io, bool broken);

    /**
      Deletes all connections to the clients.
    */
    void deleteClients();

private slots:
    /**
     * Removes the sender object of the signal that called this slot. It is
     * automatically connected to @ref KMessageIO::connectionBroken.
     * Emits @ref connectionLost (KMessageIO*), and deletes the @ref KMessageIO object.
     * Don't call it directly!
     **/
    void removeBrokenClient ();

public:
    /**
     * sets the maximum number of clients which can connect.
     * If this number is reached, no more clients can be added.
     * Setting this number to -1 means unlimited number of clients.
     *
     * NOTE: Existing connections are not affected.
     * So, clientCount > maxClients is possible, if there were already
     * more clients than allowed before reducing this value.
     *
     * @param maxnumber the number of clients
     **/
    void setMaxClients(int maxnumber);

    /**
     * returns the maximum number of clients
     *
     * @return the number of clients
     **/
    int maxClients() const;

    /**
     * returns the current number of connected clients.
     *
     * @return the number of clients
     **/
    int clientCount() const;

    /**
     * returns a list of the unique IDs of all clients.
     **/
    QList <quint32> clientIDs() const;

    /**
     * Find the @ref KMessageIO object to the given client number.
     * @param no the client number to look for, or 0 to look for the admin
     * @return address of the client, or 0 if no client with that number exists
     **/
    KMessageIO *findClient (quint32 no) const;

    /**
     * Returns the clientID of the admin, if there is a admin, 0 otherwise.
     *
     * NOTE: Most often you don't need to know that id, since you can
     * use clientID 0 to specify the admin.
     **/
    quint32 adminID() const;

    /**
     * Sets the admin to a new client with the given ID.
     * The old admin (if existed) and the new admin will get the ANS_ADMIN message.
     * If you use 0 as new adminID, no client will be admin.
     **/
    void setAdmin (quint32 adminID);


//------------------------------ ID stuff

    /*
     * The unique ID of this game
     *
     * @return int id
     **/
//    int gameId() const;

    /*
     * Application cookie. this idendifies the game application. It
     * help to distinguish between e.g. KPoker and KWin4
     *
     * @return the application cookie
     **/
//    int cookie() const;

//------------------------------ Message stuff

public:
    /**
     * Sends a message to all connected clients.
     * The message is NOT translated in any way. This method calls
     * @ref KMessageIO::send for every client added.
     **/
    virtual void broadcastMessage (const QByteArray &msg);

    /**
     * Sends a message to a single client with the given ID.
     * The message is NOT translated in any way.
     * If no client with the given id exists, nothing is done.
     * This is just a convenience method. You could also call
     * @ref findClient (id)->send(msg) manually, but this method checks for
     * errors.
     **/
    virtual void sendMessage (quint32 id, const QByteArray &msg);

    /**
     * Sends a message to a list of clients. Their ID is given in ids. If
     * a client id is given more than once in the list, the message is also
     * sent several times to that client.
     * This is just a convenience method. You could also iterate over the
     * list of IDs.
     **/
    virtual void sendMessage (const QList <quint32> &ids, const QByteArray &msg);

protected slots:
    /**
     * This slot receives all the messages from the @ref KMessageIO::received signals.
     * It stores the messages in a queue. The messages are later taken out of the queue
     * by @ref getReceivedMessage.
     *
     * NOTE: It is important that this slot may only be called from the signal
     * @ref KMessageIO::received, since the sender() object is used to find out
     * the client that sent the message!
     **/
    virtual void getReceivedMessage (const QByteArray &msg);

    /**
     * This slot is called whenever there are elements in the message queue. This queue
     * is filled by @ref getReceivedMessage.
     * This slot takes one message out of the queue and analyses processes it,
     * if it recognizes it. (See message types in the description of the class.)
     * After that, the signal @ref messageReceived is emitted. Connect to that signal if
     * you want to process other types of messages.
     **/
    virtual void processOneMessage ();

//---------------------------- Signals

signals:
    /**
     * A new client connected to the game
     * @param client the client object that connected
     **/
    void clientConnected (KMessageIO *client);

    /**
     * A network connection got broken. Note that the client will automatically get deleted
     * after this signal is emitted. The signal is not emitted when the client was removed
     * regularly.
     *
     * @param client the client which left the game
     **/
    void connectionLost (KMessageIO *client);

    /**
     * This signal is always emitted when a message from a client is received.
     *
     * You can use this signal to extend the communication server without subclassing.
     * Just connect to this signal and analyse the message, if unknown is true.
     * If you recognize a message and process it, set unknown to false, otherwise
     * a warning message is printed.
     *
     * @param data the message data
     * @param clientID the ID of the KMessageIO object that received the message
     * @param unknown true, if the message type is not known by the KMessageServer
     **/
    void messageReceived (const QByteArray &data, quint32 clientID, bool &unknown);

protected:
    /**
     * @return A unique number which can be used as the id of a @ref KMessageIO. It is
     * incremented after every call so if you need the id twice you have to save
     * it anywhere. It's currently used to initialize newly connected clints only.
     **/
    quint32 uniqueClientNumber() const;

private:
    KMessageServerPrivate* d;
};


/**
  Internal class of KMessageServer. Creates a server socket and waits for
  connections.

  NOTE: This has to be here in the header file, because it is a subclass from
  QObject and has to go through the moc.

  @short An internal class for KServerSocket
  @author Burkhard Lehner <Burkhard.Lehner@gmx.de>
*/
class KMessageServerSocket : public Q3ServerSocket
{
  Q_OBJECT

public:
  KMessageServerSocket (quint16 port, QObject *parent = 0);
  ~KMessageServerSocket ();

  void newConnection (int socket);

signals:
  void newClientConnected (KMessageIO *client);
};



#endif
