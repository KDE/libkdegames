/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Burkhard Lehner <Burkhard.Lehner@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kmessageclient.h"

// own
#include "kmessageio.h"
#include "kmessageserver.h"
#include <kdegamesprivate_kgame_logging.h>
// Qt
#include <QBuffer>
#include <QDataStream>
#include <QList>
#include <QTimer>
// Std
#include <cstdio>

class KMessageClientPrivate
{
public:
    KMessageClientPrivate()
        : adminID(0)
        , connection(nullptr)
    {
    }

    ~KMessageClientPrivate()
    {
        delete connection;
    }

    quint32 adminID;
    QList<quint32> clientList;
    KMessageIO *connection;

    bool isLocked;
    QList<QByteArray> delayedMessages;
};

KMessageClient::KMessageClient(QObject *parent)
    : QObject(parent)
    , d(new KMessageClientPrivate)
{
    d->isLocked = false;
}

KMessageClient::~KMessageClient()
{
    d->delayedMessages.clear();
}

// -- setServer stuff

void KMessageClient::setServer(const QString &host, quint16 port)
{
    setServer(new KMessageSocket(host, port));
}

void KMessageClient::setServer(KMessageServer *server)
{
    KMessageDirect *serverIO = new KMessageDirect();
    setServer(new KMessageDirect(serverIO));
    server->addClient(serverIO);
}

void KMessageClient::setServer(KMessageIO *connection)
{
    if (d->connection) {
        delete d->connection;
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": We are changing the server!";
    }

    d->connection = connection;
    if (connection) {
        connect(connection, &KMessageIO::received, this, &KMessageClient::processIncomingMessage);
        connect(connection, &KMessageIO::connectionBroken, this, &KMessageClient::removeBrokenConnection);
    }
}

// -- id stuff

quint32 KMessageClient::id() const
{
    return (d->connection) ? d->connection->id() : 0;
}

bool KMessageClient::isAdmin() const
{
    return id() != 0 && id() == adminId();
}

quint32 KMessageClient::adminId() const
{
    return d->adminID;
}

QList<quint32> KMessageClient::clientList() const
{
    return d->clientList;
}

bool KMessageClient::isConnected() const
{
    return d->connection && d->connection->isConnected();
}

bool KMessageClient::isNetwork() const
{
    return isConnected() ? d->connection->isNetwork() : false;
}

quint16 KMessageClient::peerPort() const
{
    return d->connection ? d->connection->peerPort() : 0;
}

QString KMessageClient::peerName() const
{
    return d->connection ? d->connection->peerName() : QStringLiteral("localhost");
}

// --------------------- Sending messages

void KMessageClient::sendServerMessage(const QByteArray &msg)
{
    if (!d->connection) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": We have no connection yet!";
        return;
    }
    d->connection->send(msg);
}

void KMessageClient::sendBroadcast(const QByteArray &msg)
{
    QByteArray sendBuffer;
    QBuffer buffer(&sendBuffer);
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);

    stream << static_cast<quint32>(KMessageServer::REQ_BROADCAST);
    buffer.QIODevice::write(msg);
    sendServerMessage(sendBuffer);
}

void KMessageClient::sendForward(const QByteArray &msg, const QList<quint32> &clients)
{
    QByteArray sendBuffer;
    QBuffer buffer(&sendBuffer);
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);

    stream << static_cast<quint32>(KMessageServer::REQ_FORWARD) << clients;
    buffer.QIODevice::write(msg);
    sendServerMessage(sendBuffer);
}

void KMessageClient::sendForward(const QByteArray &msg, quint32 client)
{
    sendForward(msg, QList<quint32>() << client);
}

// --------------------- Receiving and processing messages

void KMessageClient::processIncomingMessage(const QByteArray &msg)
{
    if (d->isLocked) {
        d->delayedMessages.append(msg);
        return;
    }
    if (!d->delayedMessages.isEmpty()) {
        d->delayedMessages.append(msg);
        QByteArray first = d->delayedMessages.front();
        d->delayedMessages.pop_front();
        processMessage(first);
    } else {
        processMessage(msg);
    }
}

