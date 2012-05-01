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

#ifndef __KMESSAGECLIENT_H__
#define __KMESSAGECLIENT_H__

#include <QtCore/QObject>
#include <QtCore/QString>

#include "../libkdegamesprivate_export.h"
class KMessageIO;
class KMessageServer;
class KMessageClientPrivate;

/**
  \class KMessageClient kmessageclient.h <KGame/KMessageClient>
  
  @short A client to connect to a KMessageServer

  This class implements a client that can connect to a KMessageServer object.
  It can be used to exchange messages between clients.

  Usually you will connect the signals broadcastReceived and forwardReceived to
  some specific slots. In these slot methods you can analyze the messages that are
  sent to you from other clients.

  To send messages to other clients, use the methods sendBroadcast() (to send to all
  clients) or sendForward() (to send to a list of selected clients).

  If you want to communicate with the KMessageServer object directly (on a more low
  level base), use the method sendServerMessage to send a command to the server and
  connect to the signal serverMessageReceived to see all the incoming messages.
  In that case the messages must be of the format specified in KMessageServer.
  @author Burkhard Lehner <Burkhard.Lehner@gmx.de>
*/
class KDEGAMESPRIVATE_EXPORT KMessageClient : public QObject
{
  Q_OBJECT

public:

  /**
    Constructor.
    Creates an unconnected KMessageClient object. Use setServer() later to connect to a
    KMessageServer object.
  */
  KMessageClient (QObject *parent = 0);

  /**
    Destructor.
    Disconnects from the server, if any connection was established.
  */
  ~KMessageClient ();

  /**
    @return The client ID of this client. Every client that is connected to a KMessageServer
    has a unique ID number.

    NOTE: As long as the object is not yet connected to the server, and as long as the server
    hasn't sent the client ID, this method returns 0.
  */
  quint32 id () const;

  /**
    @return Whether or not this client is the server admin.
    One of the clients connected to the server is the admin and can administrate the server
    (set maximum number of clients, remove clients, ...).

    If you use admin commands without being the admin, these commands are simply ignored by
    the server.

    NOTE: As long as you are not connected to a server, this method returns false.
  */
  bool isAdmin () const;

  /**
    @return The ID of the admin client on the message server.
  */
  quint32 adminId() const;

  /**
    @return The list of the IDs of all the message clients connected to the message server.
  */
  QList <quint32> clientList() const;

  /**
    Connects the client to (another) server.

    Tries to connect via a TCP/IP socket to a KMessageServer object
    on the given host, listening on the specified port.

    If we were already connected, the old connection is closed.
    @param host The name of the host to connect to. Must be either a hostname which can 
    be resolved to an IP or just an IP
    @param port The port to connect to
  */
  void setServer (const QString &host, quint16 port);

  /**
    Connects the client to (another) server.

    Connects to the given server, using KMessageDirect.
    (The server object has to be in the same process.)

    If we were already connected, the old connection is closed.
    @param server The KMessageServer to connect to
  */
  void setServer (KMessageServer *server);

  /**
   * Corresponds to setServer(0); but also emits the connectionBroken signal
   **/
  void disconnect();

  /**
    Connects the client to (another) server.

    To use this method, you have to create a KMessageIO object with new (indeed you must
    create an instance of a subclass of KMessageIO, e.g. KMessageSocket or KMessageDirect).
    This object must already be connected to the new server.

    Calling this method disconnects any earlier connection, and uses the new KMessageIO
    object instead. This object gets owned by the KMessageClient object, so don't delete
    or manipulate it afterwards.

    With this method it is possible to change the server on the fly. But be careful that
    there are no important messages from the old server not yet delivered.

    NOTE: It is very likely that we will have another client ID on the new server. The
    value returned by clientID may be a little outdated until the new server tells us
    our new ID.

    NOTE: The two other setServer methods are for convenience. If you use them, you don't
    have to create a KMessageIO object yourself.
  */
  virtual void setServer (KMessageIO *connection);

  /**
    @return True, if a connection to a KMessageServer has been started, and if the
    connection is ready for transferring data. (It will return false e.g. as long as
    a socket connection hasn't been established, and it will also return false after
    a socket connection is broken.)
  */
  bool isConnected () const;

  /**
    @return TRUE if isConnected() is true AND this is not a local (like
    KMessageDirect) connection.
  */
  bool isNetwork () const;

