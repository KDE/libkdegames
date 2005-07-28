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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

/*
     KMessageIO class and subclasses KMessageSocket and KMessageDirect
*/

#include "kmessageio.h"
#include <q3socket.h>
#include <kdebug.h>
#include <kprocess.h>
#include <qfile.h>
#include <QDataStream>
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
  mSocket = new Q3Socket ();
  mSocket->connectToHost (host, port);
  initSocket ();
}

KMessageSocket::KMessageSocket (QHostAddress host, Q_UINT16 port, QObject 
*parent, const char *name)
  : KMessageIO (parent, name)
{
  mSocket = new Q3Socket ();
  mSocket->connectToHost (host.toString(), port);
  initSocket ();
}

KMessageSocket::KMessageSocket (Q3Socket *socket, QObject *parent, const char 
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
  mSocket = new Q3Socket ();
  mSocket->setSocket (socketFD);
  initSocket ();
}

KMessageSocket::~KMessageSocket ()
{
  delete mSocket;
}

bool KMessageSocket::isConnected () const
{
  return mSocket->state() == Q3Socket::Connection;
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
      {
        isRecursive = false;
        return;
      }

      // Read the magic number first. If something unexpected is found,
      // start over again, ignoring the data that was read up to then.

      Q_UINT8 v;
      str >> v;
      if (v != 'M')
      {
        kdWarning(11001) << k_funcinfo << ": Received unexpected data, magic number wrong!" << endl;
        continue;
      }

      str >> mNextBlockLength;
      mAwaitingHeader = false;
    }
    else
    {
      // Data not completely read => wait for more
      if (mSocket->bytesAvailable() < (Q_ULONG) mNextBlockLength)
      {
        isRecursive = false;
        return;
      }

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

Q_UINT16 KMessageSocket::peerPort () const
{
  return mSocket->peerPort();
}

QString KMessageSocket::peerName () const
{
  return mSocket->peerName();
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
    kdWarning(11001) << k_funcinfo << ": Object is already connected!" << endl;
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
    kdError(11001) << k_funcinfo << ": Not yet connected!" << endl;
}


// ----------------------- KMessageProcess ---------------------------

KMessageProcess::~KMessageProcess()
{
  kdDebug(11001) << "@@@KMessageProcess::Delete process" << endl;
  if (mProcess)
  {
    mProcess->kill();
    delete mProcess;
    mProcess=0;
    // Remove not send buffers
    mQueue.setAutoDelete(true);
    mQueue.clear();
    // Maybe todo: delete mSendBuffer
  }
}
KMessageProcess::KMessageProcess(QObject *parent, QString file) : KMessageIO(parent,0)
{
  // Start process
  kdDebug(11001) << "@@@KMessageProcess::Start process" << endl;
  mProcessName=file;
  mProcess=new KProcess;
  int id=0;
  *mProcess << mProcessName << QString("%1").arg(id);
  kdDebug(11001) << "@@@KMessageProcess::Init:Id= " << id << endl;
  kdDebug(11001) << "@@@KMessgeProcess::Init:Processname: " << mProcessName << endl;
  connect(mProcess, SIGNAL(receivedStdout(KProcess *, char *, int )),
                        this, SLOT(slotReceivedStdout(KProcess *, char * , int )));
  connect(mProcess, SIGNAL(receivedStderr(KProcess *, char *, int )),
                        this, SLOT(slotReceivedStderr(KProcess *, char * , int )));
  connect(mProcess, SIGNAL(processExited(KProcess *)),
                        this, SLOT(slotProcessExited(KProcess *)));
  connect(mProcess, SIGNAL(wroteStdin(KProcess *)),
                        this, SLOT(slotWroteStdin(KProcess *)));
  mProcess->start(KProcess::NotifyOnExit,KProcess::All);
  mSendBuffer=0;
  mReceiveCount=0;
  mReceiveBuffer.resize(1024);
}
bool KMessageProcess::isConnected() const
{
  kdDebug(11001) << "@@@KMessageProcess::Is conencted" << endl;
  if (!mProcess) return false;
  return mProcess->isRunning();
}
void KMessageProcess::send(const QByteArray &msg)
{
  kdDebug(11001) << "@@@KMessageProcess:: SEND("<<msg.size()<<") to process" << endl;
  unsigned int size=msg.size()+2*sizeof(long);

  char *tmpbuffer=new char[size];
  long *p1=(long *)tmpbuffer;
  long *p2=p1+1;
  kdDebug(11001)  << "p1="<<p1 << "p2="<< p2 << endl;
  memcpy(tmpbuffer+2*sizeof(long),msg.data(),msg.size());
  *p1=0x4242aeae;
  *p2=size;
  
  QByteArray *buffer=new QByteArray(tmpbuffer,size);
  delete [] tmpbuffer;
  // buffer->duplicate(msg);
  mQueue.enqueue(buffer);
  writeToProcess(); 
}
void KMessageProcess::writeToProcess()
{
  // Previous send ok and item in queue
  if (mSendBuffer || mQueue.isEmpty()) return ;
  mSendBuffer=mQueue.dequeue();
  if (!mSendBuffer) return ;

  // write it out to the process
  //  kdDebug(11001) << " @@@@@@ writeToProcess::SEND to process " << mSendBuffer->size() << " BYTE " << endl;
  //  char *p=mSendBuffer->data();
  //  for (int i=0;i<16;i++) printf("%02x ",(unsigned char)(*(p+i)));printf("\n");
  mProcess->writeStdin(mSendBuffer->data(),mSendBuffer->size());

}
void KMessageProcess::slotWroteStdin(KProcess * )
{
  kdDebug(11001) << k_funcinfo << endl;
  if (mSendBuffer)
  {
    delete mSendBuffer;
    mSendBuffer=0;
  }
  writeToProcess();
}

void KMessageProcess::slotReceivedStderr(KProcess * proc, char *buffer, int buflen)
{
  int pid=0;
  int len;
  char *p;
  char *pos;
//  kdDebug(11001)<<"############# Got stderr " << buflen << " bytes" << endl;

  if (!buffer || buflen==0) return ; 
  if (proc) pid=proc->pid();


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


void KMessageProcess::slotReceivedStdout(KProcess * , char *buffer, int buflen)
{
  kdDebug(11001) << "$$$$$$ " << k_funcinfo << ": Received " << buflen << " bytes over inter process communication" << endl;

  // TODO Make a plausibility check on buflen to avoid memory overflow
  while (mReceiveCount+buflen>=mReceiveBuffer.size()) mReceiveBuffer.resize(mReceiveBuffer.size()+1024);
  memcpy(mReceiveBuffer.data()+mReceiveCount,buffer,buflen);
  mReceiveCount+=buflen;

  // Possbile message
  while (mReceiveCount>2*sizeof(long))
  {
    long *p1=(long *)mReceiveBuffer.data();
    long *p2=p1+1;
    unsigned int len;
    if (*p1!=0x4242aeae)
    {
      kdDebug(11001) << k_funcinfo << ": Cookie error...transmission failure...serious problem..." << endl;
//      for (int i=0;i<mReceiveCount;i++) fprintf(stderr,"%02x ",mReceiveBuffer[i]);fprintf(stderr,"\n");
    }
    len=(int)(*p2);
    if (len<2*sizeof(long))
    {
      kdDebug(11001) << k_funcinfo << ": Message size error" << endl;
      break;
    }
    if (len<=mReceiveCount)
    {
      kdDebug(11001) << k_funcinfo << ": Got message with len " << len << endl;

      QByteArray msg;
    //  msg.setRawData(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
      msg.duplicate(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
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

void KMessageProcess::slotProcessExited(KProcess * /*p*/)
{
  kdDebug(11001) << "Process exited (slot)" << endl;
  emit connectionBroken();
  delete mProcess;
  mProcess=0;
}


// ----------------------- KMessageFilePipe ---------------------------
KMessageFilePipe::KMessageFilePipe(QObject *parent,QFile *readfile,QFile *writefile) : KMessageIO(parent,0)
{
  mReadFile=readfile;
  mWriteFile=writefile;
  mReceiveCount=0;
  mReceiveBuffer.resize(1024);
}

KMessageFilePipe::~KMessageFilePipe()
{
}

bool KMessageFilePipe::isConnected () const
{
  return (mReadFile!=0)&&(mWriteFile!=0);
}

void KMessageFilePipe::send(const QByteArray &msg)
{
  unsigned int size=msg.size()+2*sizeof(long);

  char *tmpbuffer=new char[size];
  long *p1=(long *)tmpbuffer;
  long *p2=p1+1;
  memcpy(tmpbuffer+2*sizeof(long),msg.data(),msg.size());
  *p1=0x4242aeae;
  *p2=size;
  
  QByteArray buffer(tmpbuffer,size);
  mWriteFile->writeBlock(buffer);
  mWriteFile->flush();
  delete [] tmpbuffer;
  /*
  fprintf(stderr,"+++ KMessageFilePipe:: SEND(%d to parent) realsize=%d\n",msg.size(),buffer.size());
  for (int i=0;i<buffer.size();i++) fprintf(stderr,"%02x ",buffer[i]);fprintf(stderr,"\n");
  fflush(stderr);
  */
}

void KMessageFilePipe::exec()
{

  // According to BL: Blocking read is ok
  // while(mReadFile->atEnd()) { usleep(100); }

   int ch=mReadFile->getch();

   while (mReceiveCount>=mReceiveBuffer.size()) mReceiveBuffer.resize(mReceiveBuffer.size()+1024);
   mReceiveBuffer[mReceiveCount]=(char)ch;
   mReceiveCount++;

   // Change for message 
   if (mReceiveCount>=2*sizeof(long))
   {
     long *p1=(long *)mReceiveBuffer.data();
     long *p2=p1+1;
     unsigned int len;
     if (*p1!=0x4242aeae)
     {
       fprintf(stderr,"KMessageFilePipe::exec:: Cookie error...transmission failure...serious problem...\n");
//       for (int i=0;i<16;i++) fprintf(stderr,"%02x ",mReceiveBuffer[i]);fprintf(stderr,"\n");
     }
     len=(int)(*p2);
     if (len==mReceiveCount)
     {
       //fprintf(stderr,"KMessageFilePipe::exec:: Got Message with len %d\n",len);

       QByteArray msg;
       //msg.setRawData(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
       msg.duplicate(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
       emit received(msg);
       //msg.resetRawData(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
       mReceiveCount=0;
     }
   }
   

   return ;

  
}

#include "kmessageio.moc"
