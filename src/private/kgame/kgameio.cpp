/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgameio.h"

// own
#include "kgame.h"
#include "kgamemessage.h"
#include "kmessageio.h"
#include "kplayer.h"
#include <kdegamesprivate_kgame_logging.h>
// Qt
#include <QBuffer>
#include <QEvent>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>
// Std
#include <cstdlib>

class KGameIOPrivate
{
public:
    KGameIOPrivate() = default;
    virtual ~KGameIOPrivate() = default;

public:
    KPlayer *mPlayer = nullptr;
};

// ----------------------- Generic IO -------------------------
KGameIO::KGameIO()
    : KGameIO(*new KGameIOPrivate)
{
}

KGameIO::KGameIO(KPlayer *player)
    : KGameIO(*new KGameIOPrivate, player)
{
}

KGameIO::KGameIO(KGameIOPrivate &dd, KPlayer *player)
    : d(&dd)
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": this=" << this << ", sizeof(this)" << sizeof(KGameIO);
    if (player) {
        player->addGameIO(this);
    }
}

KGameIO::~KGameIO()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": this=" << this;
    // unregister ourselves
    if (player()) {
        player()->removeGameIO(this, false);
    }
}

KPlayer *KGameIO::player() const
{
    return d->mPlayer;
}

void KGameIO::setPlayer(KPlayer *p)
{
    d->mPlayer = p;
}

void KGameIO::initIO(KPlayer *p)
{
    setPlayer(p);
}

void KGameIO::notifyTurn(bool b)
{
    if (!player()) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": player() is NULL";
        return;
    }
    bool sendit = false;
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    Q_EMIT signalPrepareTurn(stream, b, this, &sendit);
    if (sendit) {
        QDataStream ostream(buffer);
        quint32 sender = player()->id(); // force correct sender
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Prepare turn sendInput";
        sendInput(ostream, true, sender);
    }
}

KGame *KGameIO::game() const
{
    if (!player()) {
        return nullptr;
    }
    return player()->game();
}

bool KGameIO::sendInput(QDataStream &s, bool transmit, quint32 sender)
{
    if (!player()) {
        return false;
    }
    return player()->forwardInput(s, transmit, sender);
}

void KGameIO::Debug()
{
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "------------------- KGAMEINPUT --------------------";
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "this:    " << this;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "rtti :   " << rtti();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Player:  " << player();
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "---------------------------------------------------";
}

// ----------------------- Key IO ---------------------------
class KGameKeyIOPrivate : public KGameIOPrivate
{
};

KGameKeyIO::KGameKeyIO(QWidget *parent)
    : KGameIO(*new KGameKeyIOPrivate)
{
    if (parent) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Key Event filter installed";
        parent->installEventFilter(this);
    }
}

KGameKeyIO::~KGameKeyIO()
{
    if (parent()) {
        parent()->removeEventFilter(this);
    }
}

int KGameKeyIO::rtti() const
{
    return KeyIO;
}

bool KGameKeyIO::eventFilter(QObject *o, QEvent *e)
{
    if (!player()) {
        return false;
    }

    // key press/release
    if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
        QKeyEvent *k = (QKeyEvent *)e;
        //   qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "KGameKeyIO" << this << "key press/release" <<  k->key();
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        bool eatevent = false;
        Q_EMIT signalKeyEvent(this, stream, k, &eatevent);
        QDataStream msg(buffer);

        if (eatevent && sendInput(msg)) {
            return eatevent;
        }
        return false; // do not eat otherwise
    }
    return QObject::eventFilter(o, e); // standard event processing
}

// ----------------------- Mouse IO ---------------------------
class KGameMouseIOPrivate : public KGameIOPrivate
{
};

KGameMouseIO::KGameMouseIO(QWidget *parent, bool trackmouse)
    : KGameIO(*new KGameMouseIOPrivate)
{
    if (parent) {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Mouse Event filter installed tracking=" << trackmouse;
        parent->installEventFilter(this);
        parent->setMouseTracking(trackmouse);
    }
}

KGameMouseIO::KGameMouseIO(QGraphicsScene *parent, bool /*trackmouse*/)
    : KGameIO(*new KGameMouseIOPrivate)
{
    if (parent) {
        // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Mouse Event filter installed tracking=" << trackmouse;
        parent->installEventFilter(this);
        //     parent->setMouseTracking(trackmouse);
    }
}

KGameMouseIO::~KGameMouseIO()
{
    if (parent()) {
        parent()->removeEventFilter(this);
    }
}

int KGameMouseIO::rtti() const
{
    return MouseIO;
}

void KGameMouseIO::setMouseTracking(bool b)
{
    if (parent()) {
        ((QWidget *)parent())->setMouseTracking(b);
    }
}

