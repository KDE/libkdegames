/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Burkhard Lehner <Burkhard.Lehner@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KMESSAGESERVER_P_H__
#define __KMESSAGESERVER_P_H__

// Qt
#include <QTcpServer>

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
  explicit KMessageServerSocket (quint16 port, QObject *parent = nullptr);
  ~KMessageServerSocket ();

public Q_SLOTS:
  void slotNewConnection();

Q_SIGNALS:
  void newClientConnected (KMessageIO *client);
};



#endif
