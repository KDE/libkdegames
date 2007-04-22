/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (kde at heni-online.de)

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

#include "kgameerrordialog.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include "kgame.h"

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

 connect(d->mGame, SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));

// the error signals:
 connect(d->mGame, SIGNAL(signalNetworkErrorMessage(int, QString)), 
		this, SLOT(slotError(int, QString)));
 connect(d->mGame, SIGNAL(signalConnectionBroken()), 
		this, SLOT(slotServerConnectionLost()));
 connect(d->mGame, SIGNAL(signalClientDisconnected(quint32,bool)), 
		this, SLOT(slotClientConnectionLost(quint32,bool)));
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

void KGameErrorDialog::slotServerConnectionLost()
{
// TODO: add IP/port of the server
 QString message = i18n("Connection to the server has been lost!");
 error(message, (QWidget*)parent());
}

void KGameErrorDialog::slotClientConnectionLost(quint32 /*id*/,bool)
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

void KGameErrorDialog::slotError(int errorNo, const QString& text)
{
 QString message = i18n("Received a network error!\nError number: %1\nError message: %2", errorNo, text);
 error(message, (QWidget*)parent());
}

void KGameErrorDialog::connectionError(const QString& s)
{
 QString message;
 if (s.isNull()) {
	message = i18n("No connection could be created.");
 } else {
	message = i18n("No connection could be created.\nThe error message was:\n%1", s);
 }
 error(message, (QWidget*)parent());
}



// should become the real dialog - currently we just use messageboxes 
// -> maybe unused forever
KGameErrorMessageDialog::KGameErrorMessageDialog(QWidget* parent) 
		: KDialog(parent)
{
	setCaption(i18n("Error"));
	setButtons(Ok);
	setDefaultButton(Ok);
	setModal(true);
	showButtonSeparator(true);
}

KGameErrorMessageDialog::~KGameErrorMessageDialog()
{
}



#include "kgameerrordialog.moc"
