/* **************************************************************************
                           KGameClient class
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

#include <stdio.h>
#include <assert.h>

#include <qbuffer.h>
#include <qfile.h>
#include <qqueue.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>

#include "kplayer.h"
#include "kgamemessage.h"
#include "kgameclient.h"

#include <sys/types.h>
//#include <sys/socket.h>
#include <unistd.h>

#define READ_BUFFER_SIZE  1024

class KGameClientPrivate 
{
public:
	KGameClientPrivate()
	{
	}

	QList<QByteArray> delayBufferList;
	QList<QByteArray> sendBufferList;
	size_t readBufferSize;
	size_t readBufferPos;
};

// ------------------- GAME CLIENT -------------------------
KGameClient::KGameClient(QObject* parent) : QObject(parent)
{
 kdDebug(11001) << "CREATE(KGameClient=" << this <<") sizeof(this)="<<sizeof(KGameClient) << endl;
 d = new KGameClientPrivate;
 mStatus = Invalid;
 mPort = 0;
 mId = 0;
 mTransmit = false;

 d->readBufferSize = READ_BUFFER_SIZE;
 d->readBufferPos = 0;
 d->sendBufferList.setAutoDelete(true);
 d->delayBufferList.setAutoDelete(true);
 mReadBuffer = new char[READ_BUFFER_SIZE];
 mMsgLen = 0;
 mMsgOffset = 0;
}

KGameClient::~KGameClient()
{
 kdDebug(11001) << "DESTRUCT(KGameClient=" << this <<")" << endl;
 mPlayerList.clear();
 delete mReadBuffer;
 d->sendBufferList.clear();
 delete d;
}

void KGameClient::sendMessage(const QDataStream& s)
{ sendMessage(((QBuffer*)s.device())->buffer()); }

void KGameClient::quitClient()
{
 setTransmit(false);
}

void KGameClient::setTransmit(bool t)
{ mTransmit = t;}
void KGameClient::setStatus(int i)
{ mStatus = i; }


bool KGameClient::deleteBuffer(QByteArray *buffer)
{
 if (buffer) {
	d->sendBufferList.remove(buffer);
 }
 return d->sendBufferList.count()>0;
}


void KGameClient::appendBuffer(QByteArray *buffer)
{ d->sendBufferList.append(buffer); }

void KGameClient::delayDeleteBuffer(QByteArray *buffer)
{
 int pos=d->sendBufferList.find(buffer);
 d->sendBufferList.take(pos); // remove from list without deleting
 d->delayBufferList.append(buffer);
 kdDebug(11001) << "Delay deleting of " << (int)buffer<<endl;
}

bool KGameClient::appendInput(const void *buffer,size_t buflen)
{
  while (d->readBufferPos + buflen >= d->readBufferSize)
  {
    char *m = new char[d->readBufferSize + READ_BUFFER_SIZE];
    d->readBufferSize += READ_BUFFER_SIZE; 
    memcpy(m, mReadBuffer, d->readBufferPos);
    delete mReadBuffer;
    mReadBuffer = m;
    kdDebug(11001) << " - Resizing network readbuffer to " << d->readBufferSize << endl;
  }
  memcpy(mReadBuffer + d->readBufferPos, buffer, buflen);
  d->readBufferPos += buflen;
  return true;
}

bool KGameClient::deleteMessage()
{
 if (!hasMessage()) { 
	return false;
 }
 if (d->readBufferPos <= (size_t)mMsgLen) {
	d->readBufferPos = 0;
 } else {
	memmove(mReadBuffer, mReadBuffer + mMsgLen, d->readBufferPos - mMsgLen);
	d->readBufferPos -= mMsgLen;
 }
 mMsgLen = 0;
 mMsgOffset = 0;
  
 return true;
}

bool KGameClient::hasMessage()
{
 if (!isReading()) {
	return false;
 }
 if (d->readBufferPos >= (size_t)mMsgLen) {
	return true;
 }
 return false;
}

QByteArray *KGameClient::getBuffer()
{
/* kdDebug(11001) << "KGameClient::getBuffer count=" 
		<< d->sendBufferList.count() 
		<< " first=" 
		<< (int)d->sendBufferList.first() 
		<< endl;*/
 return d->sendBufferList.first(); 
}

