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

#include "kgamenetwork.h"
#include "kgamenetwork.moc"
#include "kgamemessage.h"
#include "kgameerror.h"

#include "kmessageserver.h"
#include "kmessageclient.h"
#include "kmessageio.h"

#include <kdebug.h>

#include <qbuffer.h>


class KGameNetworkPrivate
{
public:
        KGameNetworkPrivate()
        {
                mMessageClient = 0;
                mMessageServer = 0;
                mDisconnectId = 0;
        }

public:
        KMessageClient* mMessageClient;
        KMessageServer* mMessageServer;
        Q_UINT32 mDisconnectId;  // Stores gameId() over a disconnect process

        int mCookie;
};

// ------------------- NETWORK GAME ------------------------
KGameNetwork::KGameNetwork(int c, QObject* parent) : QObject(parent, 0)
{
 d = new KGameNetworkPrivate;
 d->mCookie = (Q_INT16)c;

 // Init the game as a local game, i.e.
 // create your own KMessageServer and a KMessageClient connected to it.
 setMaster();

 kdDebug(11001) << k_funcinfo << "this=" << this <<", cookie=" << cookie() << " sizeof(this)="<<sizeof(KGameNetwork) << endl;
}

KGameNetwork::~KGameNetwork()
{
 kdDebug(11001) << k_funcinfo << "this=" << this << endl;
// Debug();
 delete d;
}

// ----------------------------- status methods
bool KGameNetwork::isNetwork() const
{ return isOfferingConnections() || d->mMessageClient->isNetwork();}

Q_UINT32 KGameNetwork::gameId() const
{
  //return d->mMessageClient->id() ;
  // Return stored id in the case of disconnect. In any other
  // case the disconnect id is 0
  if (d->mMessageClient->id()!=0 ) {
    return d->mMessageClient->id() ;
  } else {
    return d->mDisconnectId;
  }
}

int KGameNetwork::cookie() const
{ return d->mCookie; }

bool KGameNetwork::isMaster() const
{ return (d->mMessageServer != 0); }

bool KGameNetwork::isAdmin() const
{ return (d->mMessageClient->isAdmin()); }

KMessageClient* KGameNetwork::messageClient() const
{ return d->mMessageClient; }

KMessageServer* KGameNetwork::messageServer() const
{ return d->mMessageServer; }

// ----------------------- network init
void KGameNetwork::setMaster()
{
 if (!d->mMessageServer) {
   d->mMessageServer = new KMessageServer (cookie(), this);
 } else {
   kdWarning(11001) << k_funcinfo << "Server already running!!" << endl;
 }
 if (!d->mMessageClient) {
   d->mMessageClient = new KMessageClient (this);
   connect (d->mMessageClient, SIGNAL(broadcastReceived(const QByteArray&, Q_UINT32)),
            this, SLOT(receiveNetworkTransmission(const QByteArray&, Q_UINT32)));
   connect (d->mMessageClient, SIGNAL(connectionBroken()),
            this, SIGNAL(signalConnectionBroken()));
   connect (d->mMessageClient, SIGNAL(aboutToDisconnect(Q_UINT32)),
            this, SLOT(aboutToLoseConnection(Q_UINT32)));
   connect (d->mMessageClient, SIGNAL(connectionBroken()),
            this, SLOT(slotResetConnection()));

   connect (d->mMessageClient, SIGNAL(adminStatusChanged(bool)),
            this, SLOT(slotAdminStatusChanged(bool)));
   connect (d->mMessageClient, SIGNAL(eventClientConnected(Q_UINT32)),
            this, SIGNAL(signalClientConnected(Q_UINT32)));
   connect (d->mMessageClient, SIGNAL(eventClientDisconnected(Q_UINT32, bool)),
            this, SIGNAL(signalClientDisconnected(Q_UINT32, bool)));

   // broacast and direct messages are treated equally on receive.
   connect (d->mMessageClient, SIGNAL(forwardReceived(const QByteArray&, Q_UINT32, const QValueList<Q_UINT32>&)),
            d->mMessageClient, SIGNAL(broadcastReceived(const QByteArray&, Q_UINT32)));

 } else {
   // should be no problem but still has to be tested
   kdDebug(11001) << k_funcinfo << "Client already exists!" << endl;
 }
 d->mMessageClient->setServer(d->mMessageServer);
}