  /**
    @return 0 if isConnected() is FALSE, otherwise the port number this client is
    connected to. See also KMessageIO::peerPort and QSocket::peerPort.
  */
  quint16 peerPort () const;

  /**
    @return "localhost" if isConnected() is FALSE, otherwise the hostname this client is
    connected to. See also KMessageIO::peerName() and QSocket::peerName().
  */
  QString peerName() const;

  /**
    Sends a message to the KMessageServer. If we are not yet connected to one, nothing
    happens.

    Use this method to send a low level command to the server. It has to be in the
    format specified in KMessageServer.

    If you want to send messages to other clients, you should use sendBroadcast()
    and sendForward().
    @param msg The message to be sent to the server. Must be in the format specified in KMessageServer.
  */
  void sendServerMessage (const QByteArray &msg);

  /**
    Sends a message to all the clients connected to the server, including ourself.
    The message consists of an arbitrary block of data with arbitrary length.

    All the clients will receive an exact copy of this block of data, which will be
    processed in their processBroadcast() method.
    @param msg The message to be sent to the clients
  */
  //AB: processBroadcast doesn't exist!! is processIncomingMessage meant?
  void sendBroadcast (const QByteArray &msg);

  /**
    Sends a message to all the clients in a list.
    The message consists of an arbitrary block of data with arbitrary length.

    All clients will receive an exact copy of this block of data, which will be
    processed in their processForward() method.

    If the list contains client IDs that are not defined, they are ignored. If
    it contains an ID several times, that client will receive the message several
    times.

    To send a message to the admin of the KMessageServer, you can use 0 as clientID,
    instead of using the real client ID.
    @param msg The message to be sent to the clients
    @param clients A list of clients the message should be sent to
  */
  //AB: processForward doesn't exist!! is processIncomingMessage meant?
  void sendForward (const QByteArray &msg, const QList <quint32> &clients);

  /**
    Sends a message to a single client. This is a convenieance method. It calls
    sendForward (const QByteArray &msg, const QValueList &ltquint32> &clients)
    with a list containing only one client ID.

    To send a message to the admin of the  KMessageServer, you can use 0 as clientID,
    instead of using the real client ID.
    @param msg The message to be sent to the client
    @param client The id of the client the message shall be sent to
  */
  void sendForward (const QByteArray &msg, quint32 client);

  /**
    Once this function is called no message will be received anymore.
    processIncomingMessage() gets delayed until unlock() is called.

    Note that all messages are still received, but their delivery (like
    broadcastReceived()) get delayed only.
   */
  void lock();

  /**
    Deliver every message that was delayed by lock() and actually deliver 
    all messages that get received from now on.
   */
  void unlock();

  /**
    @return The number of messages that got delayed since lock() was called
   */
  unsigned int delayedMessageCount() const;

Q_SIGNALS:
  /**
    This signal is emitted when the client receives a broadcast message from the
    KMessageServer, sent by another client. Connect to this signal to analyze the
    received message and do the right reaction.

    senderID contains the ID of the client that sent the broadcast message. You can
    use this e.g. to send a reply message to only that client. Or you can use it
    to ignore broadcast messages that were sent by yourself:

    \code
      void myObject::myBroadcastSlot (const QByteArray &msg, quint32 senderID)
      {
        if (senderID == ((KMessageClient *)sender())->id())
          return;
        ...
      }
    \endcode
    @param msg The message that has been sent to us
    @param senderID The ID of the client which sent the message
  */
  void broadcastReceived (const QByteArray &msg, quint32 senderID);

  /**
    This signal is emitted when the client receives a forward message from the
    KMessageServer, sent by another client. Connect to this signal to analyze the
    received message and do the right reaction.

    senderID contains the ID of the client that sent the broadcast message. You can
    use this e.g. to send a reply message to only that client.

    receivers contains the list of the clients that got the message. (If this list
    only contains one number, this will be your client ID, and it was exclusivly
    sent to you.)

    If you don't want to distinguish between broadcast and forward messages and
    treat them the same, you can connect forwardReceived signal to the
    broadcastReceived signal. (Yes, that's possible! You can connect a Qt signal to
    a Qt signal, and the second one can have less parameters.)

    \code
      KMessageClient *client = new KMessageClient ();
      connect (client, SIGNAL (forwardReceived (const QByteArray &, quint32, const QValueList <quint32>&)),
               client, SIGNAL (broadcastReceived (const QByteArray &, quint32)));
    \endcode

    Then connect the broadcast signal to your slot that analyzes the message.
    @param msg The message that has been sent to us
    @param senderID The ID of the client which sent the message
    @param receivers All clients which receive this message
  */
  void forwardReceived (const QByteArray &msg, quint32 senderID, const QList <quint32> &receivers);

