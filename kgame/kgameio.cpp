/* **************************************************************************
                           KGameIO class
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
#include <assert.h>

#include <qwidget.h>
#include <qbuffer.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <qfile.h>

#include <kdebug.h>
#include <ksock.h>
#include <kprocess.h>

#include "kgameio.h"
#include "kgame.h"
#include "kplayer.h"
#include "kgamemessage.h"

#define READ_BUFFER_SIZE   1024   // and automatically multiples




class KGameProcessIOPrivate
{
public:
  KGameProcessIOPrivate()
  {
    mProcess = 0;
    mCanWriteBuffer = 0;

    mCanWrite = false;
  }
  
  KProcess* mProcess;
  QString mProcessName;
  bool mCanWrite;
  char* mCanWriteBuffer;
};

// ----------------------- Process IO ---------------------------
KGameProcessIO::KGameProcessIO(const QString& name) 
   : KGameIO()
{
  kdDebug(11001) << "CREATE KGameProcessIO ("<<this<<") sizeof(this)="<<sizeof(KGameProcessIO)<<endl;
  d = new KGameProcessIOPrivate;

  d->mProcessName=name;
  mSleep = 100;

}

int KGameProcessIO::rtti() const { return ProcessIO; }

void KGameProcessIO::Exit()
{
  kdDebug(11001) << " KGameProcessIO::Exit" << endl;
  if (d->mProcess)
  {
    d->mProcess->kill();
    delete d->mProcess;
    d->mProcess=(KProcess *)0;
  }
}

bool KGameProcessIO::isRunning()
{
  if (!d->mProcess) return false;
  return d->mProcess->isRunning();
}

KGameProcessIO::~KGameProcessIO()
{
  kdDebug(11001) << "DESTRUCT (KGameProcessIO="<<this<<")"<<endl;
  kdDebug(11001) << "player="<<player() << endl;
  if (player()) player()->removeGameIO(this); 
  if (player() && player()->game()) player()->game()->unregisterListener(this);
  Exit();
  // unregister ourselves
  //delete readBuffer;
  delete d;
}

bool KGameProcessIO::Init()
{
  Exit();
  d->mProcess=new KProcess;
  int id=0;
  int cookie=0;
  if (player()) id=player()->id();
  if (player() && player()->game()) cookie=player()->game()->cookie();
  *d->mProcess << d->mProcessName << QString("%1").arg(id) << QString("%1").arg(cookie);
  kdDebug(11001) << "--------->>>KGameProcessIO::Init:Id= " << id << " Cookie="<<cookie << endl;
  kdDebug(11001) << "--------->>>KGameProcessIO::Init:Processname: " << d->mProcessName << endl;
  connect(d->mProcess, SIGNAL(receivedStdout(KProcess *, char *, int )),
                        this, SLOT(slotReceivedStdout(KProcess *, char * , int )));
  connect(d->mProcess, SIGNAL(receivedStderr(KProcess *, char *, int )),
                        this, SLOT(slotReceivedStderr(KProcess *, char * , int )));
  connect(d->mProcess, SIGNAL(processExited(KProcess *)),
                        this, SLOT(slotProcessExited(KProcess *)));
  connect(d->mProcess, SIGNAL(wroteStdin(KProcess *)),
                        this, SLOT(slotWroteStdin(KProcess *)));
  d->mCanWrite=true;

  bool result=d->mProcess->start(KProcess::NotifyOnExit,KProcess::All);
  return result;
}



void KGameProcessIO::slotWroteStdin(KProcess * )
{/*
//  kdDebug(11001)<<"!!!! KGameProcessIO::slotWroteStdin" << endl;
  d->mCanWrite=true;
  mClient->deleteDelayedBuffers();
  writeBuffers();*/
}