bool KGameNetwork::offerConnections(Q_UINT16 port)
{
 kdDebug (11001) << k_funcinfo << "on port " << port << endl;
 if (!isMaster()) {
   setMaster();
 }

 // Make sure this is 0
 d->mDisconnectId = 0;

 // FIXME: This debug message can be removed when the program is working correct.
 if (d->mMessageServer && d->mMessageServer->isOfferingConnections()) {
   kdDebug (11001) << k_funcinfo << "Already running as server! Changing the port now!" << endl;
 }

 kdDebug (11001) << k_funcinfo << "before Server->initNetwork" << endl;
 if (!d->mMessageServer->initNetwork (port)) {
   kdError (11001) << k_funcinfo << "Unable to bind to port " << port << "!" << endl;
   // no need to delete - we just cannot listen to the port
//   delete d->mMessageServer;
//   d->mMessageServer = 0;
//   d->mMessageClient->setServer((KMessageServer*)0);
   return false;
 }
 kdDebug (11001) << k_funcinfo << "after Server->initNetwork" << endl;
 return true;
}

bool KGameNetwork::connectToServer (const QString& host, Q_UINT16 port)
{
 if (host.isEmpty()) {
   kdError(11001) << k_funcinfo << "No hostname given" << endl;
   return false;
 }

 // Make sure this is 0
 d->mDisconnectId = 0;

// if (!d->mMessageServer) {
//   // FIXME: What shall we do here? Probably must stop a running game.
//   kdWarning (11001) << k_funcinfo << "We are already connected to another server!" << endl;
/// }

 if (d->mMessageServer) {
   // FIXME: What shall we do here? Probably must stop a running game.
   kdWarning(11001) << "we are server but we are trying to connect to another server! "
                    << "make sure that all clients connect to that server! "
                    << "quitting the local server now..." << endl;
   stopServerConnection();
   d->mMessageClient->setServer((KMessageIO*)0);
   delete d->mMessageServer;
   d->mMessageServer = 0;
 }

 kdDebug(11001) << "    about to set server" << endl;
 d->mMessageClient->setServer(host, port);
 emit signalAdminStatusChanged(false); // as we delete the connection above isAdmin() is always false now!

 // OK: We say that we already have connected, but this isn't so yet!
 // If the connection cannot be established, it will look as being disconnected
 // again ("slotConnectionLost" is called).
 // Shall we differ between these?
 kdDebug(11001) << "connected to " << host << ":" << port << endl;
 return true;
}

Q_UINT16 KGameNetwork::port() const
{
 if (isNetwork()) {
   if (isOfferingConnections()) {
     return d->mMessageServer->serverPort();
   } else {
     return d->mMessageClient->peerPort();
   }
 }
 return 0;
}

QString KGameNetwork::hostName() const
{
 return d->mMessageClient->peerName();
}

bool KGameNetwork::stopServerConnection()
{
 // We still are the Master, we just don't accept further connections!
 if (d->mMessageServer) {
   d->mMessageServer->stopNetwork();
   return true;
 }
 return false;
}

bool KGameNetwork::isOfferingConnections() const
{ return (d->mMessageServer && d->mMessageServer->isOfferingConnections()); }

