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

#include <qiodevice.h>
#include <qbuffer.h>
#include <q3ptrlist.h>
#include <q3ptrqueue.h>
#include <qtimer.h>
#include <q3valuelist.h>

#include <kdebug.h>

#include "kmessageio.h"
#include "kmessageserver.h"

// --------------- internal class KMessageServerSocket

KMessageServerSocket::KMessageServerSocket (Q_UINT16 port, QObject *parent)
  : Q3ServerSocket (port, 0, parent)
{
}

KMessageServerSocket::~KMessageServerSocket ()
{
}

void KMessageServerSocket::newConnection (int socket)
{
  emit newClientConnected (new KMessageSocket (socket));
}

// ---------------- class for storing an incoming message

class MessageBuffer
{
  public:
    MessageBuffer (Q_UINT32 clientID, const QByteArray &messageData)
      : id (clientID), data (messageData) { }
    ~MessageBuffer () {}
    Q_UINT32 id;
    QByteArray data;
};

// ---------------- KMessageServer's private class

class KMessageServerPrivate
{
public:
  KMessageServerPrivate()
    : mMaxClients (-1), mGameId (1), mUniqueClientNumber (1), mAdminID (0), mServerSocket (0)
  {
    mClientList.setAutoDelete (true);
    mMessageQueue.setAutoDelete (true);
  }

  int mMaxClients;
  int mGameId;
  Q_UINT16 mCookie;
  Q_UINT32 mUniqueClientNumber;
  Q_UINT32 mAdminID;

  KMessageServerSocket* mServerSocket;

  Q3PtrList <KMessageIO> mClientList;
  Q3PtrQueue <MessageBuffer> mMessageQueue;
  QTimer mTimer;
  bool mIsRecursive;
};


// ------------------ KMessageServer

KMessageServer::KMessageServer (Q_UINT16 cookie,QObject* parent)
  : QObject(parent, 0)
{
  d = new KMessageServerPrivate;
  d->mIsRecursive=false;
  d->mCookie=cookie;
  connect (&(d->mTimer), SIGNAL (timeout()),
           this, SLOT (processOneMessage()));
  kdDebug(11001) << "CREATE(KMessageServer="
		<< this
		<< ") cookie="
		<< d->mCookie
		<< " sizeof(this)="
		<< sizeof(KMessageServer)
		<< endl;
}

KMessageServer::~KMessageServer()
{
  kdDebug(11001) << k_funcinfo << "this=" << this << endl;
  Debug();
  stopNetwork();
  deleteClients();
  delete d;
  kdDebug(11001) << k_funcinfo << " done" << endl;
}

//------------------------------------- TCP/IP server stuff

bool KMessageServer::initNetwork (Q_UINT16 port)
{
  kdDebug(11001) << k_funcinfo << endl;

  if (d->mServerSocket)
  {
    kdDebug (11001) << k_funcinfo << ": We were already offering connections!" << endl;
    delete d->mServerSocket;
  }

  d->mServerSocket = new KMessageServerSocket (port);
  d->mIsRecursive = false;

  if (!d->mServerSocket || !d->mServerSocket->ok())
  {
    kdError(11001) << k_funcinfo << ": Serversocket::ok() == false" << endl;
    delete d->mServerSocket;
    d->mServerSocket=0;
    return false;
  }

  kdDebug (11001) << k_funcinfo << ": Now listening to port "
                  << d->mServerSocket->port() << endl;
  connect (d->mServerSocket, SIGNAL (newClientConnected (KMessageIO*)),
           this, SLOT (addClient (KMessageIO*)));
  return true;
}

Q_UINT16 KMessageServer::serverPort () const
{
  if (d->mServerSocket)
    return d->mServerSocket->port();
  else
    return 0;
}

void KMessageServer::stopNetwork()
{
  if (d->mServerSocket) 
  {
    delete d->mServerSocket;
    d->mServerSocket = 0;
  }
}

bool KMessageServer::isOfferingConnections() const
{
  return d->mServerSocket != 0;
}

//----------------------------------------------- adding / removing clients

