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
#include <kdebug.h>
#include <k3process.h>
#include <QFile>
#include <QDataStream>
// ----------------------- KMessageIO -------------------------

KMessageIO::KMessageIO (QObject *parent)
  : QObject (parent), m_id (0)
{}

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
        kWarning(11001) << k_funcinfo << ": Received unexpected data, magic number wrong!" << endl;
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
    kWarning(11001) << k_funcinfo << ": Object is already connected!" << endl;
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
    kError(11001) << k_funcinfo << ": Not yet connected!" << endl;
}


// ----------------------- KMessageProcess ---------------------------

KMessageProcess::~KMessageProcess()
{
  kDebug(11001) << "@@@KMessageProcess::Delete process" << endl;
  if (mProcess)
  {
    mProcess->kill();
    delete mProcess;
    mProcess=0;
    // Remove not send buffers
//     mQueue.setAutoDelete(true);
     while (!mQueue.isEmpty())
         delete mQueue.dequeue();
    // Maybe todo: delete mSendBuffer
  }
}
KMessageProcess::KMessageProcess(QObject *parent, const QString& file) : KMessageIO(parent)
{
  // Start process
  kDebug(11001) << "@@@KMessageProcess::Start process" << endl;
  mProcessName=file;
  mProcess=new K3Process;
  int id=0;
  *mProcess << mProcessName << QString("%1").arg(id);
  kDebug(11001) << "@@@KMessageProcess::Init:Id= " << id << endl;
  kDebug(11001) << "@@@KMessgeProcess::Init:Processname: " << mProcessName << endl;
  connect(mProcess, SIGNAL(receivedStdout(K3Process *, char *, int )),
                        this, SLOT(slotReceivedStdout(K3Process *, char * , int )));
  connect(mProcess, SIGNAL(receivedStderr(K3Process *, char *, int )),
                        this, SLOT(slotReceivedStderr(K3Process *, char * , int )));
  connect(mProcess, SIGNAL(processExited(K3Process *)),
                        this, SLOT(slotProcessExited(K3Process *)));
  connect(mProcess, SIGNAL(wroteStdin(K3Process *)),
                        this, SLOT(slotWroteStdin(K3Process *)));
  mProcess->start(K3Process::NotifyOnExit,K3Process::All);
  mSendBuffer=0;
  mReceiveCount=0;
  mReceiveBuffer.resize(1024);
}
bool KMessageProcess::isConnected() const
{
  kDebug(11001) << "@@@KMessageProcess::Is conencted" << endl;
  if (!mProcess) return false;
  return mProcess->isRunning();
}
void KMessageProcess::send(const QByteArray &msg)
{
  kDebug(11001) << "@@@KMessageProcess:: SEND("<<msg.size()<<") to process" << endl;
  unsigned int size=msg.size()+2*sizeof(long);

  char *tmpbuffer=new char[size];
  long *p1=(long *)tmpbuffer;
  long *p2=p1+1;
  kDebug(11001)  << "p1="<<p1 << "p2="<< p2 << endl;
  memcpy(tmpbuffer+2*sizeof(long),msg.data(),msg.size());
  *p1=0x4242aeae;
  *p2=size;
  
  QByteArray* buffer = new QByteArray(tmpbuffer,size);
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
  //  kDebug(11001) << " @@@@@@ writeToProcess::SEND to process " << mSendBuffer->size() << " BYTE " << endl;
  //  char *p=mSendBuffer->data();
  //  for (int i=0;i<16;i++) printf("%02x ",(unsigned char)(*(p+i)));printf("\n");
  /// @todo avoids crash, but is it the good solution ???
  if (mProcess == 0) {
    kDebug(11001) << "@@@KMessageProcess:: cannot write to stdin, no process available" << endl;
    return;
  }
  mProcess->writeStdin(mSendBuffer->data(),mSendBuffer->size());

}
void KMessageProcess::slotWroteStdin(K3Process * )
{
  kDebug(11001) << k_funcinfo << endl;
  if (mSendBuffer)
  {
    delete mSendBuffer;
    mSendBuffer=0;
  }
  writeToProcess();
}