void KGameNetwork::disconnect()
{
 // TODO MH
 kdDebug(11001) << k_funcinfo << endl;
 stopServerConnection();
 if (d->mMessageServer) {
    QValueList <Q_UINT32> list=d->mMessageServer->clientIDs();
    QValueList<Q_UINT32>::Iterator it;
    for( it = list.begin(); it != list.end(); ++it )
    {
      kdDebug(11001) << "Client id=" << (*it) <<  endl;
      KMessageIO *client=d->mMessageServer->findClient(*it);
      if (!client)
      {
        continue;
      }
      kdDebug(11001) << "   rtti=" << client->rtti() <<  endl;
      if (client->rtti()==2)
      {
        kdDebug(11001) << "DIRECT IO " << endl;
      }
      else
      {
        d->mMessageServer->removeClient(client,false);
      }
    }
 }
 else
 {
   kdDebug(11001) << k_funcinfo << "before client->disconnect() id="<<gameId()<< endl;
   //d->mMessageClient->setServer((KMessageIO*)0);
   kdDebug(11001) << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;
   d->mMessageClient->disconnect();

   kdDebug(11001) << "++++++--------------------------------------------+++++"<<endl;
 }
 //setMaster();
 /*
 if (d->mMessageServer) {
  //delete d->mMessageServer;
  //d->mMessageServer=0;
  server=true;
  kdDebug(11001) << "  server true" << endl;
  d->mMessageServer->deleteClients();
  kdDebug(11001) << "  server deleteClients" << endl;
 }
 */
 kdDebug(11001) << k_funcinfo << "DONE" << endl;
}

void KGameNetwork::aboutToLoseConnection(Q_UINT32 clientID)
{
  kdDebug(11001) << "Storing client id of connection "<<clientID<<endl;
  d->mDisconnectId = clientID;
}

void KGameNetwork::slotResetConnection()
{
  kdDebug(11001) << "Resseting client disconnect id"<<endl;
  d->mDisconnectId = 0;
}

