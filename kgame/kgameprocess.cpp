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
#include <qfile.h>
#include <assert.h>

#include "kgameprocess.h"
#include "kplayer.h"
#include "kgame.h"
#include "kgamemessage.h"

#define READ_BUFFER_SIZE  1024


// ----------------------- Process Child ---------------------------

KGameProcess::KGameProcess() : QObject(0,0)
{
  mTerminate=false;
  mSleep=100;
  //mClient=new KGameClientProcess(this);
}
KGameProcess::~KGameProcess() 
{
  /*
 if (mClient) {
	delete mClient;
 }
 */
}


bool KGameProcess::exec(int argc, char *argv[])
{
 // Get id and cookie, ... from command line
 processArgs(argc,argv);

 // Check whether a player is set. If not create one!
 QFile rFile;
 rFile.open(IO_ReadOnly|IO_Raw,stdin);

 do {
    kdDebug(11001) << " before usleep" << endl;
	// --- read input ---
	while(rFile.atEnd()) {
		usleep(mSleep);
	}
    kdDebug(11001) << " before readInput" << endl;
	//mClient->readInput(&rFile);
    kdDebug(11001) << " before process Input" << endl;
	processInput();

 } while(!mTerminate);
 rFile.close();
 return true;
}

bool KGameProcess::processInput()
{
  /*
 QByteArray* a = mClient->readBuffer();
 if (!a) {
	return true ; // Message not yet arrived
 }

 int xcookie = 0;
 int xversion = 0;
 int msgid = 0;
 int sender = 0;
 int receiver = 0;
 QString s;

 kdDebug(11001) << "Process *************** got input message size= "<< a->size() <<endl;
    //
    // DEBUG TODO
    fprintf(stderr,"processInput:");for (int i=0;i<a->size();i++) fprintf(stderr,"%02x ",(char)(*(a->data()+i)));fprintf(stderr,"\n");

 QDataStream rstream(*a,IO_ReadOnly);

 // Process message
 KGameMessage::extractHeader(rstream,xcookie,xversion,sender,receiver,msgid);

 kdDebug(11001) << "cookie="<<xcookie<< " version="<<xversion<< " sender="<<sender<< " recv="<<receiver<<endl;

 if (xversion!=KGameMessage::version()) {
	// Hmm nothing to do right now...maybe an error message to the sender?
	kdError(11001) << "Message Version " 
			<< xversion 
			<< "!=" 
			<< (int)KGameMessage::version() 
			<< " error" 
			<< endl;
 } else {
		kdDebug(11001) << "emmiting signalReceivedCommand id= " << msgid << endl;
		emit signalReceivedCommand(rstream,msgid,receiver,sender);
 }

 delete a;
 */
 return true;
}

//    You have to do this to create a message 
//    QByteArray buffer;
//    QDataStream wstream(buffer,IO_WriteOnly);
//    then stream data into the stream and call this function
void KGameProcess::sendMessage(QDataStream &stream,int msgid,int receiver)
{
  kdDebug(11001) << "KGameProcess::sendMessage" << endl;
  QByteArray a;
  QDataStream outstream(a,IO_WriteOnly);

  QBuffer *device=(QBuffer *)stream.device();
  QByteArray data=device->buffer();;

  // Sender can be 0 as it will be forced to be correct by the KGameIO device
  // Cookie and version apply only to the communication between us and the KGameIO
  // Receiver nesessary if used not as gameMessage, id id!=IdPlayerInput
  KGameMessage::createHeader(outstream,0,receiver,msgid);
  outstream.writeRawBytes(data.data(),data.size());

  //mClient->sendMessage(a);
}

void KGameProcess::processArgs(int argc, char *argv[])
{
  int v=0;
  if (argc>2)
  {
    v=atoi(argv[2]);
    kdDebug(11001) << "cookie (unused) " << v << endl;
  }
  if (argc>1)
  {
    v=atoi(argv[1]);
    kdDebug(11001) << "id (unused) " << v << endl;
  }
}

#include "kgameprocess.moc"
