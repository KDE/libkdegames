/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Burkhard Lehner <Burkhard.Lehner@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

/*
     KMessageIO class and subclasses KMessageSocket and KMessageDirect
*/

#ifndef _KMESSAGEIO_H_
#define _KMESSAGEIO_H_

// own
#include "kdegamesprivate_export.h"
// Qt
#include <QHostAddress>
#include <QObject>
#include <QProcess>
#include <QString>

class QTcpSocket;
class KProcess;

/**
  \class KMessageIO kmessageio.h <KGame/KMessageIO>

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
class KDEGAMESPRIVATE_EXPORT KMessageIO : public QObject
{
    Q_OBJECT

public:
    /**
     * The usual QObject constructor, does nothing else.
     */
    explicit KMessageIO(QObject *parent = nullptr);

    /**
     * The usual destructor, does nothing special.
     */
    ~KMessageIO() override;

    /**
     * The runtime identification
     */
    virtual int rtti() const
    {
        return 0;
    }

    /**
     * @return Whether this KMessageIO is a network IO or not.
     */
    // virtual bool isNetwork () const = 0;
    virtual bool isNetwork() const;

    /**
      This method returns the status of the object, whether it is already
      (or still) connected to another KMessageIO object or not.

      This is a pure virtual method, so it has to be implemented in a subclass
      of KMessageIO.
    */
    // virtual bool isConnected () const = 0;
    virtual bool isConnected() const;

    /**
      Sets the ID number of this object. This number can for example be used to
      distinguish several objects in a server.

      NOTE: Sometimes it is useful to let two connected KMessageIO objects
      have the same ID number. You have to do so yourself, KMessageIO doesn't
      change this value on its own!
    */
    void setId(quint32 id);

    /**
      Queries the ID of this object.
    */
    quint32 id();

    /**
      @return 0 in the default implementation. Reimplemented in @ref KMessageSocket.
    */
    virtual quint16 peerPort() const;

    /**
      @return "localhost" in the default implementation. Reimplemented in @ref KMessageSocket
    */
    virtual QString peerName() const;

Q_SIGNALS:
    /**
      This signal is emitted when /e send() on the connected KMessageIO
      object is called. The parameter contains the same data array in /e msg
      as was used in /e send().
    */
    void received(const QByteArray &msg);

    /**
      This signal is emitted when the connection is closed. This can be caused
      by a hardware error (e.g. broken internet connection) or because the other
      object was killed.

      Note: Sometimes a broken connection can be undetected for a long time,
      or may never be detected at all. So don't rely on this signal!
    */
    void connectionBroken();

public Q_SLOTS:

    /**
      This slot sends the data block in /e msg to the connected object, that will
      emit /e received().

      For a concrete class, you have to subclass /e KMessageIO and overwrite this
      method. In the subclass, implement this method as an ordinary method, not
      as a slot! (Otherwise another slot would be defined. It would work, but uses
      more memory and time.) See /e KMessageSocket for an example implementation.
    */
    virtual void send(const QByteArray &msg) = 0;

protected:
    quint32 m_id;
};

/**
  \class KMessageSocket kmessageio.h <KGame/KMessageIO>

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
    KMessageSocket(const QString &host, quint16 port, QObject *parent = nullptr);

    /**
      Connects to a server socket on /e host with /e port. You can immediately
      use the /e sendSystem() and /e sendBroadcast() methods. The messages are
      stored and sent to the receiver after the connection is established.

      If the connection could not be established (e.g. unknown host or no server
      socket at this port), the signal /e connectionBroken is emitted.
    */
    KMessageSocket(const QHostAddress &host, quint16 port, QObject *parent = nullptr);

    /**
      Uses /e socket to do the communication.

      The socket should already be connected, or at least be in /e connecting
      state.

      Note: The /e socket object is then owned by the /e KMessageSocket object.
      So don't use it otherwise any more and don't delete it. It is deleted
      together with this KMessageSocket object. (Use 0 as parent for the QSocket
      object t ensure it is not deleted.)
    */
    explicit KMessageSocket(QTcpSocket *socket, QObject *parent = nullptr);

    /**
      Uses the socket specified by the socket descriptor socketFD to do the
      communication. The socket must already be connected.

      This constructor can be used with a QServerSocket within the (pure
      virtual) method /e newConnection.

      Note: The socket is then owned by the /e KMessageSocket object. So don't
      manipulate the socket afterwards, especially don't close it. The socket is
      automatically closed when KMessageSocket is deleted.
    */
    explicit KMessageSocket(int socketFD, QObject *parent = nullptr);

    /**
      Destructor, closes the socket.
    */
    ~KMessageSocket() override;

    /**
     * The runtime identification
     */
    int rtti() const override
    {
        return 1;
    }

    /**
      @return The port that this object is connected to. See QSocket::peerPort
    */
    quint16 peerPort() const override;

    /**
      @return The hostname this object is connected to. See QSocket::peerName.
    */
    QString peerName() const override;

    /**
      @return TRUE as this is a network IO.
    */
    bool isNetwork() const override
    {
        return true;
    }

    /**
      Returns true if the socket is in state /e connected.
    */
    bool isConnected() const override;

    /**
      Overwritten slot method from KMessageIO.

      Note: It is not declared as a slot method, since the slot is already
      defined in KMessageIO as a virtual method.
    */
    void send(const QByteArray &msg) override;

protected Q_SLOTS:
    virtual void processNewData();

protected:
    void initSocket();
    QTcpSocket *mSocket;
    bool mAwaitingHeader;
    quint32 mNextBlockLength;

    bool isRecursive; // workaround for "bug" in QSocket, Qt 2.2.3 or older
};

/**
  \class KMessageDirect kmessageio.h <KGame/KMessageIO>

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
    explicit KMessageDirect(KMessageDirect *partner = nullptr, QObject *parent = nullptr);

    /**
      Destructor, closes the connection.
    */
    ~KMessageDirect() override;

    /**
     * The runtime identification
     */
    int rtti() const override
    {
        return 2;
    }

    /**
      @return FALSE as this is no network IO.
    */
    bool isNetwork() const override
    {
        return false;
    }

    /**
      Returns true, if the object is connected to another instance.

      If you use the first constructor, the object is unconnected unless another
      object is created with this one as parameter.

      The connection can only be closed by deleting one of the objects.
    */
    bool isConnected() const override;

    /**
      Overwritten slot method from KMessageIO.

      Note: It is not declared as a slot method, since the slot is already
      defined in KMessageIO as a virtual method.
    */
    void send(const QByteArray &msg) override;

protected:
    KMessageDirect *mPartner;
};

/**
 * \class KMessageProcess kmessageio.h <KGame/KMessageIO>
 */
class KMessageProcess : public KMessageIO
{
    Q_OBJECT

public:
    KMessageProcess(QObject *parent, const QString &file);
    ~KMessageProcess() override;
    bool isConnected() const override;
    void send(const QByteArray &msg) override;

    /**
      @return FALSE as this is no network IO.
    */
    bool isNetwork() const override
    {
        return false;
    }

    /**
     * The runtime identification
     */
    int rtti() const override
    {
        return 3;
    }

public Q_SLOTS:
    void slotReceivedStdout();
    void slotReceivedStderr();
    void slotProcessExited(int, QProcess::ExitStatus);

Q_SIGNALS:
    void signalReceivedStderr(const QString &msg);

private:
    QString mProcessName;
    KProcess *mProcess;
    QByteArray *mSendBuffer;
    QByteArray mReceiveBuffer;
    int mReceiveCount;
};

#endif
