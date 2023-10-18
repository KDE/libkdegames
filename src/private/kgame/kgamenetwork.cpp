/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamenetwork.h"

// own
#include "kgameerror.h"
#include "kgamemessage.h"
#include "kmessageclient.h"
#include "kmessageio.h"
#include "kmessageserver.h"
#include <kdegamesprivate_kgame_logging.h>
// KF
#include <KDNSSD/PublicService>
// Qt
#include <QBuffer>
#include <QList>

class KGameNetworkPrivate
{
public:
    KGameNetworkPrivate() = default;

public:
    KMessageClient *mMessageClient = nullptr;
    KMessageServer *mMessageServer = nullptr;
    quint32 mDisconnectId = 0; // Stores gameId() over a disconnect process
    KDNSSD::PublicService *mService = nullptr;
    QString mType;
    QString mName;

    int mCookie;
};

// ------------------- NETWORK GAME ------------------------
KGameNetwork::KGameNetwork(int c, QObject *parent)
    : QObject(parent)
    , d(new KGameNetworkPrivate)
{
    d->mCookie = (qint16)c;

    // Init the game as a local game, i.e.
    // create your own KMessageServer and a KMessageClient connected to it.
    setMaster();

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "this=" << this << ", cookie=" << cookie() << "sizeof(this)=" << sizeof(KGameNetwork);
}

KGameNetwork::~KGameNetwork()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "this=" << this;
    // Debug();
    delete d->mService;
}

// ----------------------------- status methods
bool KGameNetwork::isNetwork() const
{
    return isOfferingConnections() || d->mMessageClient->isNetwork();
}

quint32 KGameNetwork::gameId() const
{
    // return d->mMessageClient->id() ;
    //  Return stored id in the case of disconnect. In any other
    //  case the disconnect id is 0
    if (d->mMessageClient->id() != 0) {
        return d->mMessageClient->id();
    } else {
        return d->mDisconnectId;
    }
}

int KGameNetwork::cookie() const
{
    return d->mCookie;
}

bool KGameNetwork::isMaster() const
{
    return (d->mMessageServer != nullptr);
}

bool KGameNetwork::isAdmin() const
{
    return (d->mMessageClient->isAdmin());
}

KMessageClient *KGameNetwork::messageClient() const
{
    return d->mMessageClient;
}

KMessageServer *KGameNetwork::messageServer() const
{
    return d->mMessageServer;
}

// ----------------------- network init
void KGameNetwork::setMaster()
{
    if (!d->mMessageServer) {
        d->mMessageServer = new KMessageServer(cookie(), this);
    } else {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "Server already running!!";
    }
    if (!d->mMessageClient) {
        d->mMessageClient = new KMessageClient(this);
        connect(d->mMessageClient, &KMessageClient::broadcastReceived, this, &KGameNetwork::receiveNetworkTransmission);
        connect(d->mMessageClient, &KMessageClient::connectionBroken, this, &KGameNetwork::signalConnectionBroken);
        connect(d->mMessageClient, &KMessageClient::aboutToDisconnect, this, &KGameNetwork::aboutToLoseConnection);
        connect(d->mMessageClient, &KMessageClient::connectionBroken, this, &KGameNetwork::slotResetConnection);

        connect(d->mMessageClient, &KMessageClient::adminStatusChanged, this, &KGameNetwork::slotAdminStatusChanged);
        connect(d->mMessageClient, &KMessageClient::eventClientConnected, this, &KGameNetwork::signalClientConnected);
        connect(d->mMessageClient, &KMessageClient::eventClientDisconnected, this, &KGameNetwork::signalClientDisconnected);

        // broacast and direct messages are treated equally on receive.
        connect(d->mMessageClient, &KMessageClient::forwardReceived, d->mMessageClient, &KMessageClient::broadcastReceived);

    } else {
        // should be no problem but still has to be tested
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Client already exists!";
    }
    d->mMessageClient->setServer(d->mMessageServer);
}

void KGameNetwork::setDiscoveryInfo(const QString &type, const QString &name)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << type << ":" << name;
    d->mType = type;
    d->mName = name;
    tryPublish();
}

void KGameNetwork::tryPublish()
{
    if (d->mType.isEmpty() || !isOfferingConnections())
        return;
    if (!d->mService)
        d->mService = new KDNSSD::PublicService(d->mName, d->mType, port());
    else {
        if (d->mType != d->mService->type())
            d->mService->setType(d->mType);
        if (d->mName != d->mService->serviceName())
            d->mService->setServiceName(d->mName);
    }
    if (!d->mService->isPublished())
        d->mService->publishAsync();
}

void KGameNetwork::tryStopPublishing()
{
    if (d->mService)
        d->mService->stop();
}

