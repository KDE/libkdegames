/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlayout.h>
#include <qvbox.h>

#include <klocale.h>

#include "kgameconnectdialog.h"//maybe in KGameDialogNetworkConfig?
#include "kgame.h"
#include "kplayer.h"
#include "kgamedialogconfig.h"

#include "kgamedialog.h"

#include "kgamedialog.moc"

class KGameDialogPrivate
{
public:
	KGameDialogPrivate() 
	{
		mGamePage = 0;
		mNetworkPage = 0;
		mMsgServerPage = 0;
		mTopLayout = 0;

		mNetworkConfig = 0;
		mGameConfig = 0;

		mOwner = 0;
		mGame = 0;
	}

	QVBox* mGamePage;
	QVBox* mNetworkPage;
	QVBox* mMsgServerPage;// unused here?
	QVBoxLayout* mTopLayout;
	KGameDialogNetworkConfig* mNetworkConfig;
	KGameDialogGeneralConfig* mGameConfig;

// a list of all config widgets added to this dialog
	QList<KGameDialogConfig> mConfigWidgets;

// for KGameConnectDialog
	unsigned short int mPort;
	QString mHost;

// just pointers:
	KPlayer* mOwner;
	KGame* mGame;
};

KGameDialog::KGameDialog(KGame* g, KPlayer* owner, const QString& title, 
		QWidget* parent, bool modal, bool initConfigs, int chatMsgId)
	: KDialogBase(Tabbed, title, Ok|Default|Apply,
	Ok, parent, 0, modal, true)
{
 init(g, owner);
 if (initConfigs) {
	initDefaultDialog(0, 0, 0, new KGameDialogChatConfig(chatMsgId, 0), 0);
 }
}

void KGameDialog::init(KGame* g, KPlayer* owner)
{
//AB: do we need a "Cancel" Button? currently removed

// kdDebug(11001) << "CONSTRUCT KGameDialog" << this << endl;
 d = new KGameDialogPrivate;

 setOwner(owner);
 setKGame(g);
}

void KGameDialog::initDefaultDialog(KGameDialogGeneralConfig* conf, 
		KGameDialogNetworkConfig* netConf, 
		KGameDialogMsgServerConfig* msgConf,
		KGameDialogChatConfig* chat, 
		KGameDialogConnectionConfig* connection)
{
 if (conf) {
	addGameConfig(conf);
 } else {
	addGameConfig(new KGameDialogGeneralConfig(0));
 }
 if (netConf) {
	addNetworkConfig(netConf);
 } else {
	addNetworkConfig(new KGameDialogNetworkConfig(0));
 }
 if (msgConf) {
	addMsgServerConfig(msgConf);
 } else {
	addMsgServerConfig(new KGameDialogMsgServerConfig(0));
 }
 if (d->mGamePage && chat) {
	addChatWidget(chat, d->mGamePage);
 }
 if (d->mNetworkPage) {
	// add the connection management system - ie the widget where the ADMIN can
	// kick players out
	if (connection) {
		addConnectionList(connection, d->mNetworkPage);
	} else {
		addConnectionList(new KGameDialogConnectionConfig, d->mNetworkPage);
	}
 }
}

KGameDialog::~KGameDialog()
{
// kdDebug(11001) << "DESTRUCT KGameDialog" << this << endl;
 delete d;
}

void KGameDialog::addGameConfig(KGameDialogGeneralConfig* conf)
{
 if (!conf) {
	return;
 }
/* kdDebug(11001) << "adding game config" << endl;
 d->mGamePage = addVBoxPage(i18n("Game")); //TODO game specific

 QHGroupBox* b = new QHGroupBox(i18n("Game Configuration"), d->mGamePage);
 d->mGameConfig = conf;
 addConfigWidget(d->mGameConfig, b);*/

// kdDebug(11001) << "adding game config" << endl;
 d->mGameConfig = conf;
 d->mGamePage = addConfigPage(d->mGameConfig, i18n("Game"));
// QHGroupBox* b = new QHGroupBox(i18n("Game Configuration"), d->mGamePage);
}

void KGameDialog::addNetworkConfig(KGameDialogNetworkConfig* netConf)
{
 if (!netConf) {
	return;
 }
 d->mNetworkConfig = netConf;
 if (networkConfig()) {
	connect(networkConfig(), SIGNAL(signalInitConnection(bool&, bool&)), this,
			SLOT(slotInitConnection(bool&, bool&)));// is this mGame dependant?
 }
 d->mNetworkPage = addConfigPage(netConf, i18n("Network"));
}

void KGameDialog::addMsgServerConfig(KGameDialogMsgServerConfig* msgConf)
{
 if (!msgConf) {
	return;
 }
 d->mMsgServerPage = addConfigPage(msgConf, i18n("Message Server"));
}