void KGameNetwork::electAdmin(Q_UINT32 clientID)
{
 if (!isAdmin()) {
	kdWarning(11001) << k_funcinfo << "only ADMIN is allowed to call this!" << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 stream << static_cast<Q_UINT32>( KMessageServer::REQ_ADMIN_CHANGE );
 stream << clientID;
 d->mMessageClient->sendServerMessage(buffer);
}

void KGameNetwork::setMaxClients(int max)
{
 if (!isAdmin()) {
	kdWarning(11001) << k_funcinfo << "only ADMIN is allowed to call this!" << endl;
	return;
 }
 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 stream << static_cast<Q_UINT32>( KMessageServer::REQ_MAX_NUM_CLIENTS );
 stream << (Q_INT32)max;
 d->mMessageClient->sendServerMessage(buffer);
}

void KGameNetwork::lock()
{
 if (messageClient()) {
   messageClient()->lock();
 }
}

void KGameNetwork::unlock()
{
 if (messageClient()) {
   messageClient()->unlock();
 }
}

// --------------------- send messages ---------------------------

bool KGameNetwork::sendSystemMessage(int data, int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{
 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 stream << data;
 return sendSystemMessage(buffer,msgid,receiver,sender);
}

bool KGameNetwork::sendSystemMessage(const QString &msg, int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << msg;
 return sendSystemMessage(buffer, msgid, receiver, sender);
}

bool KGameNetwork::sendSystemMessage(const QDataStream &msg, int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{ return sendSystemMessage(((QBuffer*)msg.device())->buffer(), msgid, receiver, sender); }

bool KGameNetwork::sendSystemMessage(const QByteArray& data, int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{
 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 if (!sender) {
   sender = gameId();
 }

 Q_UINT32 receiverClient = KGameMessage::rawGameId(receiver); // KGame::gameId()
 int receiverPlayer = KGameMessage::rawPlayerId(receiver); // KPlayer::id()

 KGameMessage::createHeader(stream, sender, receiver, msgid);
 stream.writeRawBytes(data.data(), data.size());

 /*
 kdDebug(11001) << "transmitGameClientMessage msgid=" << msgid << " recv="
                << receiver << " sender=" << sender << " Buffersize="
                << buffer.size() << endl;
  */

 if (!d->mMessageClient) {
   // No client created, this should never happen!
   // Having a local game means we have our own
   // KMessageServer and we are the only client.
   kdWarning (11001) << k_funcinfo << "We don't have a client! Should never happen!" << endl;
   return false;
 }

 if (receiverClient == 0 || receiverPlayer != 0)
 {
   // if receiverClient == 0 this is a broadcast message. if it is != 0 but
   // receiverPlayer is also != 0 we have to send broadcast anyway, because the
   // KPlayer object on all clients needs to receive the message.
   d->mMessageClient->sendBroadcast(buffer);
 }
 else
 {
   d->mMessageClient->sendForward(buffer, receiverClient);
 }
 return true;
}

bool KGameNetwork::sendMessage(int data, int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{ return sendSystemMessage(data,msgid+KGameMessage::IdUser,receiver,sender); }

bool KGameNetwork::sendMessage(const QString &msg, int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{ return sendSystemMessage(msg,msgid+KGameMessage::IdUser,receiver,sender); }

bool KGameNetwork::sendMessage(const QDataStream &msg, int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{ return sendSystemMessage(msg, msgid+KGameMessage::IdUser, receiver, sender); }

bool KGameNetwork::sendMessage(const QByteArray &msg, int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{ return sendSystemMessage(msg, msgid+KGameMessage::IdUser, receiver, sender); }

void KGameNetwork::sendError(int error,const QByteArray& message, Q_UINT32 receiver, Q_UINT32 sender)
{
 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 stream << (Q_INT32) error;
 stream.writeRawBytes(message.data(), message.size());
 sendSystemMessage(stream,KGameMessage::IdError,receiver,sender);
}


// ----------------- receive messages from the network
void KGameNetwork::receiveNetworkTransmission(const QByteArray& receiveBuffer, Q_UINT32 clientID)
{
 QDataStream stream(receiveBuffer, IO_ReadOnly);
 int msgid;
 Q_UINT32 sender; // the id of the KGame/KPlayer who sent the message
 Q_UINT32 receiver; // the id of the KGame/KPlayer the message is for 
 KGameMessage::extractHeader(stream, sender, receiver, msgid);
// kdDebug(11001) << k_funcinfo << "id=" << msgid << " sender=" << sender << " recv=" << receiver << endl;

 // No broadcast : receiver==0
 // No player isPlayer(receiver)
 // Different game gameId()!=receiver
 if (receiver &&  receiver!=gameId() && !KGameMessage::isPlayer(receiver) )
 {
   // receiver=0 is broadcast or player message
   kdDebug(11001) << k_funcinfo << "Message not meant for us "
            << gameId() << "!=" << receiver << " rawid="
            << KGameMessage::rawGameId(receiver) << endl;
   return;
 }
 else if (msgid==KGameMessage::IdError)
 {
   QString text;
   Q_INT32 error;
   stream >> error;
   kdDebug(11001) << k_funcinfo << "Got IdError " << error << endl;
   text = KGameError::errorText(error, stream);
   kdDebug(11001) << "Error text: " << text.latin1() << endl;
   emit signalNetworkErrorMessage((int)error,text);
 }
 else
 {
   networkTransmission(stream, msgid, receiver, sender, clientID);
 }
}

// -------------- slots for the signals of the client
void KGameNetwork::slotAdminStatusChanged(bool isAdmin)
{
 emit signalAdminStatusChanged(isAdmin);

// TODO: I'm pretty sure there are a lot of things that should be done here...
}

void KGameNetwork::Debug()
{
 kdDebug(11001) << "------------------- KNETWORKGAME -------------------------" << endl;
 kdDebug(11001) << "gameId         " << gameId() << endl;
 kdDebug(11001) << "gameMaster     " << isMaster() << endl;
 kdDebug(11001) << "gameAdmin      " << isAdmin() << endl;
 kdDebug(11001) << "---------------------------------------------------" << endl;
}

/*
 * vim: et sw=2
 */