void KMessageClient::processMessage(const QByteArray &msg)
{
    if (d->isLocked) { // must NOT happen, since we check in processIncomingMessage as well as in processFirstMessage
        d->delayedMessages.append(msg);
        return;
    }
    QBuffer in_buffer;
    in_buffer.setData(msg);
    in_buffer.open(QIODevice::ReadOnly);
    QDataStream in_stream(&in_buffer);

    bool unknown = false;

    quint32 messageID;
    in_stream >> messageID;
    switch (messageID) {
    case KMessageServer::MSG_BROADCAST: {
        quint32 clientID;
        in_stream >> clientID;
        Q_EMIT broadcastReceived(in_buffer.readAll(), clientID);
    } break;

    case KMessageServer::MSG_FORWARD: {
        quint32 clientID;
        QList<quint32> receivers;
        in_stream >> clientID >> receivers;
        Q_EMIT forwardReceived(in_buffer.readAll(), clientID, receivers);
    } break;

    case KMessageServer::ANS_CLIENT_ID: {
        bool old_admin = isAdmin();
        quint32 clientID;
        in_stream >> clientID;
        d->connection->setId(clientID);
        if (old_admin != isAdmin())
            Q_EMIT adminStatusChanged(isAdmin());
    } break;

    case KMessageServer::ANS_ADMIN_ID: {
        bool old_admin = isAdmin();
        in_stream >> d->adminID;
        if (old_admin != isAdmin())
            Q_EMIT adminStatusChanged(isAdmin());
    } break;

    case KMessageServer::ANS_CLIENT_LIST: {
        in_stream >> d->clientList;
    } break;

    case KMessageServer::EVNT_CLIENT_CONNECTED: {
        quint32 id;
        in_stream >> id;

        if (d->clientList.contains(id))
            qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": Adding a client that already existed!";
        else
            d->clientList.append(id);

        Q_EMIT eventClientConnected(id);
    } break;

    case KMessageServer::EVNT_CLIENT_DISCONNECTED: {
        quint32 id;
        qint8 broken;
        in_stream >> id >> broken;

        if (!d->clientList.contains(id))
            qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": Removing a client that doesn't exist!";
        else
            d->clientList.removeAll(id);

        Q_EMIT eventClientDisconnected(id, bool(broken));
    } break;

    default:
        unknown = true;
    }

    if (!unknown && !in_buffer.atEnd())
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": Extra data received for message ID" << messageID;

    Q_EMIT serverMessageReceived(msg, unknown);

    if (unknown)
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": received unknown message ID" << messageID;
}

void KMessageClient::processFirstMessage()
{
    if (d->isLocked) {
        return;
    }
    if (d->delayedMessages.count() == 0) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": no messages delayed";
        return;
    }
    QByteArray first = d->delayedMessages.front();
    d->delayedMessages.pop_front();
    processMessage(first);
}

void KMessageClient::removeBrokenConnection()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": timer single shot for removeBrokenConnection" << this;
    // MH We cannot directly delete the socket. otherwise QSocket crashes
    QTimer::singleShot(0, this, &KMessageClient::removeBrokenConnection2);
    return;
}

void KMessageClient::removeBrokenConnection2()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Broken:Deleting the connection object" << this;

    Q_EMIT aboutToDisconnect(id());
    delete d->connection;
    d->connection = nullptr;
    d->adminID = 0;
    Q_EMIT connectionBroken();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Broken:Deleting the connection object DONE";
}

void KMessageClient::disconnect()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Disconnect:Deleting the connection object";

    Q_EMIT aboutToDisconnect(id());
    delete d->connection;
    d->connection = nullptr;
    d->adminID = 0;
    Q_EMIT connectionBroken();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Disconnect:Deleting the connection object DONE";
}

void KMessageClient::lock()
{
    d->isLocked = true;
}

void KMessageClient::unlock()
{
    d->isLocked = false;
    for (int i = 0; i < d->delayedMessages.count(); i++) {
        QTimer::singleShot(0, this, &KMessageClient::processFirstMessage);
    }
}

unsigned int KMessageClient::delayedMessageCount() const
{
    return d->delayedMessages.count();
}

#include "moc_kmessageclient.cpp"