void KGameDialog::addChatWidget(KGameDialogChatConfig* chat, QVBox* parent)
{
 if (!chat) {
	return;
 }
 if (!parent) {
	parent = d->mGamePage;
 }
 if (!parent) {
	kdError(11001) << "cannot add chat widget without page" << endl;
	return;
 }
// QHGroupBox* b = new QHGroupBox(i18n("Chat"), parent);
 addConfigWidget(chat, parent);
}

void KGameDialog::addConnectionList(KGameDialogConnectionConfig* c, QVBox* parent)
{
 if (!c) {
	return;
 }
 if (!parent) {
	parent = d->mNetworkPage;
 }
 if (!parent) {
	kdError(11001) << "Cannot add connection list without page" << endl;
	return;
 }

// QHGroupBox* b = new QHGroupBox(i18n("Connected Players"), parent);
 addConfigWidget(c, parent);
}

QVBox* KGameDialog::addConfigPage(KGameDialogConfig* widget, const QString& title)
{
 if (!widget) {
	kdError(11001) << "Cannot add NULL config widget" << endl;
	return 0;
 }
 QVBox* page = addVBoxPage(title);
 addConfigWidget(widget, page);
 return page;
}

void KGameDialog::addConfigWidget(KGameDialogConfig* widget, QWidget* parent)
{
 if (!widget) {
	kdError(11001) << "Cannot add NULL config widget" << endl;
	return;
 }
 if (!parent) {
	kdError(11001) << "Cannot reparent to NULL widget" << endl;
	return;
 }
// kdDebug(11001) << "reparenting widget" << endl;
 widget->reparent(parent, QPoint(0,0));
 d->mConfigWidgets.append(widget);
 if (!d->mGame) {
	kdWarning(11001) << "No game has been set!" << endl;
 } else {
	widget->setKGame(d->mGame);
 }
 if (!d->mOwner) {
	kdWarning(11001) << "No player has been set!" << endl;
 } else {
	widget->setOwner(d->mOwner);
 }
 //TODO: call setAdmin?
 widget->show();
}

KGameDialogGeneralConfig* KGameDialog::gameConfig() const
{ return d->mGameConfig; }
KGameDialogNetworkConfig* KGameDialog::networkConfig() const
{ return d->mNetworkConfig; }
void KGameDialog::setDefaultNetworkInfo(unsigned short int p, const QString& h)
{ d->mPort = p; d->mHost = h; }

void KGameDialog::slotApply()
{
 submitToKGame();
}

void KGameDialog::slotDefault()
{
 if (!d->mGame) {
	return;
 }
 
//TODO *only*  call setKGame/setOwner for the *current* page!!
 setKGame(d->mGame);
 setOwner(d->mOwner);
}

void KGameDialog::slotOk()
{
 slotApply();
 return QDialog::accept();
}

void KGameDialog::slotInitConnection(bool& connected, bool& master)
{
 int result = KGameConnectDialog::initConnection(d->mPort, d->mHost, this, true);
 if (result != QDialog::Accepted) {
	connected = false;
	return;
 }
 if (d->mHost.isNull()) {
	master = true;
	if (d->mGame) {
		connected = d->mGame->offerConnections(d->mPort);
	}
	return;
 } else {
	master = false;
	if (d->mGame) {
		connected = d->mGame->connectToServer(d->mHost, d->mPort);
	}
	return;
 }
}

void KGameDialog::setOwner(KPlayer* owner)
{
//AB: note: NULL player is ok!
 d->mOwner = owner;
 for (int unsigned i = 0; i < d->mConfigWidgets.count(); i++) {
	if (d->mConfigWidgets.at(i)) {
		d->mConfigWidgets.at(i)->setOwner(d->mOwner);
		//TODO: hide playerName in KGameDialogGeneralConfig
	} else {
		kdError(11001) << "NULL widget??" << endl;
	}
 }

/* if (d->mChat) {
	if (!d->mOwner) {
		d->mChat->hide();
	} else {
		d->mChat->setFromPlayer(d->mOwner);
	}
 }*/
}

void KGameDialog::setKGame(KGame* g)
{
 if (d->mGame) {
	disconnect(d->mGame, 0, this, 0);
 }
 d->mGame = g;
 connect(d->mGame, SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));
 for (int unsigned i = 0; i < d->mConfigWidgets.count(); i++) {
	d->mConfigWidgets.at(i)->setKGame(d->mGame);
 }
}

void KGameDialog::slotUnsetKGame() // called when KGame is destroyed
{ setKGame(0); }

void KGameDialog::submitToKGame()
{
 if (!d->mGame) {
	kdError(11001) << "KGameDialog::submitToKGame: no game has been set" << endl;
	return;
 }
 if (!d->mOwner) {
	kdError(11001) << "KGameDialog::submitToKGame: no player has been set" << endl;
	return;
 }

 for (int unsigned i = 0; i < d->mConfigWidgets.count(); i++) {
// kdDebug(11001) << "submit to kgame " << i << endl;
	d->mConfigWidgets.at(i)->submitToKGame(d->mGame, d->mOwner);
// kdDebug(11001) << "done: submit to kgame " << i << endl;
 }
}
