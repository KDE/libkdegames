/* **************************************************************************
                           KGameClient class
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
#ifndef __KGAMECLIENT_H_
#define __KGAMECLIENT_H_

#include <qstring.h>
#include <qobject.h>
#include <qlist.h>

#include <ksock.h>

class QFile;

class KProcess;

class KPlayer;

class KGameClientPrivate;

/*
 * This is the base class for queue clients. You cannot use it directly, but
 * have to inherit it (see @ref KGameClientSocket and @ref KGameClientProcess)
 * @short Base class for queue clients
 * @author Marin Heni <martin@heni-online.de> and Andreas Beckermann <b_mann@gmx.de>
 **/
/**
 * The KGameClient class is only internally used to
 * store information about network clients in on 
 * place. 
 * It could be made a subclass of KGame.
 *
 * You usually need two KGameClients: one on server(master)-side which is
 * started by @ref KGameCommunicationServer. And another one on client-side
 * which is started by @ref KGameNetwork. Some KGameClients have two
 * constructors for this, some (or only one: @ref KGameClientSocket) need only
 * one which can be used at both sides.
 *
 * This is the base class for all KGameClients. Use derived classes instead
 * (like KGameClientLocal for local players or KGameClientSocket for network
 * players) as this is of no direct use.
 * @short Base class of KGameClients internally used by @ref KGame
 * @author Marin Heni <martin@heni-online.de> and Andreas Beckermann <b_mann@gmx.de>
 */
class KGameClient : public QObject
{
	Q_OBJECT
public:
	/** 
	 * Create a KGameClient object
	 **/
	KGameClient(QObject* parent);
	~KGameClient();


	/**
	 * This just sends a message. It is ensured that the messages will arrive at
	 * the client in the correct order (i.e. first sent, first arrived) but _not_
	 * that it will be sent immediately! 
	 *
	 * The message is queued first and after a @ref QTimer::singleShot it is
	 * sent.
	 * @param buffer The message
	 **/
	void sendMessage(const QByteArray& buffer);


	/**
	 * Overloaded function
	 **/
	void sendMessage(const QDataStream& message);

  enum SocketStatus {Invalid,Client,Server,Wait,Listen};// *socket* status?

  QList<KPlayer> *playerList() { return &mPlayerList; } //? 

  int status() const { return mStatus; }
  void setStatus(int i);
  int id() const { return mId; }
  void setId(int i) { mId = i; }
	bool transmit() const { return mTransmit; }
	void setTransmit(bool b);

	/**
	 * @return IP or processname or whatever is useful for the client
	 **/
	const QString& IP() const { return mIP; }
	void setIP(const QString& s) { mIP = s; }

	/**
	 * @return port or userid or whatever is useful for the client
	 **/
	ushort port() const  { return mPort; }
	void setPort(ushort p) { mPort = p; }

	/**
	 * This is called when the connection is lost by any reason. It e.g. deletes
	 * the socket so that the connection really has been quit (and the memory
	 * freed) but the client is still alive. Then you can still read the IP and
	 * port the client was connected to.
	 *
	 * The default implementation just calls @ref setTransmit(false)
	 **/
	virtual void quitClient();


protected:
	bool deleteBuffer(QByteArray *buffer);
	void deleteDelayedBuffers();
	void delayDeleteBuffer(QByteArray *buffer);
	QByteArray *getBuffer();

	bool appendInput(const void *src,size_t len);
	bool extractHeader();
	bool hasMessage();
	bool deleteMessage();
	char *input() const { return mReadBuffer+mMsgOffset; }
	bool isReading() const { return mMsgLen > 0; }
	size_t messageSize() const { return mMsgLen-mMsgOffset; }

	/**
	 * @param must be a new'ed buffer!
	 **/
	void appendBuffer(QByteArray* buffer);


protected slots:
	/**
	 * Called indirectly using @ref QTimer::singleShot by @ref sendMessage.
	 * Must be implemented by the clients. This should send the buffers.
	 **/
	virtual void delayedSend() = 0;

signals:
	void signalReceiveMessage(const QByteArray&, KGameClient* );

private:
  int mStatus;  // Status of this client
    
	QString mIP;   // IP or name
	ushort mPort;  // port or other user id

  int mId;        // unique ID
	bool mTransmit; // transmission allowed?

  QList<KPlayer> mPlayerList;  // players of this client

