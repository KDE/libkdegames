/* **************************************************************************
                           KGameErrorDialog Class
                           ----------------------
    begin                : 1 January 2001
    copyright            : (C) 2001 by Andreas Beckermann and Martin Heni
    email                : b_mann@gmx.de and martin@heni-online.de
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
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include "kgame.h"

#include "kgameerrordialog.h"

class KGameErrorDialogPrivate
{
public:
	KGameErrorDialogPrivate()
	{
		mGame = 0;
	}

	const KGame* mGame;
};

KGameErrorDialog::KGameErrorDialog(QWidget* parent) : QObject(parent)
{
 d = new KGameErrorDialogPrivate;
}

KGameErrorDialog::~KGameErrorDialog()
{
 delete d;
}

void KGameErrorDialog::setKGame(const KGame* g)
{
 slotUnsetKGame();
 d->mGame = g;
 connect(d->mGame, SIGNAL(signalNetworkErrorMessage(int, QString)), 
		this, SLOT(slotError(int, QString)));
 connect(d->mGame, SIGNAL(signalNetworkVersionError(Q_UINT32)), 
		this, SLOT(slotVersionError(Q_UINT32)));
 connect(d->mGame, SIGNAL(signalConnectionBroken()), 
		this, SLOT(slotServerConnectionLost()));
 connect(d->mGame, SIGNAL(signalConnectionLost(Q_UINT32)), 
		this, SLOT(slotClientConnectionLost(Q_UINT32)));
 connect(d->mGame, SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));
}

void KGameErrorDialog::slotUnsetKGame()
{
 if (d->mGame) {
	disconnect(d->mGame, 0, this, 0);
 }
 d->mGame = 0;
}

void KGameErrorDialog::error(const QString& errorText, QWidget* parent)
{ KMessageBox::error(parent, errorText); }

void KGameErrorDialog::slotVersionError(Q_UINT32 client)
{
 QString message;
 /*
//TODO: IP, versions/cookies of both sides, port, ...
 if (c) {
	QString IP = c->IP();
	//TODO:
//	QString version = 
//	QString cookie = 
//	QString localVersion = 
//	QString localCookie = 
	message = i18n("%1 caused a version error!!\n").arg(IP);
 } else {
	message = i18n("Version error!\n");
 }*/
 message = i18n("%1 caused a version error!!\n").arg(client);//FIXME: id is ugly.
 error(message, (QWidget*)parent());
}

void KGameErrorDialog::slotServerConnectionLost()
{
// TODO: add IP/port of the server
 QString message = i18n("Connection to the server has been lost!");
 error(message, (QWidget*)parent());
}

void KGameErrorDialog::slotClientConnectionLost(Q_UINT32 id)
{
//TODO: add IP/port of the client
 QString message;
// if (c) {
//	message = i18n("Connection to client has been lost!\nID: %1\nIP: %2").arg(c->id()).arg(c->IP());
// } else {
//	message = i18n("Connection to client has been lost!");
// }
 message = i18n("Connection to client has been lost!");
 error(message, (QWidget*)parent());
}

void KGameErrorDialog::slotError(int errorNo, QString text)
{
 QString message = i18n("Received a network error!\nError number: %1\nError message: %2").arg(errorNo).arg(text);
 error(message, (QWidget*)parent());
}

void KGameErrorDialog::connectionError(QString s)
{
 QString message;
 if (s.isNull()) {
	message = i18n("No connection could be created.");
 } else {
	message = i18n("No connection could be created.\nThe error message was:\n%1").arg(s);
 }
 error(message, (QWidget*)parent());
}



// should become the real dialog - currently we just use messageboxes 
// -> maybe unused forever
KGameErrorMessageDialog::KGameErrorMessageDialog(QWidget* parent) 
		: KDialogBase(Plain, i18n("Error"), Ok, Ok, parent, 0, true, true)
{
}

KGameErrorMessageDialog::~KGameErrorMessageDialog()
{
}



#include "kgameerrordialog.moc"
