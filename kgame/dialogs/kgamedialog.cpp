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
#include <qhgroupbox.h>
#include <qvbox.h>

#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kgameconnectdialog.h"
#include "kgame.h"
#include "kgamechat.h"
#include "kplayer.h"
#include "kgameproperty.h"
#include "kgamedialogconfig.h"

#include "kgamedialog.h"

#include "kgamedialog.moc"

class KGameDialogPrivate
{
public:
	KGameDialogPrivate() 
	{
		mTopLayout = 0;
		mNetworkConfig = 0;
		mGameConfig = 0;
		mChat = 0;
		mPlayers = 0;
		mOwner = 0;
		mGame = 0;
		mGamePage = 0;
		mNetworkPage = 0;
		mMsgServerPage = 0;

	}

	QVBox* mGamePage;
	QVBox* mNetworkPage;
	QVBox* mMsgServerPage;// unused here?
	QVBoxLayout* mTopLayout;
	KGameDialogNetworkConfig* mNetworkConfig;
	KGameDialogGeneralConfig* mGameConfig;
	KGameChat* mChat;
	KListBox* mPlayers;
	QPtrDict<KPlayer> mItem2Player;

// a list of all config widgets added to this dialog
	QList<KGameDialogConfig> mConfigWidgets;

// for KGameConnectDialog
	unsigned short int mPort;
	QString mHost;


// just pointers:
	KPlayer* mOwner;
	KGame* mGame;
};

KGameDialog::KGameDialog(KGameDialogGeneralConfig* config, KGameDialogNetworkConfig* netConf, 
	KGameDialogMsgServerConfig* msgConf,
	KGame* g, KPlayer* owner, const QString& title, QWidget* parent, bool modal, int chatMsgid)
	: KDialogBase(Tabbed, title, Ok|Default|Apply, Ok, parent, 0, modal, true)
{
 init(config, netConf, msgConf, g, title, owner, chatMsgid);
}


KGameDialog::KGameDialog(KGame* g, KPlayer* owner, const QString& title, QWidget* parent, bool modal, int chatMsgid)
	: KDialogBase(Tabbed, title, Ok|Default|Apply,
	Ok, parent, 0, modal, true)
{
// make msgserver parameter configurable!
 init(new KGameDialogGeneralConfig(0), new KGameDialogNetworkConfig(0), new KGameDialogMsgServerConfig(0), g, title, owner, chatMsgid);

}