void KGameProcessIO::slotReceivedStderr(KProcess * proc, char *buffer, int buflen)
{
  int pid=0;
  int len;
  char *p;
  char *pos;
  if (!buffer || buflen==0) return ; 
  if (proc) pid=proc->pid();

  //kdDebug(11001)<<" Got stderr " << buflen << " bytes" << endl;

  pos=buffer;
  do
  {
    p=(char *)memchr(pos,'\n',buflen);
    if (!p) len=buflen;
    else len=p-pos;

    QByteArray a;
    a.setRawData(pos,len);
    QString s(a);
    kdDebug(11001) << "PID" <<pid<< ":" << s << endl;
    a.resetRawData(pos,len);
    if (p) pos=p+1;
    buflen-=len+1;
  }while(buflen>0);

}


void KGameProcessIO::slotReceivedStdout(KProcess * , char *buffer, int buflen)
{
 kdDebug(11001) << "KGameProcessIO::slotReceivedStdout::Received " << buflen << " bytes over inter process communication" << endl;

 /*
 for (QByteArray* a = mClient->readBuffer(); a; a = mClient->readBuffer()) {
	QDataStream stream(*a,IO_ReadOnly);
	int xcookie;
	int  xversion;
	int msgid; 
	int sender;
	int receiver;
	KGameMessage::extractHeader(stream,xcookie,xversion,sender,receiver,msgid);
	QBuffer *device=(QBuffer *)stream.device();

	kdDebug(11001) << "************* Got process message with cookie=" << xcookie << " version=" << xversion << endl;
	kdDebug(11001) << "   sender =" << sender << " receiver=" << receiver << endl;
	kdDebug(11001) << "   msgid="<< msgid <<endl;

	QByteArray b;
	b.setRawData(a->data() + device->at(), a->size() - device->at());
	QDataStream ostream(b,IO_ReadOnly);

	// This is a dummy message which allows us the process to talk with its owner
	if (msgid==KGameMessage::IdProcessQuery) {
		emit signalProcessQuery(stream,this);
	} else if (player()) {
		sender=KGameMessage::calcMessageId(0,player()->id());  // force correct sender
		if (msgid==KGameMessage::IdPlayerInput) {
			player()->forwardInput(ostream,true,sender);
		} else {
			player()->forwardMessage(ostream,msgid,receiver,sender);
		}
	}
	delete a;
 }
 */
}

void KGameProcessIO::slotProcessExited(KProcess * /*p*/)
{
  kdDebug(11001) << "Process exited (slot)" << endl;
  emit signalProcessExited(this);
}

void KGameProcessIO::receiveSystemMessage(QDataStream &stream,int msgid, int receiver, int sender)
{
  receiveAllMessage(stream, msgid, receiver, sender, false);
}

void KGameProcessIO::receiveMessage(QDataStream &stream,int msgid, int receiver, int sender)
{
  receiveAllMessage(stream, msgid, receiver, sender, true);
}

// unused... can be removed .... MH
void KGameProcessIO::receiveAllMessage(QDataStream &stream,int msgid, int receiver, int sender, bool usermsg)
{
  kdDebug(11001) << "==============>  KGameProcessIO::receiveMessage (usermsg="<<usermsg<<")" << endl;
  if (!player()) return ;

  if (!player()->isActive()) return ;

  if (usermsg) msgid+=KGameMessage::IdUser;

  kdDebug(11001) << "============= ProcessIO Network SNOOP    (" << msgid << "," << receiver << "," << sender << ") ===========" << endl;

  if (sender==KGameMessage::calcMessageId(0,player()->id()))
  {
    kdDebug(11001) << "  -> ignoring message from ourselves sender="<<sender << " player->id="<<player()->id() << endl;
    return ;
  }

  bool killit=false;
  emit signalProcessIOMessage((int)msgid,killit);
  if (killit)
  {
    kdDebug(11001) << "  -> ignoring message as requested by signal" << endl;
    return ;
  }
//  sendProcess(stream,msgid,receiver,sender,xcookie);//AB: sorry, broken :-( do
//  really all these functions need the cookie as a parameter ? NO
}

