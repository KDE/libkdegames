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

#include "kggzpacket.h"

#include <kdebug.h>

#include <qabstractsocket.h>

KGGZPacket::KGGZPacket()
{
	m_socket = NULL;

	m_inputstream = new QDataStream(&m_input, QIODevice::ReadOnly);
	m_outputstream = new QDataStream(&m_output, QIODevice::WriteOnly);
}

KGGZPacket::~KGGZPacket()
{
	if(m_socket)
	{
		flush();
		delete m_socket;
	}
}

QDataStream *KGGZPacket::inputstream()
{
	return m_inputstream;
}

QDataStream *KGGZPacket::outputstream()
{
	return m_outputstream;
}

void KGGZPacket::flush()
{
	QByteArray packsize;
	QDataStream packsizestream(&packsize, QIODevice::WriteOnly);
	packsizestream << (qint16)(m_output.size() + 2);

	kDebug(11005) << "<kggzpacket> flush; packsize = " << m_output.size() << endl;

	m_socket->write(packsize.data(), packsize.size());
	m_socket->write(m_output.data(), m_output.size());
	m_output.truncate(0);
}

void KGGZPacket::slotNetwork(int fd)
{
	qint64 avail;
	qint64 len;
	QByteArray packsize;
	QDataStream packsizestream(&packsize, QIODevice::ReadOnly);
	qint16 size;

	if(!m_socket)
	{
		kDebug(11005) << "<kggzpacket> init socket device" << endl;
		m_socket = new QAbstractSocket(QAbstractSocket::TcpSocket, this);
		m_socket->setSocketDescriptor(fd);
	}

	if(m_input.size() == 0)
	{
		if(m_socket->bytesAvailable() < 2) return;
		packsize.resize(2);
		avail = m_socket->read(packsize.data(), 2);
		if(avail == -1)
		{
			// Error!
		}
		packsizestream >> size;
		m_size = (int)size - 2;
		m_input.resize(m_size);
		kDebug(11005) << "<kggzpacket> input init; packsize = 2 + " << m_size << endl;
	}

	len = m_socket->bytesAvailable();
	if(len > m_size - m_input.size()) len = m_size - m_input.size();
	avail = m_socket->read(m_input.data() + m_input.size(), len);
	if(avail == -1)
	{
		// Error!
	}
	kDebug(11005) << "<kggzpacket> input; read up to " << m_input.size() << endl;

	if(m_input.size() == (qint64)m_size)
	{
		kDebug(11005) << "<kggzpacket> input done for packet; fire signal!" << endl;
		emit signalPacket();
		m_input.truncate(0);
	}
}

#include "kggzpacket.moc"