void KMessageServer::addClient (KMessageIO* client)
{
  QByteArray msg;

  // maximum number of clients reached?
  if (d->mMaxClients >= 0 && d->mMaxClients <= clientCount())
  {
    kdError (11001) << k_funcinfo << ": Maximum number of clients reached!" << endl;
    return;
  }

  // give it a unique ID
  client->setId (uniqueClientNumber());
  kdDebug (11001) << k_funcinfo << ": " << client->id() << endl;

  // connect its signals
  connect (client, SIGNAL (connectionBroken()),
           this, SLOT (removeBrokenClient()));
  connect (client, SIGNAL (received (const QByteArray &)),
           this, SLOT (getReceivedMessage (const QByteArray &)));

  // Tell everyone about the new guest
  // Note: The new client doesn't get this message!
  QDataStream (&msg, QIODevice::WriteOnly) << Q_UINT32 (EVNT_CLIENT_CONNECTED) << client->id();
  broadcastMessage (msg);

  // add to our list
  d->mClientList.append (client);

  // tell it its ID
  QDataStream (&msg, QIODevice::WriteOnly) << Q_UINT32 (ANS_CLIENT_ID) << client->id();
  client->send (msg);

  // Give it the complete list of client IDs
  QDataStream (&msg, QIODevice::WriteOnly)  << Q_UINT32 (ANS_CLIENT_LIST) << clientIDs();
  client->send (msg);


  if (clientCount() == 1)
  {
    // if it is the first client, it becomes the admin
    setAdmin (client->id());
  }
  else
  {
    // otherwise tell it who is the admin
    QDataStream (&msg, QIODevice::WriteOnly) << Q_UINT32 (ANS_ADMIN_ID) << adminID();
    client->send (msg);
  }

  emit clientConnected (client);
}

void KMessageServer::removeClient (KMessageIO* client, bool broken)
{
  Q_UINT32 clientID = client->id();
  if (!d->mClientList.removeRef (client))
  {
    kdError(11001) << k_funcinfo << ": Deleting client that wasn't added before!" << endl;
    return;
  }

  // tell everyone about the removed client
  QByteArray msg;
  QDataStream (&msg, QIODevice::WriteOnly) << Q_UINT32 (EVNT_CLIENT_DISCONNECTED) << client->id() << (Q_INT8)broken;
  broadcastMessage (msg);

  // If it was the admin, select a new admin.
  if (clientID == adminID())
  {
    if (!d->mClientList.isEmpty())
      setAdmin (d->mClientList.first()->id());
    else
      setAdmin (0);
  }
}

void KMessageServer::deleteClients()
{
  d->mClientList.clear();
  d->mAdminID = 0;
}

void KMessageServer::removeBrokenClient ()
{
  if (!sender()->inherits ("KMessageIO"))
  {
    kdError (11001) << k_funcinfo << ": sender of the signal was not a KMessageIO object!" << endl;
    return;
  }

  KMessageIO *client = (KMessageIO *) sender();

  emit connectionLost (client);
  removeClient (client, true);
}


void KMessageServer::setMaxClients(int c)
{
  d->mMaxClients = c;
}

int KMessageServer::maxClients() const
{
  return d->mMaxClients;
}

int KMessageServer::clientCount() const
{
  return d->mClientList.count();
}

Q3ValueList <Q_UINT32> KMessageServer::clientIDs () const
{
  Q3ValueList <Q_UINT32> list;
  for (Q3PtrListIterator <KMessageIO> iter (d->mClientList); *iter; ++iter)
    list.append ((*iter)->id());
  return list;
}

KMessageIO* KMessageServer::findClient (Q_UINT32 no) const
{
  if (no == 0)
    no = d->mAdminID;

  Q3PtrListIterator <KMessageIO> iter (d->mClientList);
  while (*iter)
  {
    if ((*iter)->id() == no)
      return (*iter);
    ++iter;
  }
  return 0;
}

Q_UINT32 KMessageServer::adminID () const
{
  return d->mAdminID;
}

void KMessageServer::setAdmin (Q_UINT32 adminID)
{
  // Trying to set the the client that is already admin => nothing to do
  if (adminID == d->mAdminID)
    return;

  if (adminID > 0 && findClient (adminID) == 0)
  {
    kdWarning (11001) << "Trying to set a new admin that doesn't exist!" << endl;
    return;
  }

  d->mAdminID = adminID;

  QByteArray msg;
  QDataStream (&msg, QIODevice::WriteOnly) << Q_UINT32 (ANS_ADMIN_ID) << adminID;

  // Tell everyone about the new master
  broadcastMessage (msg);
}


//------------------------------------------- ID stuff

Q_UINT32 KMessageServer::uniqueClientNumber() const
{
  return d->mUniqueClientNumber++;
}

// --------------------- Messages ---------------------------

void KMessageServer::broadcastMessage (const QByteArray &msg)
{
  for (Q3PtrListIterator <KMessageIO> iter (d->mClientList); *iter; ++iter)
    (*iter)->send (msg);
}

void KMessageServer::sendMessage (Q_UINT32 id, const QByteArray &msg)
{
  KMessageIO *client = findClient (id);
  if (client)
    client->send (msg);
}

void KMessageServer::sendMessage (const Q3ValueList <Q_UINT32> &ids, const QByteArray &msg)
{
  for (Q3ValueListConstIterator <Q_UINT32> iter = ids.begin(); iter != ids.end(); ++iter)
    sendMessage (*iter, msg);
}