void KGameClient::deleteDelayedBuffers()
{
 d->delayBufferList.clear();
}

bool KGameClient::extractHeader()
{
//FIXME: this function uses quite a lot of arrays where some of them are
//preallocated. Check if a memory hole exists - especially after my hacks (AB)
  if (isReading()) 
  {
       return false;  // in read message mode
  }
  if (d->readBufferPos < KGameMessage::systemHeaderSize()) 
  {
       return false; // not enough data
  }

  size_t i, maxi;
  maxi = d->readBufferPos - KGameMessage::systemHeaderSize();
  // If the cookie is not found for i==0 there is something wrong...we do it anyways to be more flexible
  for (i = 0; i <= maxi; i++)
  {
      if (i != 0) 
      {
        //perhaps kdWarning()?
        kdWarning(11001) << "cookie not found for i==0! - continue searching..." << endl;
      }
      if (KGameMessage::extractMessageLength(mReadBuffer + i, mMsgLen))
      {
        size_t copyL = i + KGameMessage::systemHeaderSize();
        kdDebug(11001) << "KGameClient::extractHeader::Found message with len=" << mMsgLen << endl;
#ifdef KILL_HEADER
        if (d->readBufferPos > copyL)
        {
//          memmove(mReadBuffer, mReadBuffer + copyL, d->readBufferPos - copyL);
  //        d->readBufferPos -= copyL;
        }
        else d->readBufferPos = 0;
#endif
        mMsgOffset=i;  // remove garble? useful ??
        // mMsgOffset=copyL;
        return isReading();
      }
  }
  kdWarning(11001) << "Received garbled network data .. ignoring "<< d->readBufferPos <<" byte data... but not goood! " <<endl;
  d->readBufferPos = 0;
  return false;
}


void KGameClient::sendMessage(const QByteArray& buffer)
{
//TODO: create a virtual function canSend() which will be replaced by every
//client. E.g. the socket client will return true if it is connected to a
//socket, false if not. Can be used as return for sendMessage, if it is false
 QByteArray *newbuffer = new QByteArray;
 newbuffer->duplicate(buffer);
 char *data = newbuffer->data();
 KGameMessage::createSystemHeader(data, newbuffer->size());
 appendBuffer(newbuffer);

 kdDebug(11001) << "KGameClient::sendMessage!!!!!!!!!!" << endl;
 QTimer::singleShot(0, this, SLOT(delayedSend()));
}

class KGameClientSocketPrivate
{
public:
	KGameClientSocketPrivate()
	{
		mOwner = 0;
	}

	KSocket* mOwner;
};

KGameClientSocket::KGameClientSocket(const char* host, unsigned short int port, QObject* parent) : KGameClient(parent)
{
 KSocket* sock = new KSocket(host, port);
 //TODO: if (!sock || sock->socket() == -1) kdError(11001) << "...";
 init(sock);
 setPort(port);
 setIP(host);
}

KGameClientSocket::KGameClientSocket(KSocket* owner, QObject* parent) : KGameClient(parent)
{
 init(owner);
}

void KGameClientSocket::init(KSocket* owner)
{
 d = new KGameClientSocketPrivate;
 d->mOwner = owner;
 connect(d->mOwner, SIGNAL(readEvent(KSocket*)), this, SLOT(slotSocketRead(KSocket*)));
 connect(d->mOwner, SIGNAL(writeEvent(KSocket*)), this, SLOT(slotSocketWrite(KSocket*)));
 connect(d->mOwner, SIGNAL(closeEvent(KSocket*)), this, SLOT(slotSocketClosed(KSocket*)));
}

