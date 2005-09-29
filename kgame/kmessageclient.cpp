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

#include <kdebug.h>
#include <stdio.h>

#include <qbuffer.h>
#include <qtimer.h>

#include "kmessageio.h"
#include "kmessageserver.h"

#include "kmessageclient.h"

class KMessageClientPrivate
{
public:
  KMessageClientPrivate ()
    : adminID (0), connection (0)
  {}

  ~KMessageClientPrivate ()
  {
    delete connection;
  }

  Q_UINT32 adminID;
  QValueList <Q_UINT32> clientList;
  KMessageIO *connection;

  bool isLocked;
  QValueList <QByteArray> delayedMessages;
};

KMessageClient::KMessageClient (QObject *parent, const char *name)
  : QObject (parent, name)
{
  d = new KMessageClientPrivate ();
  d->isLocked = false;
}

KMessageClient::~KMessageClient ()
{
  d->delayedMessages.clear();
  delete d;
}

// -- setServer stuff

void KMessageClient::setServer (const QString &host, Q_UINT16 port)
{
  setServer (new KMessageSocket (host, port));
}

void KMessageClient::setServer (KMessageServer *server)
{
  KMessageDirect *serverIO = new KMessageDirect ();
  setServer (new KMessageDirect (serverIO));
  server->addClient (serverIO);
}

void KMessageClient::setServer (KMessageIO *connection)
{
  if (d->connection)
  {
    delete d->connection;
    kdDebug (11001) << k_funcinfo << ": We are changing the server!" << endl;
  }

  d->connection = connection;
  if (connection )
  {
    connect (connection, SIGNAL (received(const QByteArray &)),
             this, SLOT (processIncomingMessage(const QByteArray &)));
    connect (connection, SIGNAL (connectionBroken()),
             this, SLOT (removeBrokenConnection ()));
  }
}

// -- id stuff

Q_UINT32 KMessageClient::id () const
{
  return (d->connection) ? d->connection->id () : 0;
}

bool KMessageClient::isAdmin () const
{
  return id() != 0 && id() == adminId();
}

Q_UINT32 KMessageClient::adminId () const
{
  return d->adminID;
}

const QValueList <Q_UINT32> &KMessageClient::clientList() const
{
  return d->clientList;
}

bool KMessageClient::isConnected () const
{
  return d->connection && d->connection->isConnected();
}

bool KMessageClient::isNetwork () const
{
  return isConnected() ? d->connection->isNetwork() : false;
}

Q_UINT16 KMessageClient::peerPort () const
{
 return d->connection ? d->connection->peerPort() : 0;
}

QString KMessageClient::peerName () const
{
 return d->connection ? d->connection->peerName() : QString::fromLatin1("localhost");
}

// --------------------- Sending messages

void KMessageClient::sendServerMessage (const QByteArray &msg)
{
  if (!d->connection)
  {
    kdWarning (11001) << k_funcinfo << ": We have no connection yet!" << endl;
    return;
  }
  d->connection->send (msg);
}

void KMessageClient::sendBroadcast (const QByteArray &msg)
{
  QByteArray sendBuffer;
  QBuffer buffer (sendBuffer);
  buffer.open (IO_WriteOnly);
  QDataStream stream (&buffer);

  stream << static_cast<Q_UINT32> ( KMessageServer::REQ_BROADCAST );
  buffer.QIODevice::writeBlock (msg);
  sendServerMessage (sendBuffer);
}

void KMessageClient::sendForward (const QByteArray &msg, const QValueList <Q_UINT32> &clients)
{
  QByteArray sendBuffer;
  QBuffer buffer (sendBuffer);
  buffer.open (IO_WriteOnly);
  QDataStream stream (&buffer);

  stream << static_cast<Q_UINT32>( KMessageServer::REQ_FORWARD ) << clients;
  buffer.QIODevice::writeBlock (msg);
  sendServerMessage (sendBuffer);
}

void KMessageClient::sendForward (const QByteArray &msg, Q_UINT32 client)
{
  sendForward (msg, QValueList <Q_UINT32> () << client);
}


// --------------------- Receiving and processing messages

void KMessageClient::processIncomingMessage (const QByteArray &msg)
{
  if (d->isLocked)
  {
    d->delayedMessages.append(msg);
    return;
  }
  if (d->delayedMessages.count() > 0)
  {
    d->delayedMessages.append (msg);
    QByteArray first = d->delayedMessages.front();
    d->delayedMessages.pop_front();
    processMessage (first);
  }
  else
  {
    processMessage(msg);
  }
}

