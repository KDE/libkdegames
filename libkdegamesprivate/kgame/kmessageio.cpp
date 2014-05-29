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

/*
     KMessageIO class and subclasses KMessageSocket and KMessageDirect
*/

#include "kmessageio.h"
#include <QTcpSocket>
#include <kprocess.h>
#include <QFile>
#include <QDataStream>
// ----------------------- KMessageIO -------------------------

KMessageIO::KMessageIO (QObject *parent)
  : QObject (parent), m_id (0)
{
  QLoggingCategory::setFilterRules(QLatin1Literal("games.private.kgame.debug = true")); 
}

KMessageIO::~KMessageIO ()
{}

void KMessageIO::setId (quint32 id)
{
  m_id = id;
}

quint32 KMessageIO::id ()
{
  return m_id;
}

// ----------------------KMessageSocket -----------------------

KMessageSocket::KMessageSocket (const QString& host, quint16 port, QObject *parent)
  : KMessageIO (parent)
{
  mSocket = new QTcpSocket ();
  mSocket->connectToHost (host, port);
  initSocket ();
}

KMessageSocket::KMessageSocket (QHostAddress host, quint16 port, QObject *parent)
  : KMessageIO (parent)
{
  mSocket = new QTcpSocket ();
  mSocket->connectToHost (host.toString(), port);
  initSocket ();
}

KMessageSocket::KMessageSocket (QTcpSocket *socket, QObject *parent)
  : KMessageIO (parent)
{
  mSocket = socket;
  initSocket ();
}

KMessageSocket::KMessageSocket (int socketFD, QObject *parent)
  : KMessageIO (parent)
{
  mSocket = new QTcpSocket ();
  mSocket->setSocketDescriptor (socketFD);
  initSocket ();
}

KMessageSocket::~KMessageSocket ()
{
  delete mSocket;
}

bool KMessageSocket::isConnected () const
{
  return mSocket->state() == QAbstractSocket::ConnectedState;
}

void KMessageSocket::send (const QByteArray &msg)
{
  QDataStream str (mSocket);
  str << quint8 ('M');  // magic number for begin of message
  str.writeBytes (msg.data(), msg.size());  // writes the length (as quint32) and the data
}

void KMessageSocket::processNewData ()
{
  if (isRecursive)
    return;
  isRecursive = true;

  QDataStream str (mSocket);
  while (mSocket->bytesAvailable() > 0)
  {
    if (mAwaitingHeader)
    {
      // Header = magic number + packet length = 5 bytes
      if (mSocket->bytesAvailable() < 5)
      {
        isRecursive = false;
        return;
      }

      // Read the magic number first. If something unexpected is found,
      // start over again, ignoring the data that was read up to then.

      quint8 v;
      str >> v;
      if (v != 'M')
      {
        qCWarning(GAMES_PRIVATE_KGAME) << ": Received unexpected data, magic number wrong!";
        continue;
      }

      str >> mNextBlockLength;
      mAwaitingHeader = false;
    }
    else
    {
      // Data not completely read => wait for more
      if (mSocket->bytesAvailable() < (qint64) mNextBlockLength)
      {
        isRecursive = false;
        return;
      }

      QByteArray msg (mNextBlockLength, 0);
      str.readRawData (msg.data(), mNextBlockLength);

      // send the received message
      emit received (msg);

      // Waiting for the header of the next message
      mAwaitingHeader = true;
    }
  }

  isRecursive = false;
}

void KMessageSocket::initSocket ()
{
  connect (mSocket, SIGNAL (error(QAbstractSocket::SocketError)), this, SIGNAL (connectionBroken()));
  connect (mSocket, SIGNAL (disconnected()), this, SIGNAL (connectionBroken()));
  connect (mSocket, SIGNAL (readyRead()), this, SLOT (processNewData()));
  mAwaitingHeader = true;
  mNextBlockLength = 0;
  isRecursive = false;
}

quint16 KMessageSocket::peerPort () const
{
  return mSocket->peerPort();
}

QString KMessageSocket::peerName () const
{
  return mSocket->peerName();
}

// ----------------------KMessageDirect -----------------------

KMessageDirect::KMessageDirect (KMessageDirect *partner, QObject *parent)
  : KMessageIO (parent), mPartner (0)
{
  // 0 as first parameter leaves the object unconnected
  if (!partner)
    return;

  // Check if the other object is already connected
  if (partner && partner->mPartner)
  {
    qCWarning(GAMES_PRIVATE_KGAME) << ": Object is already connected!";
    return;
  }

  // Connect from us to that object
  mPartner = partner;

  // Connect the other object to us
  partner->mPartner = this;
}

KMessageDirect::~KMessageDirect ()
{
  if (mPartner)
  {
    mPartner->mPartner = 0;
    emit mPartner->connectionBroken();
  }
}

bool KMessageDirect::isConnected () const
{
  return mPartner != 0;
}

void KMessageDirect::send (const QByteArray &msg)
{
  if (mPartner)
    emit mPartner->received (msg);
  else
    qCCritical(GAMES_PRIVATE_KGAME) << ": Not yet connected!";
}


// ----------------------- KMessageProcess ---------------------------