void KMessageProcess::slotReceivedStderr(K3Process * proc, char *buffer, int buflen)
{
  int pid=0;
  int len;
  char *p;
  char *pos;
  kDebug(11001)<<"@@@ KMessageProcess::slotReceivedStderr " << buflen << " bytes" << endl;

  if (!buffer || buflen==0) return ;
  if (proc) pid=proc->pid();


  pos=buffer;
  do
  {
    p=(char *)memchr(pos,'\n',buflen);
    if (!p) len=buflen;
    else len=p-pos;

    QByteArray a(pos, len);
    QString s(a);
    kDebug(11001) << "K3Process:" <<pid<<"("<<len<< "):" << s << endl;
    emit signalReceivedStderr(s);
    a.clear();
    if (p) pos=p+1;
    buflen-=len+1;
  }while(buflen>0);
}


void KMessageProcess::slotReceivedStdout(K3Process * , char *buffer, int buflen)
{
  kDebug(11001) << "$$$$$$ " << k_funcinfo << ": Received " << buflen << " bytes over inter process communication" << endl;

  // TODO Make a plausibility check on buflen to avoid memory overflow
  while (mReceiveCount+buflen>=mReceiveBuffer.size()) mReceiveBuffer.resize(mReceiveBuffer.size()+1024);
  memcpy(mReceiveBuffer.data()+mReceiveCount,buffer,buflen);
  mReceiveCount+=buflen;

  // Possbile message
  while (mReceiveCount>int(2*sizeof(long)))
  {
    long *p1=(long *)mReceiveBuffer.data();
    long *p2=p1+1;
    int len;
    if (*p1!=0x4242aeae)
    {
      kDebug(11001) << k_funcinfo << ": Cookie error...transmission failure...serious problem..." << endl;
//      for (int i=0;i<mReceiveCount;i++) fprintf(stderr,"%02x ",mReceiveBuffer[i]);fprintf(stderr,"\n");
    }
    len=(int)(*p2);
    if (len<int(2*sizeof(long)))
    {
      kDebug(11001) << k_funcinfo << ": Message size error" << endl;
      break;
    }
    if (len<=mReceiveCount)
    {
      kDebug(11001) << k_funcinfo << ": Got message with len " << len << endl;

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

void KMessageProcess::slotProcessExited(K3Process * /*p*/)
{
  kDebug(11001) << "Process exited (slot)" << endl;
  emit connectionBroken();
  delete mProcess;
  mProcess=0;
}


// ----------------------- KMessageFilePipe ---------------------------
KMessageFilePipe::KMessageFilePipe(QObject *parent,QFile *readfile,QFile *writefile) : KMessageIO(parent)
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
  mWriteFile->write(buffer);
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
   char ch;
   mReadFile->getChar(&ch);

   while (mReceiveCount>=mReceiveBuffer.size()) mReceiveBuffer.resize(mReceiveBuffer.size()+1024);
   mReceiveBuffer[mReceiveCount]=ch;
   mReceiveCount++;

   // Change for message 
   if (mReceiveCount>=int(2*sizeof(long)))
   {
     long *p1=(long *)mReceiveBuffer.data();
     long *p2=p1+1;
     int len;
     if (*p1!=0x4242aeae)
     {
       fprintf(stderr,"KMessageFilePipe::exec:: Cookie error...transmission failure...serious problem...\n");
       fflush(stderr);
//       for (int i=0;i<16;i++) fprintf(stderr,"%02x ",mReceiveBuffer[i]);fprintf(stderr,"\n");
     }
     len=(int)(*p2);
     if (len==mReceiveCount)
     {
       //fprintf(stderr,"KMessageFilePipe::exec:: Got Message with len %d\n",len);

       QByteArray msg;
       msg.resize(len);
       //msg.setRawData(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
       qCopy(mReceiveBuffer.begin()+2*sizeof(long),mReceiveBuffer.begin()+len, msg.begin());
// 	msg.duplicate(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
       emit received(msg);
       //msg.resetRawData(mReceiveBuffer.data()+2*sizeof(long),len-2*sizeof(long));
       mReceiveCount=0;
     }
   }
   

   return ;

  
}

#include "kmessageio.moc"
