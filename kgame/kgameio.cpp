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
#include <assert.h>

#include <qwidget.h>
#include <qbuffer.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <qfile.h>

#include <kdebug.h>

#include "kgameio.h"
#include "kgame.h"
#include "kplayer.h"
#include "kgamemessage.h"
#include "kmessageio.h"


// ----------------------- KGameProcesPrivate ---------------------------
class KGameProcessIOPrivate
{
public:
  KGameProcessIOPrivate()
  {
    //mMessageServer = 0;
    //mMessageClient = 0;
    mProcessIO=0;
  }
  //KMessageServer *mMessageServer;
  //KMessageClient *mMessageClient;
  KMessageProcess *mProcessIO;
};

// ----------------------- Process IO ---------------------------
KGameProcessIO::KGameProcessIO(const QString& name) 
   : KGameIO()
{
  kdDebug(11001) << "CREATE KGameProcessIO ("<<this<<") sizeof(this)="<<sizeof(KGameProcessIO)<<endl;
  d = new KGameProcessIOPrivate;

  //kdDebug(11001) << "================= KMEssageServer ==================== " << endl;
  //d->mMessageServer=new KMessageServer(0,this);
  //kdDebug(11001) << "================= KMEssageClient ==================== " << endl;
  //d->mMessageClient=new KMessageClient(this);
  kdDebug(11001) << "================= KMEssageProcessIO ==================== " << endl;
  d->mProcessIO=new KMessageProcess(this,name);
  kdDebug(11001) << "================= KMEssage Add client ==================== " << endl;
  //d->mMessageServer->addClient(d->mProcessIO);
  //kdDebug(11001) << "================= KMEssage SetSErver ==================== " << endl;
  //d->mMessageClient->setServer(d->mMessageServer);
  kdDebug(11001) << "================= KMEssage: Connect ==================== " << endl;
  //connect(d->mMessageClient, SIGNAL(broadcastReceived(const QByteArray&, Q_UINT32)),
  //        this, SLOT(clientMessage(const QByteArray&, Q_UINT32)));
  //connect(d->mMessageClient, SIGNAL(forwardReceived(const QByteArray&, Q_UINT32, const QValueList <Q_UINT32> &)),
  //        this, SLOT(clientMessage(const QByteArray&, Q_UINT32, const QValueList <Q_UINT32> &)));
  connect(d->mProcessIO, SIGNAL(received(const QByteArray&)),
          this, SLOT(receivedMessage(const QByteArray&)));
  //kdDebug(11001) << "Our client is id="<<d->mMessageClient->id() << endl;


}

int KGameProcessIO::rtti() const { return ProcessIO; }

KGameProcessIO::~KGameProcessIO()
{
  kdDebug(11001) << "DESTRUCT (KGameProcessIO="<<this<<")"<<endl;
  kdDebug(11001) << "player="<<player() << endl;
  if (player()) 
  {
    player()->removeGameIO(this,false); 
  }
  if (d->mProcessIO) 
  {
    delete d->mProcessIO;
    d->mProcessIO=0;
  }
  delete d;
}


void KGameProcessIO::initIO(KPlayer *p)
{
  KGameIO::initIO(p);
  // Send 'hello' to process
  QByteArray buffer;
  QDataStream stream(buffer,IO_WriteOnly);
  Q_INT16 id=p->userId();
  stream << id;

  bool sendit=true;
  if (p)
  {
    emit signalIOAdded(this,stream,p,sendit);
    if (sendit )
    {
      Q_UINT32 sender=p->id();  
      kdDebug(11001) <<  "Sending IOAdded to process player !!!!!!!!!!!!!! " << endl;
      sendSystemMessage(stream,KGameMessage::IdIOAdded,0,sender);
    }
  }
}

void KGameProcessIO::notifyTurn(bool b) 
{
  if (!player())
  {
    kdWarning(11001) << "KGameProcessIO::notifyTurn(): player() is NULL" << endl;
    return;
  }
  bool sendit=true;
  QByteArray buffer;
  QDataStream stream(buffer,IO_WriteOnly);
  stream << (Q_INT8)b;
  emit signalPrepareTurn(stream,b,this,&sendit);
  if (sendit)
  {
    Q_UINT32 sender=player()->id();  
    kdDebug(11001) <<  "Sending Turn to process player !!!!!!!!!!!!!! " << endl;
    sendSystemMessage(stream,KGameMessage::IdTurn,0,sender);
  }
}