bool KGameMouseIO::eventFilter(QObject *o, QEvent *e)
{
    if (!player()) {
        return false;
    }
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "KGameMouseIO" << this << QLatin1String( " " ) << e->type();

    // mouse action
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease || e->type() == QEvent::MouseButtonDblClick
        || e->type() == QEvent::Wheel || e->type() == QEvent::MouseMove || e->type() == QEvent::GraphicsSceneMousePress
        || e->type() == QEvent::GraphicsSceneMouseRelease || e->type() == QEvent::GraphicsSceneMouseDoubleClick || e->type() == QEvent::GraphicsSceneWheel
        || e->type() == QEvent::GraphicsSceneMouseMove) {
        QMouseEvent *k = (QMouseEvent *)e;
        // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "KGameMouseIO" << this;
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        bool eatevent = false;
        Q_EMIT signalMouseEvent(this, stream, k, &eatevent);
        // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "################# eatevent=" << eatevent;
        QDataStream msg(buffer);
        if (eatevent && sendInput(msg)) {
            return eatevent;
        }
        return false; // do not eat otherwise
    }
    return QObject::eventFilter(o, e); // standard event processing
}

// ----------------------- KGameProcesPrivate ---------------------------
class KGameProcessIOPrivate : public KGameIOPrivate
{
public:
    KGameProcessIOPrivate() = default;

public:
    // KMessageServer *mMessageServer = nullptr;
    // KMessageClient *mMessageClient = nullptr;
    KMessageProcess *mProcessIO = nullptr;
};

// ----------------------- Process IO ---------------------------
KGameProcessIO::KGameProcessIO(const QString &name)
    : KGameIO(*new KGameProcessIOPrivate)
{
    Q_D(KGameProcessIO);

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": this=" << this << ", sizeof(this)=" << sizeof(KGameProcessIO);

    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "================= KMEssageServer ====================";
    // d->mMessageServer=new KMessageServer(0,this);
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "================= KMEssageClient ====================";
    // d->mMessageClient=new KMessageClient(this);
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "================= KMEssageProcessIO ====================";
    d->mProcessIO = new KMessageProcess(this, name);
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "================= KMEssage Add client ====================";
    // d->mMessageServer->addClient(d->mProcessIO);
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "================= KMEssage SetSErver ====================";
    // d->mMessageClient->setServer(d->mMessageServer);
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "================= KMEssage: Connect ====================";
    // connect(d->mMessageClient, SIGNAL(broadcastReceived(QByteArray,quint32)),
    //         this, SLOT(clientMessage(QByteArray,quint32)));
    // connect(d->mMessageClient, SIGNAL(forwardReceived(QByteArray,quint32,QValueList<quint32>)),
    //         this, SLOT(clientMessage(QByteArray,quint32,QValueList<quint32>)));
    connect(d->mProcessIO, &KMessageProcess::received, this, &KGameProcessIO::receivedMessage);
    // Relay signal
    connect(d->mProcessIO, &KMessageProcess::signalReceivedStderr, this, &KGameProcessIO::signalReceivedStderr);
    // qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Our client is id="<<d->mMessageClient->id();
}

KGameProcessIO::~KGameProcessIO()
{
    Q_D(KGameProcessIO);

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": this=" << this;
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "player=" << player();
    if (player()) {
        player()->removeGameIO(this, false);
    }
    if (d->mProcessIO) {
        delete d->mProcessIO;
        d->mProcessIO = nullptr;
    }
}

int KGameProcessIO::rtti() const
{
    return ProcessIO;
}

void KGameProcessIO::initIO(KPlayer *p)
{
    KGameIO::initIO(p);
    // Send 'hello' to process
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    bool sendit = true;
    if (p) {
        qint16 id = p->userId();
        stream << id;
        Q_EMIT signalIOAdded(this, stream, p, &sendit);
        if (sendit) {
            quint32 sender = p->id();
            qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Sending IOAdded to process player !!!!!!!!!!!!!! ";
            sendSystemMessage(stream, KGameMessage::IdIOAdded, 0, sender);
        }
    }
}

void KGameProcessIO::notifyTurn(bool b)
{
    if (!player()) {
        qCWarning(KDEGAMESPRIVATE_KGAME_LOG) << ": player() is NULL";
        return;
    }
    bool sendit = true;
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << (qint8)b;
    Q_EMIT signalPrepareTurn(stream, b, this, &sendit);
    if (sendit) {
        quint32 sender = player()->id();
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Sending Turn to process player !!!!!!!!!!!!!! ";
        sendSystemMessage(stream, KGameMessage::IdTurn, 0, sender);
    }
}

void KGameProcessIO::sendSystemMessage(QDataStream &stream, int msgid, quint32 receiver, quint32 sender)
{
    sendAllMessages(stream, msgid, receiver, sender, false);
}

void KGameProcessIO::sendMessage(QDataStream &stream, int msgid, quint32 receiver, quint32 sender)
{
    sendAllMessages(stream, msgid, receiver, sender, true);
}