bool KGameNetwork::offerConnections(quint16 port)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "on port" << port;
    if (!isMaster()) {
        setMaster();
    }

    // Make sure this is 0
    d->mDisconnectId = 0;

    // FIXME: This debug message can be removed when the program is working correct.
    if (d->mMessageServer && d->mMessageServer->isOfferingConnections()) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Already running as server! Changing the port now!";
    }

    tryStopPublishing();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "before Server->initNetwork";
    if (!d->mMessageServer->initNetwork(port)) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "Unable to bind to port" << port << "!";
        // no need to delete - we just cannot listen to the port
        //   delete d->mMessageServer;
        //   d->mMessageServer = 0;
        //   d->mMessageClient->setServer((KMessageServer*)0);
        return false;
    }
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "after Server->initNetwork";
    tryPublish();
    return true;
}

bool KGameNetwork::connectToServer(const QString &host, quint16 port)
{
    if (host.isEmpty()) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << "No hostname given";
        return false;
    }
    if (connectToServer(new KMessageSocket(host, port))) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "connected to" << host << ":" << port;
        return true;
    } else {
        return false;
    }
}

bool KGameNetwork::connectToServer(KMessageIO *connection)
{
    // Make sure this is 0
    d->mDisconnectId = 0;

    // if (!d->mMessageServer) {
    //   // FIXME: What shall we do here? Probably must stop a running game.
    //   qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "We are already connected to another server!";
    /// }

    if (d->mMessageServer) {
        // FIXME: What shall we do here? Probably must stop a running game.
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "we are server but we are trying to connect to another server! "
                                             << "make sure that all clients connect to that server! "
                                             << "quitting the local server now...";
        stopServerConnection();
        d->mMessageClient->setServer((KMessageIO *)nullptr);
        delete d->mMessageServer;
        d->mMessageServer = nullptr;
    }

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "    about to set server";
    d->mMessageClient->setServer(connection);
    Q_EMIT signalAdminStatusChanged(false); // as we delete the connection above isAdmin() is always false now!

    // OK: We say that we already have connected, but this isn't so yet!
    // If the connection cannot be established, it will look as being disconnected
    // again ("slotConnectionLost" is called).
    // Shall we differ between these?
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "connected";
    return true;
}

quint16 KGameNetwork::port() const
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
    tryStopPublishing();
    if (d->mMessageServer) {
        d->mMessageServer->stopNetwork();
        return true;
    }
    return false;
}

bool KGameNetwork::isOfferingConnections() const
{
    return (d->mMessageServer && d->mMessageServer->isOfferingConnections());
}

void KGameNetwork::disconnect()
{
    // TODO MH
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG);
    stopServerConnection();
    if (d->mMessageServer) {
        const QList<quint32> list = d->mMessageServer->clientIDs();
        for (quint32 id : list) {
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Client id=" << id;
            KMessageIO *client = d->mMessageServer->findClient(id);
            if (!client) {
                continue;
            }
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "   rtti=" << client->rtti();
            if (client->rtti() == 2) {
                qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "DIRECT IO";
            } else {
                d->mMessageServer->removeClient(client, false);
            }
        }
    } else {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "before client->disconnect() id=" << gameId();
        // d->mMessageClient->setServer((KMessageIO*)0);
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        d->mMessageClient->disconnect();

        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "++++++--------------------------------------------+++++";
    }
    // setMaster();
    /*
    if (d->mMessageServer) {
     //delete d->mMessageServer;
     //d->mMessageServer=0;
     server=true;
     qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "  server true";
     d->mMessageServer->deleteClients();
     qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "  server deleteClients";
    }
    */
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "DONE";
}

void KGameNetwork::aboutToLoseConnection(quint32 clientID)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Storing client id of connection " << clientID;
    d->mDisconnectId = clientID;
}

void KGameNetwork::slotResetConnection()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Resseting client disconnect id";
    d->mDisconnectId = 0;
}

void KGameNetwork::electAdmin(quint32 clientID)
{
    if (!isAdmin()) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "only ADMIN is allowed to call this!";
        return;
    }
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << static_cast<quint32>(KMessageServer::REQ_ADMIN_CHANGE);
    stream << clientID;
    d->mMessageClient->sendServerMessage(buffer);
}

void KGameNetwork::setMaxClients(int max)
{
    if (!isAdmin()) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "only ADMIN is allowed to call this!";
        return;
    }
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << static_cast<quint32>(KMessageServer::REQ_MAX_NUM_CLIENTS);
    stream << (qint32)max;
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

bool KGameNetwork::sendSystemMessage(int data, int msgid, quint32 receiver, quint32 sender)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << data;
    return sendSystemMessage(buffer, msgid, receiver, sender);
}

