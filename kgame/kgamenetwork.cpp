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

#include <qbuffer.h>

#include <kdebug.h>
#include <klocale.h>

#include "kplayer.h"
#include "kgameio.h"
#include "kgamenetwork.h"
#include "kgamemessage.h"

#include "kmessageserver.h"
#include "kmessageclient.h"

class KGameNetworkPrivate
{
public:
	KGameNetworkPrivate()
	{
		mMessageClient = 0;
		mMessageServer = 0;
	}

public:
	KMessageClient* mMessageClient;
	KMessageServer* mMessageServer;
	QList<KGameIO> mMessageListener;

	int mCookie;
};

// ------------------- NETWORK GAME ------------------------
KGameNetwork::KGameNetwork(int cookie,QObject* parent) : QObject(parent, 0)
{
 d = new KGameNetworkPrivate;

 // Init the game as a local game, i.e.
 // create your own KMessageServer and a KMessageClient connected to it.
 d->mMessageServer = new KMessageServer (cookie, this);
 d->mMessageClient = new KMessageClient (this);
 d->mMessageClient->setServer (d->mMessageServer);

 connect (d->mMessageClient, SIGNAL(broadcastReceived(const QByteArray&, Q_UINT32)),
          this, SLOT(receiveNetworkTransmission(const QByteArray&, Q_UINT32)));
 connect (d->mMessageClient, SIGNAL(connectionBroken()),
          this, SIGNAL(signalConnectionBroken())); // TODO: Return to a non-network game?
 connect (d->mMessageClient, SIGNAL(adminStatusChanged(bool)),
          this, SLOT(slotAdminStatusChanged(bool)));
 connect (d->mMessageClient, SIGNAL(eventClientConnected(Q_UINT32)),
          this, SIGNAL(signalClientConnected(Q_UINT32)));
 connect (d->mMessageClient, SIGNAL(eventClientDisconnected(Q_UINT32, bool)),
          this, SIGNAL(signalClientDisconnected(Q_UINT32, bool)));

 d->mCookie = (Q_INT16)cookie;
 kdDebug(11001) << "CREATE(KGameNetwork=" << this <<") cookie=" << d->mCookie << " sizeof(this)="<<sizeof(KGameNetwork) << endl;
}

KGameNetwork::~KGameNetwork()
{
 kdDebug(11001) << "DESTRUCT(KGameNetwork=" << this <<")" << endl;
 Debug();
 delete d;
}

// ----------------------------- status methods
Q_UINT32 KGameNetwork::gameId() const
{ return d->mMessageClient->id(); }

int KGameNetwork::cookie() const
{ return d->mCookie; }

bool KGameNetwork::isMaster() const
{ return (d->mMessageServer != 0); }

bool KGameNetwork::isAdmin() const
{ return (d->mMessageClient->isAdmin()); }

// ----------------------- network init
bool KGameNetwork::offerConnections (Q_UINT16 port)
{
 if (!d->mMessageServer) {
   // When we are connected to a server in another process, we stop the network game here.
   // FIXME: Probably there is more to do here! (Stop a running game, etc.)
   kdWarning (11001) << "KGameNetwork::offerConnections: We are connected to another client!" << endl
                     << "        We close that connection and create our own server here!" << endl;

   // Create our own server again and connect to it.
   d->mMessageServer = new KMessageServer (d->mCookie, this);
   d->mMessageClient->setServer (d->mMessageServer);
 }

 // FIXME: This debug message can be removed when the program is working correct.
 if (d->mMessageServer && d->mMessageServer->isOfferingConnections()) {
   kdDebug (11001) << "KGameNetwork::offerConnections: Already running as server! Changing the port now!" << endl;
 }

 if (!d->mMessageServer->initNetwork (port)) {
   kdError (11001) << "KGameNetwork::offerConnections: Unable to bind to port " << port << "!" << endl;
   delete d->mMessageServer;
   d->mMessageServer = 0;
   return false;
 }

 return true;
}