	KGameClientPrivate* d;

	char* mReadBuffer;
	size_t mMsgLen; // Length of message to read
  size_t mMsgOffset; // Offset of the message after the start of the header
};



class KGameClientSocketPrivate;

/**
 * This is the KGameClient used for socket (network) communication. You just
 * call one of the constructors to connect to the server (or the other way
 * round). It is unimportant which constructor you use as one creates a KSocket
 * and the other one uses a KSocket created by you.
 * @short KGameClient for socket communication
 * @author Marin Heni <martin@heni-online.de> and Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameClientSocket : public KGameClient
{
	Q_OBJECT
public:
	KGameClientSocket(const char* host, unsigned short int port, QObject* parent);
	KGameClientSocket(KSocket* socket, QObject* parent);
	~KGameClientSocket();

	virtual void quitClient();


signals:
	void signalConnectionLost(KGameClient*); // in class KGameClient better?

protected slots:
	/**
	 * Calls @ref KSocket::enableWrite(TRUE) which will call @ref
	 * slotSocketWrite which will send all buffers.
	 **/
	virtual void delayedSend();

private slots:
	/**
	 * Sends the buffers to the socket.
	 **/
	void slotSocketWrite(KSocket*);

	/**
	 * Called when there is something to read. Reads the buffers.
	 **/
	void slotSocketRead(KSocket*);
	void slotSocketClosed(KSocket*);

private:
	void init(KSocket*);

	KGameClientSocketPrivate* d;
};


class KGameClientProcessPrivate;
/**
 * This KGameClient is used to communicate with a different process, as used by
 * @ref KGameClientProcess - the computer player. This class provides two
 * constructors, one used by @ref KGameCommunicationServer which will use a
 * @ref KProcess object to communicate with the process. The other one is used
 * by @ref KGameProcess inside the computer player.
 * @short KGameClient for computer (process) player communication
 * @author Marin Heni <martin@heni-online.de> and Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameClientProcess : public KGameClient
{
	Q_OBJECT
public:
	/**
	 * @param parent probably of no use for a process player. Leave it 0!
	 **/
	KGameClientProcess(QObject* parent = 0);
	KGameClientProcess(KProcess* proc, QObject* parent = 0);
	~KGameClientProcess();

	/**
	 * Reads characters from a QFile. QFile should be opened as stdin, like
	 * <pre>
	 * QFile rFile;
	 * rFile.open(IO_ReadOnly|IO_Raw, stdin);
	 * </pre>
	 * The characters are read until either the end of the file(stdin) has been
	 * arrived (ie no more data available) or the end of the message has
	 * been arrived. Call @ref readBuffer to get the message!
	 *
	 * You can create a loop which just checks for a change in stin and then
	 * calls readInput.
	 * @param rFile a QFile which should be opened as stdin
	 **/
	void readInput(QFile* rFile);

	void readInput(const void* buffer, size_t buflen);

	/**
	 * If the message has been arrived you can use this to read it. Note
	 * that you still have to delete it!
	 * @return Either the message or NULL if the message has not yet
	 * arrived.
	 **/
	QByteArray* readBuffer();

protected slots:
	/**
	 * Write all buffers to stdout
	 **/
	virtual void delayedSend();

private:
	KGameClientProcessPrivate* d;

};


/**
 * This is the KGameClient class for local players. All it does is forwarding the
 * messages it gets.
 * 
 * One constructor just takes a parent as paramter (used by @ref KGameNetwork),
 * the other one is used by @ref KGameCommunicationServer and takes a pointer to
 * a KGameClientLocal object which connects to the first created
 * KGameClientLocal
 * @short Client for local players
 * @author Marin Heni <martin@heni-online.de> and Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameClientLocal : public KGameClient
{
	Q_OBJECT
public:
	KGameClientLocal(QObject* parent);

	/**
	 * @internal:
	 * creates a KGameClientLocal object for use in @ref KGameCommunicationServer
	 * only
	 **/
	KGameClientLocal(KGameClientLocal*, QObject* parent);

public slots:
	void slotReceiveMessage(const QByteArray&, KGameClient*);

protected slots:
	virtual void delayedSend();

signals:
	/**
	 * @internal
	 * Used by @ref KGameCommunicationServer only
	 **/
	void signalSendMessage(const QByteArray&, KGameClient*);

private:
	void init();

};
#endif
