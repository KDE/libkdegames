/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (kde at heni-online.de)
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgameprocess.h"
#include "kplayer.h"
#include "kgame.h"
#include "kgamemessage.h"
#include "kmessageio.h"

#include <krandomsequence.h>

#include <qbuffer.h>
#include <QDataStream>
#include <QtCore/QFile>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define READ_BUFFER_SIZE  1024

class KGameProcessPrivate
{
public:
    QFile rFile;
    QFile wFile;
    KRandomSequence* mRandom;
};

// ----------------------- Process Child ---------------------------

KGameProcess::KGameProcess()
    : QObject(), d(new KGameProcessPrivate)
{
  mTerminate=false;
  // Check whether a player is set. If not create one!
  d->rFile.open(stdin, QIODevice::ReadOnly|QIODevice::Unbuffered);
  d->wFile.open(stdout, QIODevice::WriteOnly|QIODevice::Unbuffered);
  mMessageIO = new KMessageFilePipe(this, &d->rFile, &d->wFile);
//  mMessageClient=new KMessageClient(this);
//  mMessageClient->setServer(mMessageIO);
//  connect (mMessageClient, SIGNAL(broadcastReceived(QByteArray,quint32)),
//          this, SLOT(receivedMessage(QByteArray,quint32)));
  connect (mMessageIO, SIGNAL(received(QByteArray)),
          this, SLOT(receivedMessage(QByteArray)));
 
  d->mRandom = new KRandomSequence;
  d->mRandom->setSeed(0);
}
KGameProcess::~KGameProcess() 
{
  delete d->mRandom;
  //delete mMessageClient;
  //delete mMessageServer;
  fprintf(stderr,"KGameProcess::destructor\n");
  fflush(stderr);
  delete mMessageIO;
  d->rFile.close();
  d->wFile.close();
  delete d;
}


bool KGameProcess::exec(int argc, char *argv[])
{
  // Get id and cookie, ... from command line
  processArgs(argc,argv);
  do
  {
    mMessageIO->exec();
  }  while(!mTerminate);
  return true;
}

//    You have to do this to create a message 
//    QByteArray buffer;
//    QDataStream wstream(buffer,QIODevice::WriteOnly);
//    then stream data into the stream and call this function
void KGameProcess::sendSystemMessage(QDataStream &stream,int msgid,quint32 receiver)
{
  fprintf(stderr,"KGameProcess::sendSystemMessage to parent id=%d recv=%ld\n",msgid,(unsigned long)receiver);
  QByteArray a;
  QDataStream outstream(&a,QIODevice::WriteOnly);

  QBuffer *device=(QBuffer *)stream.device();
  QByteArray data=device->buffer();

  KGameMessage::createHeader(outstream,0,receiver,msgid);
  outstream.writeRawData(data.data(),data.size());

  //  if (mMessageClient) mMessageClient->sendForward(a,2);
  if (mMessageIO) mMessageIO->send(a);
  else fprintf(stderr,"KGameProcess::sendSystemMessage:: NO IO DEVICE ... WILL FAIL\n");
}

void KGameProcess::sendMessage(QDataStream &stream,int msgid,quint32 receiver)
{
  sendSystemMessage(stream,msgid+KGameMessage::IdUser,receiver);
}

void KGameProcess::processArgs(int argc, char *argv[])
{
  int v=0;
  if (argc>2)
  {
    v=atoi(argv[2]);
    //kDebug(11001) << "cookie (unused) " << v;
  }
  if (argc>1)
  {
    v=atoi(argv[1]);
    //kDebug(11001) << "id (unused) " << v;
  }
  fprintf(stderr,"KGameProcess::processArgs \n");
  fflush(stderr);
}

void KGameProcess::receivedMessage(const QByteArray& receiveBuffer)
{
 QDataStream stream(receiveBuffer);
 int msgid;
 quint32 sender; 
 quint32 receiver; 
 KGameMessage::extractHeader(stream, sender, receiver, msgid);
 fprintf(stderr,"--- KGameProcess::receivedMessage(): id=%d sender=%ld,recv=%ld\n",
         msgid,(unsigned long)sender,(unsigned long)receiver);
 switch(msgid)
 {
   case KGameMessage::IdTurn:
     qint8 b;
     stream >> b;
     emit signalTurn(stream,(bool)b);
   break;
   case KGameMessage::IdIOAdded:
     qint16 id;
     stream >> id;
     emit signalInit(stream,(int)id);
   break;
   default:
      emit signalCommand(stream,msgid-KGameMessage::IdUser,receiver,sender);
   break;
 }
}

KRandomSequence* KGameProcess::random()
{
  return d->mRandom;
}


#include "kgameprocess.moc"