bool KGameProcessIO::sendProcess(const QDataStream &stream,int msgid,int receiver,int sender)
{
  if (!isRunning())
  {
    if (!Init()) 
    {
      return false;
    }
  }
  QByteArray buffer;
  QDataStream ostream(buffer,IO_WriteOnly);
  QBuffer *device=(QBuffer *)stream.device();
  QByteArray data=device->buffer();;
  
  //receiver=player()->id();
  KGameMessage::createHeader(ostream,sender,receiver,msgid);
  ostream.writeRawBytes(data.data()+device->at(),data.size()-device->at());
  /*
  mClient->sendMessage(buffer);
  */
  writeBuffers();
  return true;
}


bool KGameProcessIO::writeBuffers()
{//AB FIXME: KGameClient has changed! use KGameProcess - this function has to be ported!!

  int timeout;
  bool result=false;
  //kdDebug(11001) << "==============>  KGameProcessIO::writeBuffers:Process running=" << isRunning() << "canWRite="<<d->mCanWrite << endl;
  /*
  if (!isRunning())
  {
    if (!Init()) 
    {
      return false;
    }
  }
  if (!d->mProcess || !mClient || !mClient->transmit()) 
  {
    return false;
  }
  */
  /*//AB FIXME: must be ported
  kdDebug(11001) << "KGameProcessIO::writeBuffers:Process running= " << isRunning() << " pid=" << d->mProcess->pid() << endl;

  // Delay writing
  if (!d->mCanWrite) 
  {
    return false;
  }

  timeout=10;

  QByteArray *buffer;
  buffer=mClient->getBuffer();
  if (!buffer) kdDebug(11001)<< " No buffer found to send to process"<<endl;
  if (buffer && buffer->size()>0)
  {
    do
    {
      result=d->mProcess->writeStdin(buffer->data(),buffer->size());
      d->mCanWrite=false; // we have to wait until we are clear for writing again
      //kdDebug(11001) << "result="<<result << "timeout="<< timeout <<" running="<< d->mProcess->isRunning() <<endl;
      if (!result)
      {
        timeout--;
        usleep(mSleep); 
      }
    }while(!result && timeout>0);
  }
  else kdWarning(11001) << "   processWrite no buffer (data) found" << endl;
  // mClient->sendBufferList.clear();
  //if (buffer) mClient->sendBufferList.remove(buffer);
  if (buffer) mClient->delayDeleteBuffer(buffer);
  */
  return result;
}

void KGameProcessIO::notifyTurn(bool b) 
{
  bool sendit=true;
  QByteArray buffer;
  QDataStream stream(buffer,IO_WriteOnly);
  emit signalPrepareTurn(this,stream,b,sendit);
  int sender=KGameMessage::calcMessageId(0,player()->id());  
  kdDebug(11001) <<  "Sending Turn to process player !!!!!!!!!!!!!! " << endl;
  sendProcess(stream,KGameMessage::IdTurn,0,sender);
}


// ----------------------- Computer IO --------------------------
KGameComputerIO::KGameComputerIO() 
   : KGameIO()
{
}

int KGameComputerIO::rtti() const { return ComputerIO; }

KGameComputerIO::~KGameComputerIO()
{
   if (player()) player()->removeGameIO(this); 
}

void KGameComputerIO::notifyTurn(bool b) 
{
  bool sendit=true;
  QByteArray buffer;
  QDataStream stream(buffer,IO_WriteOnly);
  emit signalPrepareTurn(this,stream,b,sendit);
  if (sendit)
  {
    QDataStream ostream(buffer,IO_ReadOnly);
    int	sender=KGameMessage::calcMessageId(0,player()->id());  // force correct sender
    player()->forwardInput(ostream,true,sender);
  }
}


