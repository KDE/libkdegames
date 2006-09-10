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

#ifndef _KMESSAGEIO_H_
#define _KMESSAGEIO_H_

#include <q3cstring.h>
#include <qhostaddress.h>
#include <QObject>
#include <QString>
#include <q3ptrqueue.h>
#include <QFile>
#include <kdebug.h>

class Q3Socket;
class KProcess;
//class QFile;


/**
  This abstract base class represents one end of a message connections
  between two clients. Each client has one object of a subclass of
  KMessageIO. Calling /e send() on one object will emit the signal
  /e received() on the other object, and vice versa.

  For each type of connection (TCP/IP socket, COM port, direct connection
  within the same class) a subclass of KMessageIO must be defined that
  implements the pure virtual methods /e send() and /e isConnected(),
  and sends the signals. (See /e KMessageSocket for an example implementation.)

  Two subclasses are already included: /e KMessageSocket (connection using
  TCP/IP sockets) and /e KMessageDirect (connection using method calls, both
  sides must be within the same process).
*/

class KMessageIO : public QObject
{
  Q_OBJECT

public:
  /**
   * The usual QObject constructor, does nothing else.
   **/
  KMessageIO (QObject *parent = 0);

  /**
   * The usual destructor, does nothing special.
   **/
  ~KMessageIO ();

  /**
  * The runtime idendifcation
  */
  virtual int rtti() const {return 0;}

  /**
   * @return Whether this KMessageIO is a network IO or not.
   **/
  //virtual bool isNetwork () const = 0;
  virtual bool isNetwork () const
  {
   kError(11001) << "Calling PURE virtual isNetwork...BAD" << endl;
   return false;
  }

  /**
    This method returns the status of the object, whether it is already
    (or still) connected to another KMessageIO object or not.

    This is a pure virtual method, so it has to be implemented in a subclass
    of KMessageIO.
  */
  //virtual bool isConnected () const = 0;
  virtual bool isConnected () const
  {
   kError(11001) << "Calling PURE virtual isConencted...BAD" << endl;
   return false;
  }

  /**
    Sets the ID number of this object. This number can for example be used to
    distinguish several objects in a server.

    NOTE: Sometimes it is useful to let two connected KMessageIO objects
    have the same ID number. You have to do so yourself, KMessageIO doesn't
    change this value on its own!
  */
  void setId (quint32 id);

  /**
    Queries the ID of this object.
  */
  quint32 id ();

  /**
    @return 0 in the default implementation. Reimplemented in @ref KMessageSocket.
  */
  virtual quint16 peerPort () const { return 0; }

  /**
    @return "localhost" in the default implementation. Reimplemented in @ref KMessageSocket
  */
  virtual QString peerName () const { return QString::fromLatin1("localhost"); }


signals:
  /**
    This signal is emitted when /e send() on the connected KMessageIO
    object is called. The parameter contains the same data array in /e msg
    as was used in /e send().
  */
  void received (const QByteArray &msg);

  /**
    This signal is emitted when the connection is closed. This can be caused
    by a hardware error (e.g. broken internet connection) or because the other
    object was killed.

    Note: Sometimes a broken connection can be undetected for a long time,
    or may never be detected at all. So don't rely on this signal!
  */
  void connectionBroken ();

public slots:

  /**
    This slot sends the data block in /e msg to the connected object, that will
    emit /e received().

    For a concrete class, you have to subclass /e KMessageIO and overwrite this
    method. In the subclass, implement this method as an ordinary method, not
    as a slot! (Otherwise another slot would be defined. It would work, but uses
    more memory and time.) See /e KMessageSocket for an example implementation.
  */
  virtual void send (const QByteArray &msg) = 0;

protected:
  quint32 m_id;
};


/**
  This class implements the message communication using a TCP/IP socket. The
  object can connect to a server socket, or can use an already connected socket.
*/

class KMessageSocket : public KMessageIO
{
  Q_OBJECT

public:
  /**
    Connects to a server socket on /e host with /e port. host can be an
    numerical (e.g. "192.168.0.212") or symbolic (e.g. "wave.peter.org")
    IP address. You can immediately use the /e sendSystem() and
    /e sendBroadcast() methods. The messages are stored and sent to the
    receiver after the connection is established.

    If the connection could not be established (e.g. unknown host or no server
    socket at this port), the signal /e connectionBroken is emitted.
  */
  KMessageSocket (const QString& host, quint16 port, QObject *parent = 0 );

  /**
    Connects to a server socket on /e host with /e port. You can immediately
    use the /e sendSystem() and /e sendBroadcast() methods. The messages are
    stored and sent to the receiver after the connection is established.

    If the connection could not be established (e.g. unknown host or no server
    socket at this port), the signal /e connectionBroken is emitted.
  */
  KMessageSocket (QHostAddress host, quint16 port, QObject *parent = 0);

