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

#ifndef KGGZRAW_H
#define KGGZRAW_H

#include <QtCore/QObject>

#include "kggznet_export.h"

/**
 * @mainpage
 *
 * The \b kggznet library is a KDE/Qt game developer's interface to
 * networking on the GGZ Gaming Zone. The library provides classes to
 * handle low-level protocols for communication between game clients
 * and servers.
 *
 * There are two classes contained within \b kggznet: The KGGZRaw class
 * and the KGGZPacket class. Both have their unique strengths, so using
 * one of them should be considered, but depending on the protocol
 * requirements other classes (QXmlSimpleReader, KMessage, QTextStream)
 * might be more appropriate.
 *
 */

class QAbstractSocket;
class QDataStream;

/**
 * @short Communication interface to raw (non-quantized) binary communication.
 *
 * The KGGZRaw class can be used to read and write binary data on a network
 * stream. To use it, make the socket's file descriptor known to KGGZRaw
 * through \ref setNetwork, and connect the \ref signalError signal to a
 * slot just in case an error happens. Afterwards, read and write data at
 * any time.
 *
 * KGGZRaw implements the 'easysock' binary protocol from the GGZ Gaming Zone
 * project. For alternative protocols, please have a look at the \b KGGZPacket
 * class, or at http://dev.ggzgamingzone.org/protocols/.
 *
 * From a developer point of view, this class should be used as a replacement
 * for QDataStream on TCP/IP sockets for blocking read and write operations.
 * It only covers the three basic data types supported by the easysock
 * library, namely integers, characters and strings. All const char* strings
 * are automatically converted to and from QStrings. Since the native format
 * of serialized QStrings on QDataStream differs, the Qt behaviour can be
 * restored with \ref setFormat. In the context of GGZ games, this should
 * however not be necessary.
 *
 * Note: Blocking operations might freeze the GUI, so either use KGGZRaw in
 * a separate thread only, or ensure that data can always be read (i.e. for
 * local Unix socket operations).
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class KGGZNET_EXPORT KGGZRaw : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Constructor.
		 *
		 * Creates a new KGGZRaw object. It will remain unusable
		 * unless \ref setNetwork is called.
		 */
		KGGZRaw();

		/**
		 * Destructor.
		 *
		 * Destroys a KGGZRaw object.
		 */
		~KGGZRaw();

		/**
		 * Data serialization format.
		 *
		 * The default format is \ref EasysockFormat.
		 */
		enum Format
		{
			QtFormat,
			EasysockFormat
		};

		/**
		 * Specifies the socket file descriptor to use.
		 *
		 * This function must be called exactly once before a KGGZRaw object
		 * can be used. Calling it more than once will result in an error.
		 * The socket must be a TCP/IP socket and already be opened.
		 * Usually, game clients will use KGGZRaw on the socket to the game
		 * server as returned by KGGZMod's \b Module object.
		 *
		 * @param fd File descriptor of the underlying TCP/IP socket
		 */
		void setNetwork(int fd);

		/**
		 * Changes the serialization format.
		 *
		 * The format of the datatypes on the wire can be influenced
		 * by calling this method. However, this should not be necessary
		 * in most cases, since the \ref EasysockFormat is used by default.
		 *
		 * @param format Format to use for data serialization
		 */
		void setFormat(Format format);

		KGGZRaw& operator<<(qint32 i);
		KGGZRaw& operator<<(qint8 i);
		KGGZRaw& operator<<(QString s);

		KGGZRaw& operator>>(qint32 &i);
		KGGZRaw& operator>>(qint8 &i);
		KGGZRaw& operator>>(QString &s);

	Q_SIGNALS:
		/**
		 * An error has occurred.
		 *
		 * The game should destroy the KGGZRaw object and disable all
		 * networking activities associated with it.
		 */
		void signalError();

	private:
		bool ensureBytes(int bytes);
		int peekedStringBytes();

		QAbstractSocket *m_socket;
		QDataStream *m_net;
		Format m_format;
};

#endif