void KGameDialog::init(KGameDialogGeneralConfig* conf, KGameDialogNetworkConfig* netConf,
	KGameDialogMsgServerConfig* msgConf, 
	KGame* g, const QString& title, KPlayer* owner, int chatMsgid)
{
//TODO title
// kdDebug(11001) << "CONSTRUCT KGameDialog" << this << endl;
 d = new KGameDialogPrivate;

 setOwner(owner);
 setKGame(g);
 
 if (conf) {
	addGameConfig(conf);
 }
 if (netConf) {
	addNetworkConfig(netConf);
 }
 if (msgConf) {
	addMsgServerConfig(msgConf);
 }

//AB: do we need a "Cancel" Button? currently removed

// addConfigPages();
 addChatWidget(chatMsgid);

// add the connection management system - ie the widget where the ADMIN can
// kick players out
 addConnectionList();

// the owner of the dialog
 if (owner) {
	setOwner(owner);
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
 d->mGamePage = addConfigPage(d->mGameConfig, i18n("Page"));
// QHGroupBox* b = new QHGroupBox(i18n("Game Configuration"), d->mGamePage);
}

void KGameDialog::addNetworkConfig(KGameDialogNetworkConfig* netConf)
{
 if (!netConf) {
	return;
 }
// kdDebug(11001) << "adding network config" << endl;
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
 widget->show();
}

void KGameDialog::addChatWidget(int chatMsgId)
{
// kdDebug(11001) << "adding chat widget" << endl;
 if (!d->mGamePage) {
	kdError(11001) << "cannot add chat widget without game page" << endl;
 }
 QHGroupBox* b = new QHGroupBox(i18n("Chat"), d->mGamePage);
 d->mChat = new KGameChat(d->mGame, chatMsgId, b);
}

void KGameDialog::addConnectionList()
{
 if (!d->mNetworkPage) {
	kdWarning(11001) << "Cannot add connection list without network page" << endl;
	return;
 }
// kdDebug(11001) << "adding connection list" << endl;
 //TODO: prevent player to ban himself

 if (!d->mNetworkPage) {
	kdError(11001) << "cannot add connection list without network page" << endl;
	return;
 }
 QHGroupBox* b = new QHGroupBox(i18n("Connected Players"), d->mNetworkPage);
 d->mPlayers = new KListBox(b);
// h->addWidget(b, 1);
 if (d->mGame->isAdmin()) {
	connect(d->mPlayers, SIGNAL(executed(QListBoxItem*)), this, 
			SLOT(slotKickPlayerOut(QListBoxItem*)));
 }

// react to changes in KGame::playerList()
 connect(d->mGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
		this, SLOT(slotPlayerChanged(KPlayer*)));
 connect(d->mGame, SIGNAL(signalPlayerLeftGame(KPlayer*)), 
		this, SLOT(slotPlayerChanged(KPlayer*)));
}

QVBoxLayout* KGameDialog::layout() const 
{ return d->mTopLayout; }
KGameDialogGeneralConfig* KGameDialog::gameConfig() const
{ return d->mGameConfig; }
KGameDialogNetworkConfig* KGameDialog::networkConfig() const
{ return d->mNetworkConfig; }
void KGameDialog::setDefaultNetworkInfo(unsigned short int p, const QString& h)
{ d->mPort = p; d->mHost = h; }


void KGameDialog::slotPlayerChanged(KPlayer*)
{
 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	// disconnect everything first
	this->disconnect(it.current());
	++it;
 }
 d->mItem2Player.clear();
 d->mPlayers->clear();

 KGame::KGamePlayerList l = *d->mGame->playerList();
 for (KPlayer* p = l.first(); p; p = l.next()) {
	QListBoxText* t = new QListBoxText(p->name());
	d->mItem2Player.insert(t, p);
	d->mPlayers->insertItem(t);

	connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
			this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*)));
 }
}

void KGameDialog::slotKickPlayerOut(QListBoxItem* item)
{
 KPlayer* p = d->mItem2Player[item];
 if (!d->mGame || !d->mGame->isAdmin() || !p) {
	return;
 }

 if (p == d->mOwner) { // you wanna ban the ADMIN ??
	return;
 }

 if (KMessageBox::questionYesNo(this, 
		i18n("Do you want to ban player \"%1\" from the game?").arg(p->name())) == KMessageBox::Yes) {
	kdDebug(11001) << "will remove player " << p << endl;
	d->mGame->removePlayer(p);
	d->mPlayers->removeItem(d->mPlayers->index(item));
	slotPlayerChanged(p);
 } else {
	kdDebug(11001) << "will NOT remove player " << p << endl;
 }
}

void KGameDialog::slotPropertyChanged(KGamePropertyBase* prop, KPlayer* player)
{
 if(prop->id() == KGamePropertyBase::IdName) {
	QListBoxText* old = 0;
	QPtrDictIterator<KPlayer> it(d->mItem2Player);
	while (it.current() && !old) {
		if (it.current() == player) {
			old = (QListBoxText*)it.currentKey();
		}
		++it;
	}
	QListBoxText* t = new QListBoxText(player->name());
	d->mPlayers->changeItem(t, d->mPlayers->index(old));
	d->mItem2Player.remove(old);
	d->mItem2Player.insert(t, player);
 }
}

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
	d->mConfigWidgets.at(i)->setOwner(d->mOwner);
 }

 if (d->mChat) {
	if (!d->mOwner) {
		d->mChat->hide();
		//TODO: hide playerName in KGameDialogGeneralConfig
	} else {
		d->mChat->setFromPlayer(d->mOwner);
	}
 }
}

void KGameDialog::setKGame(KGame* g)
{
 if (!g) {
	kdError(11001) << "Cannot add NULL KGame!" << endl;
	return;
 }
 d->mGame = g;
 for (int unsigned i = 0; i < d->mConfigWidgets.count(); i++) {
	d->mConfigWidgets.at(i)->setKGame(d->mGame);
 }
}

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