  /**
    This signal is emitted when the connection to the KMessageServer is broken.
    Reasons for this can be: a network error, a server breakdown, or you were just kicked
    from the server.

    When this signal is sent, the connection is already lost and the client is unconnected.
    You can connect to another server by calling setServer() afterwards. But keep in mind that
    some important messages might have vanished.
  */
  void connectionBroken ();

  /**
    This signal is emitted right before the client disconnects. It can be used
    to this store the id of the client which is about to be lost.
  */
  void aboutToDisconnect(quint32 id);

  /**
    This signal is emitted when this client becomes the admin client or when it loses
    the admin client status. Connect to this signal if you have to do any initialization
    or cleanup.
    @param isAdmin Whether we are now admin or not
  */
  void adminStatusChanged (bool isAdmin);

  /**
    This signal is emitted when another client has connected
    to the server. Connect to this method if that clients needs initialization.
    This should usually only be done in one client, e.g. the admin client.
    @param clientID The ID of the client that has newly connectd.
  */
  void eventClientConnected (quint32 clientID);

  /**
    This signal is emitted when the server has lost the
    connection to one of the clients (This could be because of a bad internet connection
    or because the client disconnected on purpose).
    @param clientID The ID of the client that has disconnected
    @param broken true if it was disconnected because of a network error
  */
  void eventClientDisconnected (quint32 clientID, bool broken);

  /**
    This signal is emitted on every message that came from the server. You can connect to this
    signal to see the messages directly. They are in the format specified in KMessageServer.

    @param msg The message that has been sent to us
    @param unknown True when KMessageClient didn't recognize the message, i.e. it contained an unknown
    message ID. If you want to add additional message types to the client, connect to this signal,
    and if unknown is true, analyze the message by yourself. If you recognized the message,
    set unknown to false (Otherwise a debug message will be printed).
  */
  //AB: maybe add a setNoEmit() so that the other signals can be deactivated?
  //Could be a performance benefit (note: KMessageClient is a time critical
  //class!!!)
  void serverMessageReceived (const QByteArray &msg, bool &unknown);

protected:
  /**
    This slot is called from processIncomingMessage or 
    processFirstMessage, depending on whether the client is locked or a delayed
    message is still here (see lock)

    It processes the message and analyzes it. If it is a broadcast or a forward message from
    another client, it emits the signal processBroadcast or processForward accordingly.

    If you want to treat additional server messages, you can overwrite this method. Don't
    forget to call processIncomingMessage of your superclass!

    At the moment, the following server messages are interpreted:

    MSG_BROADCAST, MSG_FORWARD, ANS_CLIENT_ID, ANS_ADMIN_ID, ANS_CLIENT_LIST
    @param msg The incoming message
  */

  virtual void processMessage (const QByteArray& msg);

protected Q_SLOTS:
  /**
    This slot is called from the signal KMessageIO::received whenever a message from the
    KMessageServer arrives.

    It processes the message and analyzes it. If it is a broadcast or a forward message from
    another client, it emits the signal processBroadcast or processForward accordingly.

    If you want to treat additional server messages, you can overwrite this method. Don't
    forget to call processIncomingMessage() of your superclass!

    At the moment, the following server messages are interpreted:

    MSG_BROADCAST, MSG_FORWARD, ANS_CLIENT_ID, ANS_ADMIN_ID, ANS_CLIENT_LIST
    @param msg The incoming message
  */
  virtual void processIncomingMessage (const QByteArray &msg);

  /**
    Called from unlock() (using QTimer::singleShot) until all delayed 
    messages are delivered.
  */
  void processFirstMessage();

  /**
    This slot is called from the signal KMessageIO::connectionBroken.

    It deletes the internal KMessageIO object, and resets the client to default
    values. To connect again to another server, use setServer.
  */
  virtual void removeBrokenConnection ();
  void removeBrokenConnection2 ();

private:
  KMessageClientPrivate * const d;
};

#endif