bool KGameNetwork::sendSystemMessage(const QString &msg, int msgid, quint32 receiver, quint32 sender)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << msg;
    return sendSystemMessage(buffer, msgid, receiver, sender);
}

bool KGameNetwork::sendSystemMessage(const QDataStream &msg, int msgid, quint32 receiver, quint32 sender)
{
    return sendSystemMessage(((QBuffer *)msg.device())->buffer(), msgid, receiver, sender);
}

bool KGameNetwork::sendSystemMessage(const QByteArray &data, int msgid, quint32 receiver, quint32 sender)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (!sender) {
        sender = gameId();
    }

    quint32 receiverClient = KGameMessage::rawGameId(receiver); // KGame::gameId()
    int receiverPlayer = KGameMessage::rawPlayerId(receiver); // KPlayer::id()

    KGameMessage::createHeader(stream, sender, receiver, msgid);
    stream.writeRawData(data.data(), data.size());

    /*
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "transmitGameClientMessage msgid=" << msgid << "recv="
                   << receiver << "sender=" << sender << "Buffersize="
                   << buffer.size();
     */

    if (!d->mMessageClient) {
        // No client created, this should never happen!
        // Having a local game means we have our own
        // KMessageServer and we are the only client.
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "We don't have a client! Should never happen!";
        return false;
    }

    if (receiverClient == 0 || receiverPlayer != 0) {
        // if receiverClient == 0 this is a broadcast message. if it is != 0 but
        // receiverPlayer is also != 0 we have to send broadcast anyway, because the
        // KPlayer object on all clients needs to receive the message.
        d->mMessageClient->sendBroadcast(buffer);
    } else {
        d->mMessageClient->sendForward(buffer, receiverClient);
    }
    return true;
}

bool KGameNetwork::sendMessage(int data, int msgid, quint32 receiver, quint32 sender)
{
    return sendSystemMessage(data, msgid + KGameMessage::IdUser, receiver, sender);
}

bool KGameNetwork::sendMessage(const QString &msg, int msgid, quint32 receiver, quint32 sender)
{
    return sendSystemMessage(msg, msgid + KGameMessage::IdUser, receiver, sender);
}

bool KGameNetwork::sendMessage(const QDataStream &msg, int msgid, quint32 receiver, quint32 sender)
{
    return sendSystemMessage(msg, msgid + KGameMessage::IdUser, receiver, sender);
}

bool KGameNetwork::sendMessage(const QByteArray &msg, int msgid, quint32 receiver, quint32 sender)
{
    return sendSystemMessage(msg, msgid + KGameMessage::IdUser, receiver, sender);
}

void KGameNetwork::sendError(int error, const QByteArray &message, quint32 receiver, quint32 sender)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << (qint32)error;
    stream.writeRawData(message.data(), message.size());
    sendSystemMessage(stream, KGameMessage::IdError, receiver, sender);
}

// ----------------- receive messages from the network
void KGameNetwork::receiveNetworkTransmission(const QByteArray &receiveBuffer, quint32 clientID)
{
    QDataStream stream(receiveBuffer);
    int msgid;
    quint32 sender; // the id of the KGame/KPlayer who sent the message
    quint32 receiver; // the id of the KGame/KPlayer the message is for
    KGameMessage::extractHeader(stream, sender, receiver, msgid);
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "id=" << msgid << "sender=" << sender << "recv=" << receiver;

    // No broadcast : receiver==0
    // No player isPlayer(receiver)
    // Different game gameId()!=receiver
    if (receiver && receiver != gameId() && !KGameMessage::isPlayer(receiver)) {
        // receiver=0 is broadcast or player message
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Message not meant for us " << gameId() << "!=" << receiver << "rawid=" << KGameMessage::rawGameId(receiver);
        return;
    } else if (msgid == KGameMessage::IdError) {
        QString text;
        qint32 error;
        stream >> error;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Got IdError" << error;
        text = KGameError::errorText(error, stream);
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Error text:" << text.toLatin1();
        Q_EMIT signalNetworkErrorMessage((int)error, text);
    } else {
        networkTransmission(stream, msgid, receiver, sender, clientID);
    }
}

// -------------- slots for the signals of the client
void KGameNetwork::slotAdminStatusChanged(bool isAdmin)
{
    Q_EMIT signalAdminStatusChanged(isAdmin);

    // TODO: I'm pretty sure there are a lot of things that should be done here...
}

void KGameNetwork::Debug()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "------------------- KNETWORKGAME -------------------------";
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "gameId         " << gameId();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "gameMaster     " << isMaster();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "gameAdmin      " << isAdmin();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "---------------------------------------------------";
}

#include "moc_kgamenetwork.cpp"

/*
 * vim: et sw=2
 */