void KMessageServer::getReceivedMessage (const QByteArray &msg)
{
  if (!sender() || !sender()->inherits("KMessageIO"))
  {
    kdError (11001) << k_funcinfo << ": slot was not called from KMessageIO!" << endl;
    return;
  }
  //kdDebug(11001) << k_funcinfo << ": size=" << msg.size() << endl;
  KMessageIO *client = (KMessageIO *) sender();
  Q_UINT32 clientID = client->id();

  //QByteArray *ta=new QByteArray;
  //ta->duplicate(msg);
  //d->mMessageQueue.enqueue (new MessageBuffer (clientID, *ta));

  
  d->mMessageQueue.enqueue (new MessageBuffer (clientID, msg));
  if (!d->mTimer.isActive())
    d->mTimer.start(0); // AB: should be , TRUE i guess
}

void KMessageServer::processOneMessage ()
{
  // This shouldn't happen, since the timer should be stopped before. But only to be sure!
  if (d->mMessageQueue.isEmpty())
  {
    d->mTimer.stop();
    return;
  }
  if (d->mIsRecursive)
  {
    return;
  }
  d->mIsRecursive = true;

  MessageBuffer *msg_buf = d->mMessageQueue.head();

  Q_UINT32 clientID = msg_buf->id;
  QBuffer in_buffer (&msg_buf->data);
  in_buffer.open (QIODevice::ReadOnly);
  QDataStream in_stream (&in_buffer);

  QByteArray out_msg;
  QBuffer out_buffer (&out_msg);
  out_buffer.open (QIODevice::WriteOnly);
  QDataStream out_stream (&out_buffer);

  bool unknown = false;

  QByteArray ttt=in_buffer.buffer();
  Q_UINT32 messageID;
  in_stream >> messageID;
  //kdDebug(11001) << k_funcinfo << ": got message with messageID=" << messageID << endl;
  switch (messageID)
  {
    case REQ_BROADCAST:
      out_stream << Q_UINT32 (MSG_BROADCAST) << clientID;
      // FIXME, compiler bug?
      // this should be okay, since QBuffer is subclass of QIODevice! :
      // out_buffer.writeBlock (in_buffer.readAll());
      out_buffer.QIODevice::writeBlock (in_buffer.readAll());
      broadcastMessage (out_msg);
      break;

    case REQ_FORWARD:
      {
        Q3ValueList <Q_UINT32> clients;
        in_stream >> clients;
        out_stream << Q_UINT32 (MSG_FORWARD) << clientID << clients;
        // see above!
        out_buffer.QIODevice::writeBlock (in_buffer.readAll());
        sendMessage (clients, out_msg);
      }
      break;

    case REQ_CLIENT_ID:
      out_stream << Q_UINT32 (ANS_CLIENT_ID) << clientID;
      sendMessage (clientID, out_msg);
      break;

    case REQ_ADMIN_ID:
      out_stream << Q_UINT32 (ANS_ADMIN_ID) << d->mAdminID;
      sendMessage (clientID, out_msg);
      break;

    case REQ_ADMIN_CHANGE:
      if (clientID == d->mAdminID)
      {
        Q_UINT32 newAdmin;
        in_stream >> newAdmin;
        setAdmin (newAdmin);
      }
      break;

    case REQ_REMOVE_CLIENT:
      if (clientID == d->mAdminID)
      {
        Q3ValueList <Q_UINT32> client_list;
        in_stream >> client_list;
        for (Q3ValueListIterator <Q_UINT32> iter = client_list.begin(); iter != client_list.end(); ++iter)
        {
          KMessageIO *client = findClient (*iter);
          if (client)
            removeClient (client, false);
          else
            kdWarning (11001) << k_funcinfo << ": removing non-existing clientID" << endl;
        }
      }
      break;

    case REQ_MAX_NUM_CLIENTS:
      if (clientID == d->mAdminID)
      {
        Q_INT32 maximum_clients;
        in_stream >> maximum_clients;
        setMaxClients (maximum_clients);
      }
      break;

    case REQ_CLIENT_LIST:
      {
        out_stream << Q_UINT32 (ANS_CLIENT_LIST) << clientIDs();
        sendMessage (clientID, out_msg);
      }
      break;

    default:
      unknown = true;
  }

  // check if all the data has been used
  if (!unknown && !in_buffer.atEnd())
    kdWarning (11001) << k_funcinfo << ": Extra data received for message ID " << messageID << endl;

  emit messageReceived (msg_buf->data, clientID, unknown);

  if (unknown)
    kdWarning (11001) << k_funcinfo << ": received unknown message ID " << messageID << endl;

  // remove the message, since we are ready with it
  d->mMessageQueue.remove();
  if (d->mMessageQueue.isEmpty())
    d->mTimer.stop();
  d->mIsRecursive = false;
}

void KMessageServer::Debug()
{
   kdDebug(11001) << "------------------ KMESSAGESERVER -----------------------" << endl;
   kdDebug(11001) << "MaxClients :   " << maxClients() << endl;
   kdDebug(11001) << "NoOfClients :  " << clientCount() << endl;
   kdDebug(11001) << "---------------------------------------------------" << endl;
}

#include "kmessageserver.moc"
