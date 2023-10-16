/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Burkhard Lehner <Burkhard.Lehner@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kmessageserver.h"
#include "kmessageserver_p.h"

// own
#include "kmessageio.h"
#include <kdegamesprivate_kgame_logging.h>
// Qt
#include <QBuffer>
#include <QDataStream>
#include <QIODevice>
#include <QList>
#include <QQueue>
#include <QTimer>

// --------------- internal class KMessageServerSocket

KMessageServerSocket::KMessageServerSocket(quint16 port, QObject *parent)
    : QTcpServer(parent)
{
    listen(QHostAddress::Any, port);
    connect(this, &KMessageServerSocket::newConnection, this, &KMessageServerSocket::slotNewConnection);
}

KMessageServerSocket::~KMessageServerSocket()
{
}

void KMessageServerSocket::slotNewConnection()
{
    if (hasPendingConnections()) {
        Q_EMIT newClientConnected(new KMessageSocket(nextPendingConnection()));
    }
}

// ---------------- class for storing an incoming message

class MessageBuffer
{
public:
    MessageBuffer(quint32 clientID, const QByteArray &messageData)
        : id(clientID)
        , data(messageData)
    {
    }
    ~MessageBuffer()
    {
    }
    quint32 id;
    QByteArray data;
};

// ---------------- KMessageServer's private class

class KMessageServerPrivate
{
public:
    KMessageServerPrivate()
        : mMaxClients(-1)
        , mGameId(1)
        , mUniqueClientNumber(1)
        , mAdminID(0)
        , mServerSocket(nullptr)
    {
    }

    ~KMessageServerPrivate()
    {
        qDeleteAll(mClientList);
        qDeleteAll(mMessageQueue);
    }

    int mMaxClients;
    int mGameId;
    quint16 mCookie;
    quint32 mUniqueClientNumber;
    quint32 mAdminID;

    KMessageServerSocket *mServerSocket;

    QList<KMessageIO *> mClientList;
    QQueue<MessageBuffer *> mMessageQueue;
    QTimer mTimer;
    bool mIsRecursive;
};

// ------------------ KMessageServer

KMessageServer::KMessageServer(quint16 cookie, QObject *parent)
    : QObject(parent)
    , d(new KMessageServerPrivate)
{
    d->mIsRecursive = false;
    d->mCookie = cookie;
    connect(&(d->mTimer), &QTimer::timeout, this, &KMessageServer::processOneMessage);
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "CREATE(KMessageServer=" << this << ") cookie=" << d->mCookie << "sizeof(this)=" << sizeof(KMessageServer);
}

KMessageServer::~KMessageServer()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "this=" << this;
    Debug();
    stopNetwork();
    deleteClients();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "done";
}

//------------------------------------- TCP/IP server stuff

bool KMessageServer::initNetwork(quint16 port)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG);

    if (d->mServerSocket) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": We were already offering connections!";
        delete d->mServerSocket;
    }

    d->mServerSocket = new KMessageServerSocket(port);
    d->mIsRecursive = false;

    if (!d->mServerSocket || !d->mServerSocket->isListening()) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": Serversocket::ok() == false";
        delete d->mServerSocket;
        d->mServerSocket = nullptr;
        return false;
    }

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Now listening to port " << d->mServerSocket->serverPort();
    connect(d->mServerSocket, &KMessageServerSocket::newClientConnected, this, &KMessageServer::addClient);
    return true;
}

quint16 KMessageServer::serverPort() const
{
    if (d->mServerSocket)
        return d->mServerSocket->serverPort();
    else
        return 0;
}

void KMessageServer::stopNetwork()
{
    if (d->mServerSocket) {
        delete d->mServerSocket;
        d->mServerSocket = nullptr;
    }
}

bool KMessageServer::isOfferingConnections() const
{
    return d->mServerSocket != nullptr;
}

//----------------------------------------------- adding / removing clients

