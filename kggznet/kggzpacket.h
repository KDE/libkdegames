/*
    This file is part of the kggznet library.
    Copyright (c) 2006 - 2007 Josef Spillner <josef@ggzgamingzone.org>

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

#ifndef KGGZPACKET_H
#define KGGZPACKET_H

#include <QtCore/QObject>
#include <QtCore/QDataStream>

#include "kggznet_export.h"

class QAbstractSocket;

/**
 * @short Communication interface to packet (quantized) binary communication.
 *
 * The KGGZPacket class can be used to read and write binary data on a network
 * stream. To use it, connect any signal which reports file descriptor activity
 * to \ref slotNetwork, or call this method manually with the file descriptor
 * as its argument every time traffic arrives on the corresponding socket.
 * Next, connect the \ref signalPacket signal to a slot
 * in your application. Finally, start writing data to the \ref outputstream
 * (calling \ref flush whenever a packet is done), and start reading data
 * whenever the \ref signalPacket signal is sent from the \ref inputstream.
 *
 * KGGZPacket implements the 'dio' binary protocol from the GGZ Gaming Zone
 * project. For alternative protocols, please have a look at the \b KGGZRaw
 * class, or at http://dev.ggzgamingzone.org/protocols/.
 *
 * Note: Using KGGZPacket implies nonblocking operations, so it is safe to
 * use this class in a single-threaded GUI application. The drawback for
 * this convenience (as opposed to \b KGGZRaw) is that slightly more traffic
 * is caused on the network.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class KGGZNET_EXPORT KGGZPacket : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Constructor.
		 *
		 * Creates a new KGGZPacket object. It will remain unusable
		 * unless \ref slotNetwork is called.
		 */
		KGGZPacket();

		/**
		 * Destructor.
		 *
		 * Destroys a KGGZPacket object. All unflushed data is
		 * flushed before the object is destroyed.
		 */
		~KGGZPacket();

		/**
		 * The stream for reading data from.
		 *
		 * Data can safely be read from this stream whenever \ref
		 * signalPacket was emitted. No reading should be done in
		 * other situations.
		 *
		 * @return Input stream to read data from
		 */
		QDataStream *inputstream();

		/**
		 * The stream to write data to.
		 *
		 * Data can be written at any time. A new packet is created
		 * automatically and must always be finished with \ref flush.
		 *
		 * @return Output stream to write data to
		 */
		QDataStream *outputstream();

		/**
		 * Flushes pending output data.
		 *
		 * This method finishes up a pending packet and sends it
		 * out over the active connection.
		 */
		void flush();

	public Q_SLOTS:
		/**
		 * Network activity monitor.
		 *
		 * This method must be called whenever network activity is
		 * happening on the specified socket file descriptor.
		 * In many cases, \ref signalPacket will then be emitted.
		 *
		 * @param fd File descriptor for the monitored socket
		 */
		void slotNetwork(int fd);

	Q_SIGNALS:
		/**
		 * A new packet has arrived.
		 *
		 * This signal gets emitted whenever \ref slotNetwork is
		 * called and a whole packet can be read through it.
		 */
 		void signalPacket();

		/**
		 * An error has occurred.
		 *
		 * The game should destroy the KGGZPacket object and disable
		 * all networking activities associated with it.
		 */
		void signalError();

	private Q_SLOTS:
		void slotSocketError();

	private:
		void errorhandler();
		void readchunk();

		QDataStream *m_inputstream, *m_outputstream;
		QAbstractSocket *m_socket;
		QByteArray m_input, m_output;
		int m_size;
};

#endif

