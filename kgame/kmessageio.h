/*
     KMessageIO class and subclasses KMessageSocket and KMessageDirect

     Begin     : 13 April 2001
     Copyright : (c) 2001 by Burkhard Lehner
     eMail     : Burkhard.Lehner@gmx.de
*/

#ifndef _KMESSAGEIO_H_
#define _KMESSAGEIO_H_

#include <qcstring.h>
#include <qhostaddress.h>
#include <qobject.h>
#include <qstring.h>

class QSocket;


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
    The usual QObject constructor, does nothing else.
  */
  KMessageIO (QObject *parent = 0, const char *name = 0);

  /**
    The usual destructor, does nothing special.
  */
  ~KMessageIO ();

  /**
    This method returns the status of the object, whether it is already
    (or still) connected to another KMessageIO object or not.

    This is a pure virtual method, so it has to be implemented in a subclass
    of KMessageIO.
  */
  virtual bool isConnected () = 0;

  /**
    Sets the ID number of this object. This number can for example be used to
    distinguish several objects in a server.

    NOTE: Sometimes it is useful to let two connected KMessageIO objects
    have the same ID number. You have to do so yourself, KMessageIO doesn't
    change this value on its own!
  */
  void setId (Q_UINT32 id);

  /**
    Queries the ID of this object.
  */
  Q_UINT32 id ();


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
  Q_UINT32 m_id;
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
  KMessageSocket (QString host, Q_UINT16 port, QObject *parent = 0,
                  const char *name = 0);

  /**
    Connects to a server socket on /e host with /e port. You can immediately
    use the /e sendSystem() and /e sendBroadcast() methods. The messages are
    stored and sent to the receiver after the connection is established.

    If the connection could not be established (e.g. unknown host or no server
    socket at this port), the signal /e connectionBroken is emitted.
  */
  KMessageSocket (QHostAddress host, Q_UINT16 port, QObject *parent = 0,
                  const char *name = 0);

  /**
    Uses /e socket to do the communication.

    The socket should already be connected, or at least be in /e connecting
    state.

    Note: The /e socket object is then owned by the /e KMessageSocket object.
    So don't use it otherwise any more and don't delete it. It is deleted
    together with this KMessageSocket object. (Use 0 as parent for the QSocket
    object t ensure it is not deleted.)
  */
  KMessageSocket (QSocket *socket, QObject *parent = 0, const char *name = 0);

  /**
    Uses the socket specified by the socket descriptor socketFD to do the
    communication. The socket must already be connected.

    This constructor can be used with a QServerSocket within the (pure
    virtual) method /e newConnection.

    Note: The socket is then owned by the /e KMessageSocket object. So don't
    manipulate the socket afterwards, especially don't close it. The socket is
    automatically closed when KMessageSocket is deleted.
  */
  KMessageSocket (int socketFD, QObject *parent = 0, const char *name = 0);

  /**
    Destructor, closes the socket.
  */
  ~KMessageSocket ();

  /**
    Returns true if the socket is in state /e connected.
  */
  bool isConnected ();

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
  QSocket *mSocket;
  bool mAwaitingHeader;
  Q_UINT32 mNextBlockLength;

  bool isRecursive;  // workarround for "bug" in QSocket, Qt 2.2.3 or older
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
  KMessageDirect (KMessageDirect *partner = 0, QObject *parent = 0, const char
*name = 0);

  /**
    Destructor, closes the connection.
  */
  ~KMessageDirect ();

  /**
    Returns true, if the object is connected to another instance.

    If you use the first constructor, the object is unconnected unless another
    object is created with this one as parameter.

    The connection can only be closed by deleting one of the objects.
  */
  bool isConnected ();

  /**
    Overwritten slot method from KMessageIO.

    Note: It is not declared as a slot method, since the slot is already
    defined in KMessageIO as a virtual method.
  */
  void send (const QByteArray &msg);

protected:
  KMessageDirect *mPartner;
};


#endif
