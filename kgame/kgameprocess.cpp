/* **************************************************************************
                           KGameProcess class
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
#include "kmessageclient.h"
#include "kmessageserver.h"

#define READ_BUFFER_SIZE  1024


// ----------------------- Process Child ---------------------------

KGameProcess::KGameProcess() : QObject(0,0)
{
  mTerminate=false;
  // Check whether a player is set. If not create one!
  rFile.open(IO_ReadOnly|IO_Raw,stdin);
  wFile.open(IO_WriteOnly|IO_Raw,stdout);
  mMessageIO=new KMessageFilePipe(this,&rFile,&wFile);
  //mMessageServer=new KMessageServer(0,this);
  mMessageClient=new KMessageClient(this);
  mMessageClient->setServer(mMessageIO);
  //mMessageClient->setServer(mMessageServer);
  //mMessageServer->addClient(mMessageIO);
  connect (mMessageClient, SIGNAL(broadcastReceived(const QByteArray&, Q_UINT32)),
          this, SLOT(receivedMessage(const QByteArray&, Q_UINT32)));
  fprintf(stderr,"KGameProcess::construtcor %p %p\n",&rFile,&wFile);
 
  mRandom = new KRandomSequence;
  mRandom->setSeed(0);
}
KGameProcess::~KGameProcess() 
{
  delete mRandom;
  delete mMessageClient;
  delete mMessageServer;
  // TODO: do we need to delete the KMessageIO ?
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
  if (mMessageClient) mMessageClient->sendForward(a,2);
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

void KGameProcess::receivedMessage(const QByteArray& receiveBuffer, Q_UINT32 clientID)
{
 QDataStream stream(receiveBuffer, IO_ReadOnly);
 int msgid;
 int sender; 
 int receiver; 
 KGameMessage::extractHeader(stream, sender, receiver, msgid);
 Q_UINT32 gameid = KGameMessage::calcGameId(receiver);
 fprintf(stderr,"------ receiveNetworkTransmission(): id=%d sender=%d,recv=%d\n",msgid,sender,receiver);
 emit signalCommand(stream,msgid,receiver,sender);
}

#include "kgameprocess.moc"