KMessageProcess::~KMessageProcess()
{
  qCDebug(GAMES_PRIVATE_KGAME) << "@@@KMessageProcess::Delete process";
  if (mProcess)
  {
    mProcess->kill();
    mProcess->deleteLater();
    mProcess=0;
    // Maybe todo: delete mSendBuffer
  }
}
KMessageProcess::KMessageProcess(QObject *parent, const QString& file) : KMessageIO(parent)
{
  // Start process
  qCDebug(GAMES_PRIVATE_KGAME) << "@@@KMessageProcess::Start process";
  mProcessName=file;
  mProcess=new KProcess;
  // Need both stdout and stderr as separate channels in the communication
  mProcess-> setOutputChannelMode(KProcess::SeparateChannels);
  int id=0;
  *mProcess << mProcessName << QString::fromLatin1( "%1").arg(id);
  qCDebug(GAMES_PRIVATE_KGAME) << "@@@KMessageProcess::Init:Id=" << id;
  qCDebug(GAMES_PRIVATE_KGAME) << "@@@KMessgeProcess::Init:Processname:" << mProcessName;
  connect(mProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReceivedStdout()));
  connect(mProcess, SIGNAL(readyReadStandardError()),  this, SLOT(slotReceivedStderr()));
  connect(mProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                        this, SLOT(slotProcessExited(int,QProcess::ExitStatus)));
  mProcess->start();
  mSendBuffer=0;
  mReceiveCount=0;
  mReceiveBuffer.resize(1024);
}
bool KMessageProcess::isConnected() const
{
  qCDebug(GAMES_PRIVATE_KGAME) << "@@@KMessageProcess::Is conencted";
  if (!mProcess)
     return false;
  return (mProcess->state() == QProcess::Running);
}

// Send to process
void KMessageProcess::send(const QByteArray &msg)
{
  qCDebug(GAMES_PRIVATE_KGAME) << "@@@KMessageProcess:: SEND("<<msg.size()<<") to process";
  unsigned int size=msg.size()+2*sizeof(long);

  if (mProcess == 0) {
    qCDebug(GAMES_PRIVATE_KGAME) << "@@@KMessageProcess:: cannot write to stdin, no process available";
    return;
  }

  char *tmpbuffer=new char[size];
  long *p1=(long *)tmpbuffer;
  long *p2=p1+1;
  qCDebug(GAMES_PRIVATE_KGAME)  << "p1="<<p1 << "p2="<< p2;
  memcpy(tmpbuffer+2*sizeof(long),msg.data(),msg.size());
  *p1=0x4242aeae;
  *p2=size;

  // no need to add it to a queue -> qiodevice is buffered
  mProcess->write(tmpbuffer,size);
  delete [] tmpbuffer;
}

void KMessageProcess::slotReceivedStderr()
{
  QByteArray ba;
  qCDebug(GAMES_PRIVATE_KGAME)<<"@@@ KMessageProcess::slotReceivedStderr";

  mProcess->setReadChannel(QProcess::StandardError);
  while(mProcess->canReadLine())
  {
    ba = mProcess->readLine();
    if( ba.isEmpty() )
      return;
    ba.chop( 1 );   // remove QLatin1Char( '\n' )

    qCDebug(GAMES_PRIVATE_KGAME) << "KProcess (" << ba.size() << "):" << ba.constData();
    emit signalReceivedStderr(QLatin1String( ba ));
    ba.clear();
  };
}


void KMessageProcess::slotReceivedStdout()
{
  mProcess->setReadChannel(QProcess::StandardOutput);
  QByteArray ba = mProcess->readAll();
  qCDebug(GAMES_PRIVATE_KGAME) << "$$$$$$ " << ": Received" << ba.size() << "bytes over inter process communication";

  // Resize receive buffer
  while (mReceiveCount+ba.size()>=mReceiveBuffer.size()) mReceiveBuffer.resize(mReceiveBuffer.size()+1024);
  // was 08/2007: mReceiveBuffer += ba;
  qCopy(ba.begin(), ba.begin()+ba.size(), mReceiveBuffer.begin()+mReceiveCount);
  mReceiveCount += ba.size();

  // Possbile message
  while (mReceiveCount>int(2*sizeof(long)))
  {
    long *p1=(long *)mReceiveBuffer.data();
    long *p2=p1+1;
    int len;
    if (*p1!=0x4242aeae)
    {
      qCDebug(GAMES_PRIVATE_KGAME) << ": Cookie error...transmission failure...serious problem...";
    }
    len=(int)(*p2);
    if (len<int(2*sizeof(long)))
    {
      qCDebug(GAMES_PRIVATE_KGAME) << ": Message size error";
      break;
    }
    if (len<=mReceiveCount)
    {
      qCDebug(GAMES_PRIVATE_KGAME) << ": Got message with len" << len;

      QByteArray msg ;
      msg.resize(len);
    //  msg.setRawData(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));

      qCopy(mReceiveBuffer.begin()+2*sizeof(long),mReceiveBuffer.begin()+len, msg.begin());
//       msg.duplicate(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
      emit received(msg);
     // msg.resetRawData(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
      // Shift buffer
      if (len<mReceiveCount)
      {
        memmove(mReceiveBuffer.data(),mReceiveBuffer.data()+len,mReceiveCount-len);
      }
      mReceiveCount-=len;
    }
    else break;
  }
}

void KMessageProcess::slotProcessExited(int exitCode, QProcess::ExitStatus)
{
  qCDebug(GAMES_PRIVATE_KGAME) << "Process exited (slot) with code" << exitCode;
  emit connectionBroken();
  delete mProcess;
  mProcess=0;
}

#include "kmessageio.moc"