KGameClientSocket::~KGameClientSocket()
{
 kdDebug(11001) << "DESTRUCT(KGameClientSocket=" << this <<")" << endl;
 if (d->mOwner)
 {
   kdDebug(11001) << "  KGameClient destructor " << this << " kills socket " << d->mOwner << endl;
   delete d->mOwner;
   d->mOwner = 0;
 }
 delete d;
}

void KGameClientSocket::delayedSend()
{
 kdDebug(11001) << "KGameClientSocket::delayedSend" << endl;
 if (id() == 0) {
	kdError(11001) << "sending before initializing!!" << endl;
 }
 if(d->mOwner) {
	d->mOwner->enableWrite(TRUE);
 }
}

void KGameClientSocket::slotSocketClosed(KSocket *)
{
 kdDebug(11001) << "KGameClientSocket::slotSocketClosed ------- LOST CONNECTION" << endl;

 // Otherwise we seem to have problems when the signal centers the QT event
 // handler again
 quitClient();
 emit signalConnectionLost(this);

//TODO (in the slot connected to our signal):
//  unregisterClient(sock);
//  // become master if no more network conenctions exists
//  if (!connected()) setGameMaster(true);
}

void KGameClientSocket::slotSocketWrite(KSocket *sock)
{
 if (!sock ) {
	return;
 }
 sock->enableWrite(false);

 QByteArray *buffer;
 buffer = getBuffer();
 if (buffer && buffer->size() > 0) {
	// connected?
	if (sock->socket() != -1) {
		if (-1 == write(sock->socket(), buffer->data(), buffer->size())) {
			kdWarning(11001) << "Warning: Problems writing to socket." << endl;
		} // else {kdDebug(11001) << "KGameNetwork::slotSocketWrite:"<<buffer->size() << "bytes written" << endl;}
	}
 } else {
	kdWarning(11001) << "socketWrite no buffer (data) found" << endl;
 }
 if (deleteBuffer(buffer)) {
	sock->enableWrite(true);
 }
}

void KGameClientSocket::slotSocketRead(KSocket *sock)
{
 if (!sock || -1 == sock->socket()) {
	return;
 }
 ssize_t buflen;
 char * buffer = new char[READ_BUFFER_SIZE];
 buflen=read(sock->socket(),buffer,READ_BUFFER_SIZE);
 // kdDebug(11001) << buflen << " bytes read from socket" << endl;
 if (buflen <= 0) {
	return ;
 }

 appendInput(buffer,buflen);

 // Try to find Header
 while (isReading() || extractHeader()) {
	if (!hasMessage()) {
		return ; // Message not yet arrived
	}
    
	// stream it
	QByteArray a;
	a.setRawData(input(),messageSize()); // client->readBufferPos); // TODO
	// Main message loop
	{
		emit signalReceiveMessage(a, this);
	}
	a.resetRawData(input(), messageSize()); // client->readBufferPos); // TODO
	deleteMessage();
 }
}

void KGameClientSocket::quitClient()
{
 setTransmit(false);

 if (d->mOwner) {
	delete d->mOwner;
 }
 d->mOwner=0;
}



KGameClientLocal::KGameClientLocal(QObject* parent) : KGameClient(parent)
{
 init();
}

KGameClientLocal::KGameClientLocal(KGameClientLocal* localClient, QObject* parent) : KGameClient(parent)
{
 if (localClient) {
	connect(this, SIGNAL(signalSendMessage(const QByteArray&, KGameClient*)),
			localClient, SLOT(slotReceiveMessage(const QByteArray&,	KGameClient*)));
	connect(localClient, SIGNAL(signalSendMessage(const QByteArray&, KGameClient*)),
			this, SLOT(slotReceiveMessage(const QByteArray&, KGameClient*)));
 }
 init();
}

