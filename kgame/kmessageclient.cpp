/* **************************************************************************
                           KMessageClient Class
                           -------------------
    begin                : 17 April 2001
    copyright            : (C) 2001 by Burkhard Lehner
    email                : Burkhard.Lehner@gmx.de
***************************************************************************/

#include <kdebug.h>

#include <qbuffer.h>

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
};

KMessageClient::KMessageClient (QObject *parent, const char *name)
  : QObject (parent, name)
{
  d = new KMessageClientPrivate ();
}

KMessageClient::~KMessageClient ()
{
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
// AB: TODO: check if connection was really established!
// seems that the game is crashing if you try to connect to a server that
// doesn't exist - probably the bug is here. For socket connections check e.g.
// if socket == -1 and (if so) abort
  if (d->connection)
  {
    delete d->connection;
    kdDebug (11001) << "KMessageClient::setServer: We are changing the server!" << endl;
  }

  d->connection = connection;
  if (connection)
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

bool KMessageClient::isConnected ()
{
  return d->connection && d->connection->isConnected();
}

// --------------------- Sending messages

void KMessageClient::sendServerMessage (const QByteArray &msg)
{
  if (!d->connection)
  {
    kdWarning (11001) << "KMessageClient::sendServerMessage: We have no connection yet!" << endl;
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

  stream << KMessageServer::REQ_BROADCAST;
  buffer.QIODevice::writeBlock (msg);
  sendServerMessage (sendBuffer);
}

void KMessageClient::sendForward (const QByteArray &msg, const QValueList <Q_UINT32> &clients)
{
  QByteArray sendBuffer;
  QBuffer buffer (sendBuffer);
  buffer.open (IO_WriteOnly);
  QDataStream stream (&buffer);

  stream << KMessageServer::REQ_FORWARD << clients;
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
          kdWarning (11001) << "KMessageClient::processIncomingMessage: Adding a client that already existed!" << endl;
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
          kdWarning (11001) << "KMessageClient::processIncomingMessage: Removing a client that doesn't exist!" << endl;
        else
          d->clientList.remove (id);

        emit eventClientDisconnected (id, bool (broken));
      }
      break;

    default:
      unknown = true;
  }

  if (!unknown && !in_buffer.atEnd())
    kdWarning (11001) << "KMessageClient::processIncomingMessage: Extra data received for message ID " << messageID << endl;

  emit serverMessageReceived (msg, unknown);

  if (unknown)
    kdWarning (11001) << "KMessageClient::processIncomingMessage: received unknown message ID " << messageID << endl;
}

void KMessageClient::removeBrokenConnection ()
{
  kdDebug (11001) << "KMessageClient::removeBrokenConnection: Deleting the connection object" << endl;
  emit connectionBroken();

  delete d->connection;
  d->connection = 0;
  d->adminID = 0;
}

#include "kmessageclient.moc"