bool KGameNetwork::connectToServer (const QString& host, Q_UINT16 port)
{
 if (host.isEmpty()) {
   kdError(11001) << "KGameNetwork::connectToServer: No hostname given" << endl;
   return false;
 }

// if (!d->mMessageServer) {
//   // FIXME: What shall we do here? Probably must stop a running game.
//   kdWarning (11001) << "KGameNetwork::connectToServer: We are already connected to another server!" << endl;
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

 d->mMessageClient->setServer (host, port);

 // FIXME: We say that we already have connected, but this isn't so yet!
 // If the connection cannot be established, it will look as being disconnected
 // again ("slotConnectionLost" is called).
 // Shall we differ between these?

 kdDebug(11001) << "connected to " << host << ":" << port << endl;
 return true;
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

// --------------------- send messages ---------------------------

bool KGameNetwork::sendSystemMessage(int data, int msgid, int receiver, int sender)
{
 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 stream << data;
 return sendSystemMessage(buffer,msgid,receiver,sender);
}

bool KGameNetwork::sendSystemMessage(const QString &msg, int msgid, int receiver, int sender)
{
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << msg;
 return sendSystemMessage(buffer, msgid, receiver, sender);
}

bool KGameNetwork::sendSystemMessage(const QDataStream &msg, int msgid, int receiver, int sender)
{ return sendSystemMessage(((QBuffer*)msg.device())->buffer(), msgid, receiver, sender); }

bool KGameNetwork::sendSystemMessage(const QByteArray& data, int msgid, int receiver, int sender)
{
 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 if (!sender) {
   sender = (int)KGameMessage::calcMessageId(gameId(), 0);
 }

 KGameMessage::createHeader(stream, sender, receiver, msgid);
 stream.writeRawBytes(data.data(), data.size());
 kdDebug(11001) << "transmitGameClientMessage msgid=" << msgid << " recv="
                << receiver << " sender=" << sender << " Buffersize="
                << buffer.size() << endl;
// kdDebug(11001) << "   cookie=" << cookie() << " version="
//    << KGameMessage::version() << endl;

 if (!d->mMessageClient) {
   // No client created, this should never happen!
   // Having a local game means we have our own
   // KMessageServer and we are the only client.
   kdWarning (11001) << "KGameNetwork::sendSystemMessage: We don't have a client! Should never happen!" << endl;
   return false;
 }

 d->mMessageClient->sendBroadcast(buffer);
 return true;
}

bool KGameNetwork::sendMessage(int data, int msgid, int receiver, int sender)
{ return sendSystemMessage(data,msgid+KGameMessage::IdUser,receiver,sender); }

bool KGameNetwork::sendMessage(const QString &msg, int msgid, int receiver, int sender)
{ return sendSystemMessage(msg,msgid+KGameMessage::IdUser,receiver,sender); }

bool KGameNetwork::sendMessage(const QDataStream &msg, int msgid, int receiver, int sender)
{ return sendSystemMessage(msg, msgid+KGameMessage::IdUser, receiver, sender); }

bool KGameNetwork::sendMessage(const QByteArray &msg, int msgid, int receiver, int sender)
{ return sendSystemMessage(msg, msgid+KGameMessage::IdUser, receiver, sender); }

void KGameNetwork::sendError(int error,const QString& text,int receiver,int sender)
{
 QByteArray buffer;
 QDataStream stream(buffer,IO_WriteOnly);
 stream << (Q_INT32) error << text;
 sendSystemMessage(stream,KGameMessage::IdError,receiver,sender);
}

bool KGameNetwork::sendGameIOMessage(const QByteArray& sendBuffer, int msgid, KGameIO *client,int receiver, int sender)
{
 if (!client) {
   return false;
 }

 QByteArray buffer;
 buffer.duplicate(sendBuffer);

 bool userMessage = msgid > KGameMessage::IdUser;
 if (userMessage) {
   msgid -= KGameMessage::IdUser;
 }

 QDataStream ostream(buffer,IO_ReadOnly);
 int m, r, s;
 KGameMessage::extractHeader(ostream, m, r, s);
 if (!userMessage) {
   client->receiveSystemMessage(ostream, msgid, receiver, sender);
 } else {
   client->receiveMessage(ostream, msgid, receiver, sender);
 }
 return true;
}

// ----------------- receive messages from the network
void KGameNetwork::receiveNetworkTransmission(const QByteArray& receiveBuffer, Q_UINT32 clientID)
{
 QDataStream stream(receiveBuffer, IO_ReadOnly);
 int msgid;
 int sender; // the id of the KGame/KPlayer who sent the message - see KGameMessage::calcMessageId()
 int receiver; // the id of the KGame/KPlayer the message is for - see KGameMessage::calcMessageId()
 KGameMessage::extractHeader(stream, sender, receiver, msgid);
 Q_UINT32 gameid = KGameMessage::calcGameId(receiver);
 kdDebug(11001) << "------ receiveNetworkTransmission(): id=" << msgid << " sender=" << sender << " recv=" << receiver << endl;

 // Forward to registered KGameIO clients
 KGameIO *input;
 for ( input = d->mMessageListener.first(); input != 0; input = d->mMessageListener.next() ) {
   if (input == d->mMessageListener.first())  {
     kdDebug(11001) << "forwarding to IOs ; id=" << msgid << " recv=" << receiver
                    << " sender=" << sender << endl;
//                    << " Buffersize=" << buffer.size()
//                    << "   cookie=" << cookie() << " version="
//                    << KGameMessage::version() << endl;
   }
   sendGameIOMessage(receiveBuffer, msgid, input, receiver, sender);
 }

// kdDebug(11001) << "============= ReceiveNetworkTransmission (" << msgid << "," << receiver << "," << sender << ") ===========" << endl;
 if (gameid && gameid!=gameId()) { // gameid=0 is broadcast or player message
   kdDebug(11001) << "KGameNetwork::receiveNetworkTransmission: Message not meant for us " << gameId() << "!=" << gameid << endl;
   return;
 } else if (msgid==KGameMessage::IdError) {
   QString text;
   Q_INT32 error;
   stream >> error >> text;
   kdDebug(11001) << "KGame::receiveNetworkTransmission: Got IdError " << error << endl;
   kdDebug(11001) << "Error text: " << text.latin1() << endl;
   emit signalNetworkErrorMessage((int)error,text);
 } else if (msgid == 0 && receiver == 0) {
     // TODO: Must be supported, but who shall do it?
 /*
   kdDebug(11001) << "receive new id old id: " << client->id() << endl;
   int newid;
   stream >> newid;
   client->setId(newid);
   setGameId(newid);// is this correct? is the local client->id() == gameId()?
   kdDebug(11001) << "receive new id new id: " << client->id() << endl;
 */
 } else {
   networkTransmission(stream, msgid, receiver, sender, clientID);
 }
}

// -------------------- Listener ---------------------------
bool KGameNetwork::registerListener (KGameIO *l)
{
 kdDebug(11001) << "KGameNetwork::registerListener " << l << endl;
 if (!l) {
   return false;
 }
 d->mMessageListener.append(l);
 return true;
}

bool KGameNetwork::unregisterListener(KGameIO *l)
{
 kdDebug(11001) << "KGameNetwork::unregisterListener " << l << endl;
 return d->mMessageListener.remove(l);
}

// -------------- slots for the signals of the client
void KGameNetwork::slotAdminStatusChanged(bool isAdmin)
{
//FIXME TODO
}

void KGameNetwork::Debug()
{
 kdDebug(11001) << "------------------- KNETWORKGAME -------------------------" << endl;
 kdDebug(11001) << "gameId         " << gameId() << endl;
 kdDebug(11001) << "gameMaster     " << isMaster() << endl;
 kdDebug(11001) << "gameAdmin      " << isAdmin() << endl;
 kdDebug(11001) << "---------------------------------------------------" << endl;
}

#include "kgamenetwork.moc"