void KGameProcessIO::sendAllMessages(QDataStream &stream, int msgid, quint32 receiver, quint32 sender, bool usermsg)
{
    Q_D(KGameProcessIO);

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "==============>  KGameProcessIO::sendMessage (usermsg=" << usermsg << ")";
    // if (!player()) return ;
    // if (!player()->isActive()) return ;

    if (usermsg) {
        msgid += KGameMessage::IdUser;
    }

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "=============* ProcessIO (" << msgid << "," << receiver << "," << sender << ") ===========";

    QByteArray buffer;
    QDataStream ostream(&buffer, QIODevice::WriteOnly);
    QBuffer *device = (QBuffer *)stream.device();
    QByteArray data = device->buffer();
    ;

    KGameMessage::createHeader(ostream, sender, receiver, msgid);
    // ostream.writeRawBytes(data.data()+device->at(),data.size()-device->at());
    ostream.writeRawData(data.data(), data.size());
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "   Adding user data from pos=" << device->pos() << " amount=" << data.size() << "byte";
    // if (d->mMessageClient) d->mMessageClient->sendBroadcast(buffer);
    if (d->mProcessIO) {
        d->mProcessIO->send(buffer);
    }
}

// void KGameProcessIO::clientMessage(const QByteArray& receiveBuffer, quint32 clientID, const QValueList <quint32> &recv)
void KGameProcessIO::receivedMessage(const QByteArray &receiveBuffer)
{
    QDataStream stream(receiveBuffer);
    int msgid;
    quint32 sender;
    quint32 receiver;
    KGameMessage::extractHeader(stream, sender, receiver, msgid);

    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "************* Got process message sender =" << sender << "receiver=" << receiver << "   msgid=" << msgid;

    // Cut out the header part...to not confuse network code
    QBuffer *buf = (QBuffer *)stream.device();
    QByteArray newbuffer;
    newbuffer = QByteArray::fromRawData(buf->buffer().data() + buf->pos(), buf->size() - buf->pos());
    QDataStream ostream(newbuffer);
    qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << "Newbuffer size=" << newbuffer.size();

    // This is a dummy message which allows us the process to talk with its owner
    if (msgid == KGameMessage::IdProcessQuery) {
        Q_EMIT signalProcessQuery(ostream, this);
    } else if (player()) {
        sender = player()->id(); // force correct sender
        if (msgid == KGameMessage::IdPlayerInput) {
            sendInput(ostream, true, sender);
        } else {
            player()->forwardMessage(ostream, msgid, receiver, sender);
        }
    } else {
        qCDebug(KDEGAMESPRIVATE_KGAME_LOG) << ": Got message from process but no player defined!";
    }
    newbuffer.clear();
}

// ----------------------- Computer IO --------------------------
class KGameComputerIOPrivate : public KGameIOPrivate
{
    // TODO: maybe these should be KGameProperties!!
public:
    KGameComputerIOPrivate() = default;

public:
    int mAdvanceCounter = 0;
    int mReactionPeriod = 0;

    int mPauseCounter = 0;

    QTimer *mAdvanceTimer = nullptr;
};

KGameComputerIO::KGameComputerIO()
    : KGameIO(*new KGameComputerIOPrivate)
{
}

KGameComputerIO::KGameComputerIO(KPlayer *p)
    : KGameIO(*new KGameComputerIOPrivate, p)
{
}

KGameComputerIO::~KGameComputerIO()
{
    Q_D(KGameComputerIO);

    if (d->mAdvanceTimer) {
        delete d->mAdvanceTimer;
    }
}

int KGameComputerIO::rtti() const
{
    return ComputerIO;
}

void KGameComputerIO::setReactionPeriod(int calls)
{
    Q_D(KGameComputerIO);

    d->mReactionPeriod = calls;
}

int KGameComputerIO::reactionPeriod() const
{
    Q_D(const KGameComputerIO);

    return d->mReactionPeriod;
}

void KGameComputerIO::setAdvancePeriod(int ms)
{
    Q_D(KGameComputerIO);

    stopAdvancePeriod();
    d->mAdvanceTimer = new QTimer(this);
    connect(d->mAdvanceTimer, &QTimer::timeout, this, &KGameComputerIO::advance);
    d->mAdvanceTimer->start(ms);
}

void KGameComputerIO::stopAdvancePeriod()
{
    Q_D(KGameComputerIO);

    if (d->mAdvanceTimer) {
        d->mAdvanceTimer->stop();
        delete d->mAdvanceTimer;
    }
}

void KGameComputerIO::pause(int calls)
{
    Q_D(KGameComputerIO);

    d->mPauseCounter = calls;
}

void KGameComputerIO::unpause()
{
    pause(0);
}

void KGameComputerIO::advance()
{
    Q_D(KGameComputerIO);

    if (d->mPauseCounter > 0) {
        d->mPauseCounter--;
        return;
    } else if (d->mPauseCounter < 0) {
        return;
    }
    d->mAdvanceCounter++;
    if (d->mAdvanceCounter >= d->mReactionPeriod) {
        d->mAdvanceCounter = 0;
        reaction();
    }
}

void KGameComputerIO::reaction()
{
    Q_EMIT signalReaction();
}

#include "moc_kgameio.cpp"
