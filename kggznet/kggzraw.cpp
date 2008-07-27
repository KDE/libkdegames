/*
    This file is part of the kggznet library.
    Copyright (c) 2005 - 2007 Josef Spillner <josef@ggzgamingzone.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kggzraw.h"

#include <kdebug.h>

#include <qabstractsocket.h>
#include <qdatastream.h>

/*
KGGZRaw provides a replacement for QDataStream when being used on an arbitrary
file descriptor, such as within the kggzmod library or for those games
which use a non-quantized binary protocol (i.e. blocking or in a thread).

This class is necessary for two reasons:
1) to ensure the QAbstractSocket is opened in unbuffered mode, so we can
   read ancillary data from the underlying fd as needed at any point in time
2) to ensure that QDataStream will block until enough bytes are available
   for reading
*/

/*
FIXME: one open issue is that when the destructor of QAbstractSocket is
called, the application will segfault!
*/

/*
FIXME: See qt-copy/doc/html/datastreamformat.html for compatibility with
easysock and other implementations
*/

KGGZRaw::KGGZRaw()
{
	m_socket = NULL;
	m_net = NULL;
	m_format = EasysockFormat;
}

KGGZRaw::~KGGZRaw()
{
	kDebug(11005) << "[raw] *destructor* net";
	delete m_net;
	kDebug(11005) << "[raw] *destructor* socket";
	delete m_socket;
	kDebug(11005) << "[raw] *destructor* done";
}

void KGGZRaw::setFormat(Format format)
{
	m_format = format;
}

void KGGZRaw::setNetwork(int fd)
{
	// Ensure this method gets called only once
	if(m_socket)
	{
		kError(11005) << "[raw] setNetwork called more than once";
		emit signalError();
		return;
	}

	// Create a datastream on an unbuffered TCP socket
	m_socket = new QAbstractSocket(QAbstractSocket::TcpSocket, this);
	m_socket->setSocketDescriptor(fd, QAbstractSocket::ConnectedState, QIODevice::ReadWrite | QIODevice::Unbuffered);

	m_net = new QDataStream(m_socket);
}

bool KGGZRaw::ensureBytes(int bytes)
{
	// Ensure that kggzraw has been initialised properly
	if((!m_net) || (!m_socket))
	{
		kError(11005) << "[raw] setNetwork not called yet";
		emit signalError();
		return false;
	}

	// Shortcut for invalid dynamic sizes - signalError() was already sent
	if(bytes < 0) return false;

	// Shortcut for outgoing data (operator<<)
	if(bytes == 0) return true;

	// Ensure that this number of bytes is available
	int waitcycles = 0;
	while(m_socket->bytesAvailable() < bytes)
	{
		m_socket->waitForReadyRead(-1);
		kWarning(11005) << "[raw] bytesAvailable grows to:" << m_socket->bytesAvailable();

		waitcycles++;
		if(waitcycles > 100)
		{
			kError(11005) << "[raw] failed to receive" << bytes << "bytes";
			emit signalError();
			return false;
		}
	}

	return true;
}

KGGZRaw& KGGZRaw::operator>>(qint32 &i)
{
	kDebug(11005) << "[raw] bytesAvailable(i32):" << m_socket->bytesAvailable();

	if(!ensureBytes(4)) return *this;
	*m_net >> i;

	kDebug(11005) << "[raw] i32 is:" << i;

	return *this;
}

KGGZRaw& KGGZRaw::operator>>(qint8 &i)
{
	kDebug(11005) << "[raw] bytesAvailable(i8):" << m_socket->bytesAvailable();

	if(!ensureBytes(1)) return *this;
	*m_net >> i;

	kDebug(11005) << "[raw] i8 is:" << i;

	return *this;
}

KGGZRaw& KGGZRaw::operator>>(QString &s)
{
	char *tmp;

	kDebug(11005) << "[raw] bytesAvailable(qstring):" << m_socket->bytesAvailable();

	if(!ensureBytes(peekedStringBytes())) return *this;
	if(m_format == QtFormat)
	{
		*m_net >> s;
	}
	else
	{
		kDebug(11005) << "[raw] use easysock conversion";
		*m_net >> tmp;
		s = tmp;
		delete[] tmp;
	}

	kDebug(11005) << "[raw] qstring is:" << s;

	return *this;
}

KGGZRaw& KGGZRaw::operator<<(qint32 i)
{
	kDebug(11005) << "[raw] out(i32):" << i;

	if(!ensureBytes(0)) return *this;
	*m_net << i;

	return *this;
}

KGGZRaw& KGGZRaw::operator<<(qint8 i)
{
	kDebug(11005) << "[raw] out(i8):" << i;

	if(!ensureBytes(0)) return *this;
	*m_net << i;

	return *this;
}

KGGZRaw& KGGZRaw::operator<<(QString s)
{
	kDebug(11005) << "[raw] out(qstring):" << s;

	if(!ensureBytes(0)) return *this;
	if(m_format == QtFormat)
	{
		*m_net << s;
	}
	else
	{
		kDebug(11005) << "[raw] use easysock conversion";
		*m_net << s.toUtf8().constData();
	}

	return *this;
}

int KGGZRaw::peekedStringBytes()
{
	int strsize;

	if(!ensureBytes(4)) return -1;

	QByteArray strsizear = m_socket->peek(4);
	QDataStream strsizestream(strsizear);
	strsizestream >> strsize;
	kDebug(11005) << "[raw] string length is" << strsize;

	if(m_format == QtFormat)
	{
		// an empty QString()
		if(strsize == -1) strsize = 0;
	}

	return strsize + 4;
}

#include "kggzraw.moc"