void KMessageServer::addClient(KMessageIO *client)
{
    QByteArray msg;

    // maximum number of clients reached?
    if (d->mMaxClients >= 0 && d->mMaxClients <= clientCount()) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": Maximum number of clients reached!";
        return;
    }

    // give it a unique ID
    client->setId(uniqueClientNumber());
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ":" << client->id();

    // connect its signals
    connect(client, &KMessageIO::connectionBroken, this, &KMessageServer::removeBrokenClient);
    connect(client, &KMessageIO::received, this, &KMessageServer::getReceivedMessage);

    // Tell everyone about the new guest
    // Note: The new client doesn't get this message!
    QDataStream(&msg, QIODevice::WriteOnly) << quint32(EVNT_CLIENT_CONNECTED) << client->id();
    broadcastMessage(msg);

    // add to our list
    d->mClientList.push_back(client);

    // tell it its ID
    QDataStream(&msg, QIODevice::WriteOnly) << quint32(ANS_CLIENT_ID) << client->id();
    client->send(msg);

    // Give it the complete list of client IDs
    QDataStream(&msg, QIODevice::WriteOnly) << quint32(ANS_CLIENT_LIST) << clientIDs();
    client->send(msg);

    if (clientCount() == 1) {
        // if it is the first client, it becomes the admin
        setAdmin(client->id());
    } else {
        // otherwise tell it who is the admin
        QDataStream(&msg, QIODevice::WriteOnly) << quint32(ANS_ADMIN_ID) << adminID();
        client->send(msg);
    }

    Q_EMIT clientConnected(client);
}

void KMessageServer::removeClient(KMessageIO *client, bool broken)
{
    quint32 clientID = client->id();
    if (!d->mClientList.removeAll(client)) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": Deleting client that wasn't added before!";
        return;
    }

    // tell everyone about the removed client
    QByteArray msg;
    QDataStream(&msg, QIODevice::WriteOnly) << quint32(EVNT_CLIENT_DISCONNECTED) << client->id() << (qint8)broken;
    broadcastMessage(msg);

    // If it was the admin, select a new admin.
    if (clientID == adminID()) {
        if (!d->mClientList.isEmpty())
            setAdmin(d->mClientList.front()->id());
        else
            setAdmin(0);
    }
}

void KMessageServer::deleteClients()
{
    qDeleteAll(d->mClientList);
    d->mClientList.clear();
    d->mAdminID = 0;
}

void KMessageServer::removeBrokenClient()
{
    KMessageIO *client = sender() ? qobject_cast<KMessageIO *>(sender()) : nullptr;
    if (!client) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": sender of the signal was not a KMessageIO object!";
        return;
    }

    Q_EMIT connectionLost(client);
    removeClient(client, true);
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

QList<quint32> KMessageServer::clientIDs() const
{
    QList<quint32> list;
    list.reserve(d->mClientList.size());
    for (QList<KMessageIO *>::iterator iter(d->mClientList.begin()); iter != d->mClientList.end(); ++iter)
        list.append((*iter)->id());
    return list;
}

KMessageIO *KMessageServer::findClient(quint32 no) const
{
    if (no == 0)
        no = d->mAdminID;

    QList<KMessageIO *>::iterator iter = d->mClientList.begin();
    while (iter != d->mClientList.end()) {
        if ((*iter)->id() == no)
            return (*iter);
        ++iter;
    }
    return nullptr;
}

quint32 KMessageServer::adminID() const
{
    return d->mAdminID;
}

void KMessageServer::setAdmin(quint32 adminID)
{
    // Trying to set the client that is already admin => nothing to do
    if (adminID == d->mAdminID)
        return;

    if (adminID > 0 && findClient(adminID) == nullptr) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << "Trying to set a new admin that doesn't exist!";
        return;
    }

    d->mAdminID = adminID;

    QByteArray msg;
    QDataStream(&msg, QIODevice::WriteOnly) << quint32(ANS_ADMIN_ID) << adminID;

    // Tell everyone about the new master
    broadcastMessage(msg);
}

//------------------------------------------- ID stuff

quint32 KMessageServer::uniqueClientNumber() const
{
    return d->mUniqueClientNumber++;
}

// --------------------- Messages ---------------------------

void KMessageServer::broadcastMessage(const QByteArray &msg)
{
    for (QList<KMessageIO *>::iterator iter(d->mClientList.begin()); iter != d->mClientList.end(); ++iter)
        (*iter)->send(msg);
}

void KMessageServer::sendMessage(quint32 id, const QByteArray &msg)
{
    KMessageIO *client = findClient(id);
    if (client)
        client->send(msg);
}

void KMessageServer::sendMessage(const QList<quint32> &ids, const QByteArray &msg)
{
    for (QList<quint32>::ConstIterator iter = ids.begin(); iter != ids.end(); ++iter)
        sendMessage(*iter, msg);
}

