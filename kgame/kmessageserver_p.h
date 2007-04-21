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

#ifndef __KMESSAGESERVER_P_H__
#define __KMESSAGESERVER_P_H__

#include <QtNetwork/QTcpServer>

class KMessageIO;

/**
  Internal class of KMessageServer. Creates a server socket and waits for
  connections.

  @short An internal class for KServerSocket
  @author Burkhard Lehner <Burkhard.Lehner@gmx.de>
*/
class KMessageServerSocket : public QTcpServer
{
  Q_OBJECT

public:
  KMessageServerSocket (quint16 port, QObject *parent = 0);
  ~KMessageServerSocket ();

public slots:
  void slotNewConnection();

Q_SIGNALS:
  void newClientConnected (KMessageIO *client);
};



#endif
