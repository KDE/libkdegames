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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <qbuffer.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <assert.h>

#include <krandomsequence.h>
#include "kgameprocess.h"
#include "kplayer.h"
#include "kgame.h"
#include "kgamemessage.h"
#include "kmessageio.h"
//#include "kmessageclient.h"
//#include "kmessageserver.h"

#define READ_BUFFER_SIZE  1024


// ----------------------- Process Child ---------------------------

KGameProcess::KGameProcess() : QObject(0,0)
{
  mTerminate=false;
  // Check whether a player is set. If not create one!
  rFile.open(IO_ReadOnly|IO_Raw,stdin);
  wFile.open(IO_WriteOnly|IO_Raw,stdout);
  mMessageIO=new KMessageFilePipe(this,&rFile,&wFile);
//  mMessageClient=new KMessageClient(this);
//  mMessageClient->setServer(mMessageIO);
//  connect (mMessageClient, SIGNAL(broadcastReceived(const QByteArray&, Q_UINT32)),
//          this, SLOT(receivedMessage(const QByteArray&, Q_UINT32)));
  connect (mMessageIO, SIGNAL(received(const QByteArray&)),
          this, SLOT(receivedMessage(const QByteArray&)));
  fprintf(stderr,"KGameProcess::construtcor %p %p\n",&rFile,&wFile);
 
  mRandom = new KRandomSequence;
  mRandom->setSeed(0);
}
KGameProcess::~KGameProcess() 
{
  delete mRandom;
  //delete mMessageClient;
  //delete mMessageServer;
  delete mMessageIO;
  rFile.close();
  wFile.close();
  fprintf(stderr,"KGameProcess::destructor\n");
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
//    QDataStream wstream(buffer,IO_WriteOnly);
//    then stream data into the stream and call this function
void KGameProcess::sendSystemMessage(QDataStream &stream,int msgid,int receiver)
{
  fprintf(stderr,"KGameProcess::sendMessage id=%d recv=%d",msgid,receiver);
  QByteArray a;
  QDataStream outstream(a,IO_WriteOnly);

  QBuffer *device=(QBuffer *)stream.device();
  QByteArray data=device->buffer();;

  KGameMessage::createHeader(outstream,0,receiver,msgid);
  outstream.writeRawBytes(data.data(),data.size());

  //if (mMessageClient) mMessageClient->sendBroadcast(a);
  // TODO: The fixed received 2 will cause problems. But how to address the
  // proper one?
//  if (mMessageClient) mMessageClient->sendForward(a,2);
  if (mMessageIO) mMessageIO->send(a);
}

void KGameProcess::sendMessage(QDataStream &stream,int msgid,int receiver)
{
  sendSystemMessage(stream,msgid+KGameMessage::IdUser,receiver);
}

void KGameProcess::processArgs(int argc, char *argv[])
{
  int v=0;
  if (argc>2)
  {
    v=atoi(argv[2]);
    //kdDebug() << "cookie (unused) " << v << endl;
  }
  if (argc>1)
  {
    v=atoi(argv[1]);
    //kdDebug() << "id (unused) " << v << endl;
  }
  fprintf(stderr,"processArgs \n");
  fflush(stderr);
}

void KGameProcess::receivedMessage(const QByteArray& receiveBuffer)
{
 QDataStream stream(receiveBuffer, IO_ReadOnly);
 int msgid;
 int sender; 
 int receiver; 
 KGameMessage::extractHeader(stream, sender, receiver, msgid);
 fprintf(stderr,"------ receiveNetworkTransmission(): id=%d sender=%d,recv=%d\n",msgid,sender,receiver);
 switch(msgid)
 {
   case KGameMessage::IdTurn:
     Q_INT8 b;
     stream >> b;
     emit signalTurn(stream,(bool)b);
   break;
   case KGameMessage::IdIOAdded:
     Q_INT16 id;
     stream >> id;
     emit signalInit(stream,(int)id);
   break;
   default:
      emit signalCommand(stream,msgid,receiver,sender);
   break;
 }
}

#include "kgameprocess.moc"