void KMessageClient::processMessage (const QByteArray &msg)
{
  if (d->isLocked)
  { // must NOT happen, since we check in processIncomingMessage as well as in processFirstMessage
    d->delayedMessages.append(msg);
    return;
  }
  QBuffer in_buffer (msg);
  in_buffer.open (IO_ReadOnly);
  QDataStream in_stream (&in_buffer);


  bool unknown = false;

  Q_UINT32 messageID;
  in_stream >> messageID;
  switch (messageID)
  {
    case KMessageServer::MSG_BROADCAST:
      {
        Q_UINT32 clientID;
        in_stream >> clientID;
        emit broadcastReceived (in_buffer.readAll(), clientID);
      }
      break;

    case KMessageServer::MSG_FORWARD:
      {
        Q_UINT32 clientID;
        QValueList <Q_UINT32> receivers;
        in_stream >> clientID >> receivers;
        emit forwardReceived (in_buffer.readAll(), clientID, receivers);
      }
      break;

    case KMessageServer::ANS_CLIENT_ID:
      {
        bool old_admin = isAdmin();
        Q_UINT32 clientID;
        in_stream >> clientID;
        d->connection->setId (clientID);
        if (old_admin != isAdmin())
          emit adminStatusChanged (isAdmin());
      }
      break;

    case KMessageServer::ANS_ADMIN_ID:
      {
        bool old_admin = isAdmin();
        in_stream >> d->adminID;
        if (old_admin != isAdmin())
          emit adminStatusChanged (isAdmin());
      }
      break;

    case KMessageServer::ANS_CLIENT_LIST:
      {
        in_stream >> d->clientList;
      }
      break;

    case KMessageServer::EVNT_CLIENT_CONNECTED:
      {
        Q_UINT32 id;
        in_stream >> id;

        if (d->clientList.contains (id))
          kdWarning (11001) << k_funcinfo << ": Adding a client that already existed!" << endl;
        else
          d->clientList.append (id);

        emit eventClientConnected (id);
      }
      break;

    case KMessageServer::EVNT_CLIENT_DISCONNECTED:
      {
        Q_UINT32 id;
        Q_INT8 broken;
        in_stream >> id >> broken;

        if (!d->clientList.contains (id))
          kdWarning (11001) << k_funcinfo << ": Removing a client that doesn't exist!" << endl;
        else
          d->clientList.remove (id);

        emit eventClientDisconnected (id, bool (broken));
      }
      break;

    default:
      unknown = true;
  }

  if (!unknown && !in_buffer.atEnd())
    kdWarning (11001) << k_funcinfo << ": Extra data received for message ID " << messageID << endl;

  emit serverMessageReceived (msg, unknown);

  if (unknown)
    kdWarning (11001) << k_funcinfo << ": received unknown message ID " << messageID << endl;
}

void KMessageClient::processFirstMessage()
{
  if (d->isLocked)
  {
    return;
  }
  if (d->delayedMessages.count() == 0)
  {
    kdDebug(11001) << k_funcinfo << ": no messages delayed" << endl;
    return;
  }
  QByteArray first = d->delayedMessages.front();
  d->delayedMessages.pop_front();
  processMessage (first);
}

void KMessageClient::removeBrokenConnection ()
{
  kdDebug (11001) << k_funcinfo << ": timer single shot for removeBrokenConnection"<<this << endl;
  // MH We cannot directly delete the socket. otherwise QSocket crashes
  QTimer::singleShot( 0, this, SLOT(removeBrokenConnection2()) );
  return;
}


void KMessageClient::removeBrokenConnection2 ()
{
  kdDebug (11001) << k_funcinfo << ": Broken:Deleting the connection object"<<this << endl;

  emit aboutToDisconnect(id());
  delete d->connection;
  d->connection = 0;
  d->adminID = 0;
  emit connectionBroken();
  kdDebug (11001) << k_funcinfo << ": Broken:Deleting the connection object DONE" << endl;
}

void KMessageClient::disconnect ()
{
  kdDebug (11001) << k_funcinfo << ": Disconnect:Deleting the connection object" << endl;

  emit aboutToDisconnect(id());
  delete d->connection;
  d->connection = 0;
  d->adminID = 0;
  emit connectionBroken();
  kdDebug (11001) << k_funcinfo << ": Disconnect:Deleting the connection object DONE" << endl;
}

void KMessageClient::lock ()
{
  d->isLocked = true;
}

void KMessageClient::unlock ()
{
  d->isLocked = false;
  for (unsigned int i = 0; i < d->delayedMessages.count(); i++)
  {
    QTimer::singleShot(0, this, SLOT(processFirstMessage()));
  }
}

unsigned int KMessageClient::delayedMessageCount() const
{
  return d->delayedMessages.count();
}

#include "kmessageclient.moc"