  /**
    Uses /e socket to do the communication.

    The socket should already be connected, or at least be in /e connecting
    state.

    Note: The /e socket object is then owned by the /e KMessageSocket object.
    So don't use it otherwise any more and don't delete it. It is deleted
    together with this KMessageSocket object. (Use 0 as parent for the QSocket
    object t ensure it is not deleted.)
  */
  KMessageSocket (Q3Socket *socket, QObject *parent = 0);

  /**
    Uses the socket specified by the socket descriptor socketFD to do the
    communication. The socket must already be connected.

    This constructor can be used with a QServerSocket within the (pure
    virtual) method /e newConnection.

    Note: The socket is then owned by the /e KMessageSocket object. So don't
    manipulate the socket afterwards, especially don't close it. The socket is
    automatically closed when KMessageSocket is deleted.
  */
  KMessageSocket (int socketFD, QObject *parent = 0);

  /**
    Destructor, closes the socket.
  */
  ~KMessageSocket ();

  /**
  * The runtime idendifcation
  */
  virtual int rtti() const {return 1;}

  /**
    @return The port that this object is connected to. See QSocket::peerPort
  */
  virtual quint16 peerPort () const;

  /**
    @return The hostname this object is connected to. See QSocket::peerName.
  */
  virtual QString peerName () const;

  /**
    @return TRUE as this is a network IO.
  */
  bool isNetwork() const { return true; }

  /**
    Returns true if the socket is in state /e connected.
  */
  bool isConnected () const;

  /**
    Overwritten slot method from KMessageIO.

    Note: It is not declared as a slot method, since the slot is already
    defined in KMessageIO as a virtual method.
  */
  void send (const QByteArray &msg);

protected slots:
  virtual void processNewData ();

protected:
  void initSocket ();
  Q3Socket *mSocket;
  bool mAwaitingHeader;
  quint32 mNextBlockLength;

  bool isRecursive;  // workaround for "bug" in QSocket, Qt 2.2.3 or older
};


/**
  This class implements the message communication using function calls
  directly. It can only be used when both sides of the message pipe are
  within the same process. The communication is very fast.

  To establish a communication, you have to create two instances of
  KMessageDirect, the first one with no parameters in the constructor,
  the second one with the first as parameter:

  /code
    KMessageDirect *peer1, *peer2;
    peer1 = new KMessageDirect ();       // unconnected
    peer2 = new KMessageDirect (peer1);  // connect with peer1
  /endcode

  The connection is only closed when one of the instances is deleted.
*/

class KMessageDirect : public KMessageIO
{
  Q_OBJECT

public:
  /**
    Creates an object and connects it to the object given in the first
    parameter. Use 0 as first parameter to create an unconnected object,
    that is later connected.

    If that object is already connected, the object remains unconnected.
  */
  KMessageDirect (KMessageDirect *partner = 0, QObject *parent = 0);

  /**
    Destructor, closes the connection.
  */
  ~KMessageDirect ();

  /**
  * The runtime idendifcation
  */
  virtual int rtti() const {return 2;}


  /**
    @return FALSE as this is no network IO.
  */
  bool isNetwork() const { return false; }

  /**
    Returns true, if the object is connected to another instance.

    If you use the first constructor, the object is unconnected unless another
    object is created with this one as parameter.

    The connection can only be closed by deleting one of the objects.
  */
  bool isConnected () const;

  /**
    Overwritten slot method from KMessageIO.

    Note: It is not declared as a slot method, since the slot is already
    defined in KMessageIO as a virtual method.
  */
  void send (const QByteArray &msg);

protected:
  KMessageDirect *mPartner;
};

class KMessageProcess : public KMessageIO
{
  Q_OBJECT 

  public:
    KMessageProcess(QObject *parent, const QString& file);
    ~KMessageProcess();
    bool isConnected() const;
    void send (const QByteArray &msg);
    void writeToProcess();

    /**
      @return FALSE as this is no network IO.
    */
    bool isNetwork() const { return false; }

  /**
  * The runtime idendifcation
  */
  virtual int rtti() const {return 3;}



  public slots:
  void  slotReceivedStdout(KProcess *proc, char *buffer, int buflen);
  void  slotReceivedStderr(KProcess *proc, char *buffer, int buflen);
  void  slotProcessExited(KProcess *p);
  void  slotWroteStdin(KProcess *p);

  private:
    QString mProcessName;
    KProcess *mProcess;
    Q3PtrQueue <QByteArray> mQueue;
    QByteArray *mSendBuffer;
    QByteArray mReceiveBuffer;
    unsigned int mReceiveCount;
};

class KMessageFilePipe : public KMessageIO
{
  Q_OBJECT 

  public:
    KMessageFilePipe(QObject *parent,QFile *readFile,QFile *writeFile);
    ~KMessageFilePipe();
    bool isConnected() const;
    void send (const QByteArray &msg);
    void exec();

    /**
      @return FALSE as this is no network IO.
    */
    bool isNetwork() const { return false; }

  /**
  * The runtime idendifcation
  */
  virtual int rtti() const {return 4;}



  private:
    QFile *mReadFile;
    QFile *mWriteFile;
    QByteArray mReceiveBuffer;
    unsigned int mReceiveCount;
};

#endif