void KGameProcessIO::sendSystemMessage(QDataStream &stream,int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{
  sendAllMessages(stream, msgid, receiver, sender, false);
}

void KGameProcessIO::sendMessage(QDataStream &stream,int msgid, Q_UINT32 receiver, Q_UINT32 sender)
{
  sendAllMessages(stream, msgid, receiver, sender, true);
}

void KGameProcessIO::sendAllMessages(QDataStream &stream,int msgid, Q_UINT32 receiver, Q_UINT32 sender, bool usermsg)
{
  kdDebug(11001) << "==============>  KGameProcessIO::sendMessage (usermsg="<<usermsg<<")" << endl;
  // if (!player()) return ;
  //if (!player()->isActive()) return ;

  if (usermsg) msgid+=KGameMessage::IdUser;

  kdDebug(11001) << "=============* ProcessIO (" << msgid << "," << receiver << "," << sender << ") ===========" << endl;

  QByteArray buffer;
  QDataStream ostream(buffer,IO_WriteOnly);
  QBuffer *device=(QBuffer *)stream.device();
  QByteArray data=device->buffer();;
  
  KGameMessage::createHeader(ostream,sender,receiver,msgid);
  // ostream.writeRawBytes(data.data()+device->at(),data.size()-device->at());
  ostream.writeRawBytes(data.data(),data.size());
  kdDebug(11001) << "   Adding user data from pos="<< device->at() <<" amount= " << data.size() << " byte " << endl;
  //if (d->mMessageClient) d->mMessageClient->sendBroadcast(buffer);
  if (d->mProcessIO) 
  {
    d->mProcessIO->send(buffer);
  }
}

//void KGameProcessIO::clientMessage(const QByteArray& receiveBuffer, Q_UINT32 clientID, const QValueList <Q_UINT32> &recv)
void KGameProcessIO::receivedMessage(const QByteArray& receiveBuffer)
{
  QDataStream stream(receiveBuffer,IO_ReadOnly);
  int msgid; 
  Q_UINT32 sender;
  Q_UINT32 receiver;
  KGameMessage::extractHeader(stream,sender,receiver,msgid);

  kdDebug(11001) << "************* Got process message sender =" << sender << " receiver=" << receiver << "   msgid="<< msgid <<endl;


  // Cut out the header part...to not confuse network code
  QBuffer *buf=(QBuffer *)stream.device();
  QByteArray newbuffer;
  newbuffer.setRawData(buf->buffer().data()+buf->at(),buf->size()-buf->at());
  QDataStream ostream(newbuffer,IO_ReadOnly);
  kdDebug(11001) << "Newbuffer size=" << newbuffer.size() << endl;



	// This is a dummy message which allows us the process to talk with its owner
	if (msgid==KGameMessage::IdProcessQuery)
  {
		emit signalProcessQuery(ostream,this);
	}
  else if (player())
  {
		sender=player()->id();  // force correct sender
    if (msgid==KGameMessage::IdPlayerInput) 
    {
      sendInput(ostream,true,sender);
    }
    else
    {
			player()->forwardMessage(ostream,msgid,receiver,sender);
		}
	}
  else 
  {
    kdDebug(11001) << "KGameProcessIO::receivedMessage: Got message from process but no player defined!" << endl;
  }
  newbuffer.resetRawData(buf->buffer().data()+buf->at(),buf->size()-buf->at());
}


// ----------------------- Computer IO --------------------------
KGameComputerIO::KGameComputerIO() 
   : KGameIO()
{
}

int KGameComputerIO::rtti() const { return ComputerIO; }

KGameComputerIO::~KGameComputerIO()
{
 if (player()) 
 {
//   player()->removeGameIO(this,false); //KGameIO
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
  if (parent())
  {
    ((QWidget*)parent())->setMouseTracking(b);
  }
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
     if (eatevent && sendInput(msg))
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
  if (!player()) 
  {
    return false;
  }


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
     
     if (eatevent && sendInput(msg)) 
     {
       return eatevent;
     }
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

KGameIO::KGameIO(KPlayer* player) : QObject(0,0)
{
  kdDebug(11001) << "CREATE(KGameIO=" << this <<") sizeof(this)"<<sizeof(KGameIO) << endl;
  mPlayer=0;
  if (player)
  {
    player->addGameIO(this);
  }
}

KGameIO::~KGameIO()
{
   kdDebug(11001) << "DESTRUCT(KGameIO=" << this <<")" << endl;
   // unregister ourselves
   if (player()) 
   {
     player()->removeGameIO(this,false); 
   }
}

void KGameIO::initIO(KPlayer *p)
{
  setPlayer(p);
}

void KGameIO::notifyTurn(bool b)
{
  if (!player())
  {
    kdWarning(11001) << "KGameIO::notifyTurn(): player() is NULL" << endl;
    return;
  }
  bool sendit=false;
  QByteArray buffer;
  QDataStream stream(buffer,IO_WriteOnly);
  emit signalPrepareTurn(stream, b, this, &sendit);
  if (sendit)
  {
    QDataStream ostream(buffer,IO_ReadOnly);
    Q_UINT32 sender = player()->id();  // force correct sender
    kdDebug(11001) << "Prepare turn sendInput" << endl;
    sendInput(ostream,true,sender);
  }
}

KGame* KGameIO::game() const
{
  if (!player()) 
  {
    return 0;
  }
  return player()->game();
}

bool KGameIO::sendInput(QDataStream& s, bool transmit, Q_UINT32 sender)
{
  if (!player()) 
  {
    return false;
  }
  return player()->forwardInput(s, transmit, sender);
}


void KGameIO::Debug()
{
   kdDebug(11001) << "------------------- KGAMEINPUT --------------------" << endl;
   kdDebug(11001) << "this:    " << this << endl;
   kdDebug(11001) << "rtti :   " << rtti() << endl;
   kdDebug(11001) << "Player:  " << player() << endl;
   kdDebug(11001) << "---------------------------------------------------" << endl;
}

#include "kgameio.moc"