void KMessageServer::getReceivedMessage(const QByteArray &msg)
{
    KMessageIO *client = sender() ? qobject_cast<KMessageIO *>(sender()) : nullptr;
    if (!client) {
        qCCritical(KDEGAMESPRIVATE_KGAME_LOG) << ": slot was not called from KMessageIO!";
        return;
    }
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": size=" << msg.size();
    quint32 clientID = client->id();

    // QByteArray *ta=new QByteArray;
    // ta->duplicate(msg);
    // d->mMessageQueue.enqueue (new MessageBuffer (clientID, *ta));

    d->mMessageQueue.enqueue(new MessageBuffer(clientID, msg));
    if (!d->mTimer.isActive())
        d->mTimer.start(0); // AB: should be , TRUE i guess
}

void KMessageServer::processOneMessage()
{
    // This shouldn't happen, since the timer should be stopped before. But only to be sure!
    if (d->mMessageQueue.isEmpty()) {
        d->mTimer.stop();
        return;
    }
    if (d->mIsRecursive) {
        return;
    }
    d->mIsRecursive = true;

    MessageBuffer *msg_buf = d->mMessageQueue.head();

    quint32 clientID = msg_buf->id;
    QBuffer in_buffer(&msg_buf->data);
    in_buffer.open(QIODevice::ReadOnly);
    QDataStream in_stream(&in_buffer);

    QByteArray out_msg;
    QBuffer out_buffer(&out_msg);
    out_buffer.open(QIODevice::WriteOnly);
    QDataStream out_stream(&out_buffer);

    bool unknown = false;

    quint32 messageID;
    in_stream >> messageID;
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": got message with messageID=" << messageID;
    switch (messageID) {
    case REQ_BROADCAST:
        out_stream << quint32(MSG_BROADCAST) << clientID;
        // FIXME, compiler bug?
        // this should be okay, since QBuffer is subclass of QIODevice! :
        // out_buffer.write (in_buffer.readAll());
        out_buffer.QIODevice::write(in_buffer.readAll());
        broadcastMessage(out_msg);
        break;

    case REQ_FORWARD: {
        QList<quint32> clients;
        in_stream >> clients;
        out_stream << quint32(MSG_FORWARD) << clientID << clients;
        // see above!
        out_buffer.QIODevice::write(in_buffer.readAll());
        sendMessage(clients, out_msg);
    } break;

    case REQ_CLIENT_ID:
        out_stream << quint32(ANS_CLIENT_ID) << clientID;
        sendMessage(clientID, out_msg);
        break;

    case REQ_ADMIN_ID:
        out_stream << quint32(ANS_ADMIN_ID) << d->mAdminID;
        sendMessage(clientID, out_msg);
        break;

    case REQ_ADMIN_CHANGE:
        if (clientID == d->mAdminID) {
            quint32 newAdmin;
            in_stream >> newAdmin;
            setAdmin(newAdmin);
        }
        break;

    case REQ_REMOVE_CLIENT:
        if (clientID == d->mAdminID) {
            QList<quint32> client_list;
            in_stream >> client_list;
            for (QList<quint32>::Iterator iter = client_list.begin(); iter != client_list.end(); ++iter) {
                KMessageIO *client = findClient(*iter);
                if (client)
                    removeClient(client, false);
                else
                    qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": removing non-existing clientID";
            }
        }
        break;

    case REQ_MAX_NUM_CLIENTS:
        if (clientID == d->mAdminID) {
            qint32 maximum_clients;
            in_stream >> maximum_clients;
            setMaxClients(maximum_clients);
        }
        break;

    case REQ_CLIENT_LIST: {
        out_stream << quint32(ANS_CLIENT_LIST) << clientIDs();
        sendMessage(clientID, out_msg);
    } break;

    default:
        unknown = true;
    }

    // check if all the data has been used
    if (!unknown && !in_buffer.atEnd())
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": Extra data received for message ID" << messageID;

    Q_EMIT messageReceived(msg_buf->data, clientID, unknown);

    if (unknown)
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": received unknown message ID" << messageID;

    // remove the message, since we are ready with it
    delete d->mMessageQueue.dequeue();
    if (d->mMessageQueue.isEmpty())
        d->mTimer.stop();
    d->mIsRecursive = false;
}

void KMessageServer::Debug()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "------------------ KMESSAGESERVER -----------------------";
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "MaxClients :   " << maxClients();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "NoOfClients :  " << clientCount();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "---------------------------------------------------";
}

#include "moc_kmessageserver.cpp"
#include "moc_kmessageserver_p.cpp"