void KGameClientLocal::init()
{
 setIP("local");//umm - use something like QString::fromLocal8Bit??? - just a dummy entry!
 setPort(0); // dummy entry
}

void KGameClientLocal::delayedSend()
{
 kdDebug(11001) << "KGameClientLocal::delayedSend" << endl;
 if (id() == 0) {
	kdError(11001) << "sending before initializing!!" << endl;
 }
 QByteArray* a = 0;
 do {
	a = getBuffer();
	if (a && a->size() > 0) {
		emit signalSendMessage(*a, this); 
	}
 } while (deleteBuffer(a));
}

void KGameClientLocal::slotReceiveMessage(const QByteArray& a, KGameClient*)
{
 emit signalReceiveMessage(a, this);//FIXME client->this
}



class KGameClientProcessPrivate
{
public:
	KGameClientProcessPrivate()
	{
		mProc = 0;
	}

	QQueue<QByteArray> mReadBuffer; // the buffers that have been read from stdin
	KProcess* mProc;
};

KGameClientProcess::KGameClientProcess(KProcess* proc, QObject* parent) : KGameClient(parent)
{
 d = new KGameClientProcessPrivate;
 d->mProc = proc;
}

KGameClientProcess::KGameClientProcess(QObject* parent) : KGameClient(parent)
{
 d = new KGameClientProcessPrivate;
}

KGameClientProcess::~KGameClientProcess()
{
 kdDebug(11001) << "DESTRUCT(KGameClientProcess=" << this <<")" << endl;
 delete d;
}

void KGameClientProcess::delayedSend()
{
 kdDebug(11001) << "KGameClientProcess::delayedSend()" << endl;
 QByteArray* buffer;
 do {
	buffer = getBuffer();
	if (buffer && buffer->size() > 0) {

    // DEBUG TODO
    printf("writeStdin:");for (int i=0;i<buffer->size();i++) printf("%02x ",(char)(*(buffer->data()+i)));printf("\n");
    
		size_t sent=0;
    // WRITING TO stdout is WRONG..it is the KProcess pipe you have to write to
		// sent = fwrite(buffer->data(), buffer->size(), 1, stdout);
    if (d->mProc) sent=d->mProc->writeStdin(buffer->data(),buffer->size());
    else kdError(11001) << "KGameProcess::writeBuffers(): KProcess not started" << endl;
		if (sent < 1) {
			kdError(11001) << "KGameProcess::writeBuffers(): fwrite did not write data!" << endl;
		} else {
			kdDebug(11001) << "KGameProcess::writeBuffers(): " << buffer->size() << " (" << sent << ") bytes sent" << endl;
		}
	} else {
		kdWarning(11001) << "KGameProcess::writeBuffers(): no buffer(data) found" << endl;
	}
 } while(deleteBuffer(buffer));
}

void KGameClientProcess::readInput(QFile* rFile)
{
 while(!rFile->atEnd())
 {
	char inputbyte = (char)rFile->getch();
	appendInput(&inputbyte, 1);

	while((isReading() || extractHeader()) && hasMessage()) {
		QByteArray *a = new QByteArray;
		a->setRawData(input(), messageSize()); // client->readBufferPos); // TODO
		d->mReadBuffer.enqueue(a); // message is completed. Queue it
		deleteMessage();
    return ;
	}
 }
}

void KGameClientProcess::readInput(const void* buffer, size_t buflen)
{
 appendInput(buffer, buflen);
 while((isReading() || extractHeader()) && hasMessage()) {
	QByteArray *a = new QByteArray;
	a->setRawData(input(), messageSize()); // client->readBufferPos); // TODO
	d->mReadBuffer.enqueue(a); // message is completed. Queue it
	deleteMessage();
 }
}

QByteArray* KGameClientProcess::readBuffer()
{ 
//MUST BE DELETED BY THE USER!!
 return d->mReadBuffer.dequeue();
}

#include "kgameclient.moc"