// ----------------------- Mouse IO ---------------------------
KGameMouseIO::KGameMouseIO(QWidget *parent,bool trackmouse) 
   : KGameIO()
{
  if (parent)
  {
    kdDebug(11001) << "Mouse Event filter installed tracking=" << trackmouse << endl;
    parent->installEventFilter(this);
    parent->setMouseTracking(trackmouse);
  }
}

int KGameMouseIO::rtti() const { return MouseIO; }

void KGameMouseIO::setMouseTracking(bool b)
{
  if (parent()) ((QWidget*)parent())->setMouseTracking(b);
}

bool KGameMouseIO::eventFilter( QObject *o, QEvent *e )
{
  if (!player()) return false;
//  kdDebug(11001) << "KGameMouseIO " << this  << endl ;
 
  // mouse action
  if ( e->type() == QEvent::MouseButtonPress ||
       e->type() == QEvent::MouseButtonRelease ||
       e->type() == QEvent::MouseButtonDblClick ||
       e->type() == QEvent::Wheel ||
       e->type() == QEvent::MouseMove 
       )
  { 
     QMouseEvent *k = (QMouseEvent*)e;
     // kdDebug(11001) << "KGameMouseIO " << this  << endl ;
     QByteArray buffer;
     QDataStream stream(buffer,IO_WriteOnly);
     bool eatevent=false;
     emit signalMouseEvent(this,stream,k,eatevent);
     QDataStream msg(buffer,IO_ReadOnly);
     if (eatevent && player()->forwardInput(msg))
     {
       return eatevent;
     }
     return false; // do not eat otherwise
  }
  return QObject::eventFilter( o, e );    // standard event processing
}

// ----------------------- Key IO ---------------------------
KGameKeyIO::KGameKeyIO(QWidget *parent) 
   : KGameIO()
{
  if (parent)
  {
    kdDebug(11001) << "Key Event filter installed" << endl;
    parent->installEventFilter(this);
  }
}

int KGameKeyIO::rtti() const { return KeyIO; }

bool KGameKeyIO::eventFilter( QObject *o, QEvent *e )
{
  if (!player()) return false;


  // key press/release
  if ( e->type() == QEvent::KeyPress ||
       e->type() == QEvent::KeyRelease )
  { 
     QKeyEvent *k = (QKeyEvent*)e;
  //   kdDebug(11001) << "KGameKeyIO " << this << " key press/release " <<  k->key() << endl ;
     QByteArray buffer;
     QDataStream stream(buffer,IO_WriteOnly);
     bool eatevent=false;
     emit signalKeyEvent(this,stream,k,eatevent);
     QDataStream msg(buffer,IO_ReadOnly);
     
     if (eatevent && player()->forwardInput(msg)) return eatevent;
     return false; // do not eat otherwise
  }
  return QObject::eventFilter( o, e );    // standard event processing
}


// ----------------------- Generic IO -------------------------

KGameIO::KGameIO() : QObject(0,0)
{
   kdDebug(11001) << "CREATE(KGameIO=" << this <<") sizeof(this)"<<sizeof(KGameIO) << endl;
   mPlayer=0;
}

KGameIO::~KGameIO()
{
   kdDebug(11001) << "DESTRUCT(KGameIO=" << this <<")" << endl;
   // unregister ourselves
   if (player()) player()->removeGameIO(this); 
}

void KGameIO::receiveSystemMessage(QDataStream &,int , int , int ) {}
void KGameIO::receiveMessage(QDataStream & /*stream*/,int, int, int) {}
void KGameIO::notifyTurn(bool ) {}


void KGameIO::Debug()
{
   kdDebug(11001) << "------------------- KGAMEINPUT --------------------" << endl;
   kdDebug(11001) << "this:    " << this << endl;
   kdDebug(11001) << "rtti :   " << rtti() << endl;
   kdDebug(11001) << "Player:  " << player() << endl;
   kdDebug(11001) << "---------------------------------------------------" << endl;
}

#include "kgameio.moc"
