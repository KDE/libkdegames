/*
     KMessageIO class and subclasses KMessageSocket and KMessageDirect

     Begin     : 13 April 2001
     Copyright : (c) 2001 by Burkhard Lehner
     eMail     : Burkhard.Lehner@gmx.de
*/

#include "kmessageio.h"
#include <qsocket.h>
#include <kdebug.h>

// ----------------------- KMessageIO -------------------------

KMessageIO::KMessageIO (QObject *parent, const char *name)
  : QObject (parent, name), m_id (0)
{}

KMessageIO::~KMessageIO ()
{}

void KMessageIO::setId (Q_UINT32 id)
{
  m_id = id;
}

Q_UINT32 KMessageIO::id ()
{
  return m_id;
}

// ----------------------KMessageSocket -----------------------

KMessageSocket::KMessageSocket (QString host, Q_UINT16 port, QObject *parent, 
const char *name)
  : KMessageIO (parent, name)
{
  mSocket = new QSocket ();
  mSocket->connectToHost (host, port);
  initSocket ();
}

KMessageSocket::KMessageSocket (QHostAddress host, Q_UINT16 port, QObject 
*parent, const char *name)
  : KMessageIO (parent, name)
{
  mSocket = new QSocket ();
  mSocket->connectToHost (host.toString(), port);
  initSocket ();
}

KMessageSocket::KMessageSocket (QSocket *socket, QObject *parent, const char 
*name)
  : KMessageIO (parent, name)
{
  mSocket = socket;
  initSocket ();
}

KMessageSocket::KMessageSocket (int socketFD, QObject *parent, const char 
*name)
  : KMessageIO (parent, name)
{
  mSocket = new QSocket ();
  mSocket->setSocket (socketFD);
  initSocket ();
}

KMessageSocket::~KMessageSocket ()
{
  delete mSocket;
}

bool KMessageSocket::isConnected ()
{
  return mSocket->state() == QSocket::Connection;
}

void KMessageSocket::send (const QByteArray &msg)
{
  QDataStream str (mSocket);
  str << Q_UINT8 ('M');  // magic number for begin of message
  str.writeBytes (msg.data(), msg.size());  // writes the length (as Q_UINT32) and the data
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
        return;

      // Read the magic number first. If something unexpected is found,
      // start over again, ignoring the data that was read up to then.

      Q_UINT8 v;
      str >> v;
      if (v != 'M')
      {
        kdWarning(11001) << "KMessageSocket::processNewData: Received unexpected data, magic number wrong!" << endl;
        continue;
      }

      str >> mNextBlockLength;
      mAwaitingHeader = false;
    }
    else
    {
      // Data not completly read => wait for more
      if (mSocket->bytesAvailable() < (int) mNextBlockLength)
        return;

      QByteArray msg (mNextBlockLength);
      str.readRawBytes (msg.data(), mNextBlockLength);

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
  connect (mSocket, SIGNAL (error(int)), SIGNAL (connectionBroken()));
  connect (mSocket, SIGNAL (connectionClosed()), SIGNAL (connectionBroken()));
  connect (mSocket, SIGNAL (readyRead()), SLOT (processNewData()));
  mAwaitingHeader = true;
  mNextBlockLength = 0;
  isRecursive = false;
}

// ----------------------KMessageDirect -----------------------

KMessageDirect::KMessageDirect (KMessageDirect *partner, QObject *parent, 
const char *name)
  : KMessageIO (parent, name), mPartner (0)
{
  // 0 as first parameter leaves the object unconnected
  if (!partner)
    return;

  // Check if the other object is already connected
  if (partner && partner->mPartner)
  {
    kdWarning(11001) << "KMessageDirect::KMessageDirect: Object is already connected!" << endl;
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

bool KMessageDirect::isConnected ()
{
  return mPartner != 0;
}

void KMessageDirect::send (const QByteArray &msg)
{
  if (mPartner)
    emit mPartner->received (msg);
  else
    kdError(11001) << "KMessageDirect::send: Not yet connected!" << endl;
}

#include "kmessageio.moc"

