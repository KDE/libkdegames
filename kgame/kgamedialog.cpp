/* **************************************************************************
                            KGameDialog Class
                           -------------------
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
#include <qlayout.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qvbox.h>

#include <klistbox.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kseparator.h>

#include "kgameconnectdialog.h"
#include "kgame.h"
#include "kgamechat.h"
#include "kplayer.h"
#include "kplayerdata.h"
#include "kgamedialog.h"

#include "kgamedialog.moc"

class KGameDialogConfigPrivate
{
public:
	KGameDialogConfigPrivate() 
	{

	}

};

KGameDialogConfig::KGameDialogConfig(QWidget* parent) : QWidget(parent)
{
 d = new KGameDialogConfigPrivate;
}

KGameDialogConfig::~KGameDialogConfig()
{
 delete d;
}

void KGameDialogConfig::setKGame(KGame* )
{ }

void KGameDialogConfig::setOwner(KPlayer* )
{ }

//AB Master migh change to Admin very soon
void KGameDialogConfig::setMaster(bool )
{ }


class KGameDialogNetworkConfigPrivate
{
public:
	KGameDialogNetworkConfigPrivate() 
	{
//		mMaxClients = 0;
		mInitConnection = 0;
		mNetworkLabel = 0;
	}

//	KIntNumInput* mMaxClients;
	QPushButton* mInitConnection;
	QLabel* mNetworkLabel;
};


KGameDialogNetworkConfig::KGameDialogNetworkConfig(QWidget* parent) 
		: KGameDialogConfig(parent)
{
 kdDebug(11001) << "CONSTRUCT KGameDialogNetworkConfig " << this << endl;

 d = new KGameDialogNetworkConfigPrivate();

 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

 d->mNetworkLabel = new QLabel(i18n("No network"), this);
 topLayout->addWidget(d->mNetworkLabel);

 d->mInitConnection = new QPushButton(this);
 d->mInitConnection->setText(i18n("Start Network game"));
 connect(d->mInitConnection, SIGNAL(clicked()), this, SLOT(slotInitConnection()));
 topLayout->addWidget(d->mInitConnection);

// d->mMaxClients = new KIntNumInput(this);
// d->mMaxClients->setLabel(i18n("Maximal allowed connections:"));
// topLayout->addWidget(d->mMaxClients);
}

KGameDialogNetworkConfig::~KGameDialogNetworkConfig()
{ delete d; }

void KGameDialogNetworkConfig::disableInitConnection()
{ d->mInitConnection->setEnabled(false); }
void KGameDialogNetworkConfig::setNetworkText(const QString& text)
{ d->mNetworkLabel->setText(text); }
void KGameDialogNetworkConfig::slotInitConnection()
{
 bool connected = false;
 bool master = true;
 emit signalInitConnection(connected, master);
 if (!connected) {
	return;
 }
 d->mInitConnection->setEnabled(false);
 if (master) {
	d->mNetworkLabel->setText(i18n("You are MASTER"));
 } else {
	d->mNetworkLabel->setText(i18n("You are connected"));
 }
}

void KGameDialogNetworkConfig::submitToKGame(KGame* g, KPlayer* p)
{
 if (!g) {
	return;
 }
// g->setMaxClients(maxClients());//FIXME

}

void KGameDialogNetworkConfig::setKGame(KGame* g)
{
// setMaxClients(g->maxClients());
}


class KGameDialogGeneralConfigPrivate
{
public:
	KGameDialogGeneralConfigPrivate() {}
	
	KIntNumInput* mMaxPlayers;
	KIntNumInput* mMinPlayers;
	QLineEdit* mName;


	QGridLayout* mTopLayout; //TODO
};

KGameDialogGeneralConfig::KGameDialogGeneralConfig(QWidget* parent) 
		: KGameDialogConfig(parent)
{
 kdDebug(11001) << "CONSTRUCT KGameDialogGeneralConfig " << this << endl;

 d = new KGameDialogGeneralConfigPrivate();

 d->mTopLayout = new QGridLayout(this, 2, 2, KDialog::marginHint(), KDialog::spacingHint());

 d->mMaxPlayers = new KIntNumInput(this);
 d->mMaxPlayers->setLabel(i18n("Maximal number of players:"));
 d->mTopLayout->addWidget(d->mMaxPlayers, 0, 0);

 d->mMinPlayers = new KIntNumInput(this);
 d->mMinPlayers->setLabel(i18n("Minimum number of players:"));
 d->mTopLayout->addWidget(d->mMinPlayers, 0, 1);

 d->mName = new QLineEdit(this);
 d->mTopLayout->addWidget(d->mName, 1, 0);
}

KGameDialogGeneralConfig::~KGameDialogGeneralConfig()
{ delete d; } 

void KGameDialogGeneralConfig::setMaxPlayers(int m)
{ d->mMaxPlayers->setValue(m); }
void KGameDialogGeneralConfig::setPlayerName(const QString& name)
{ d->mName->setText(name); }
void KGameDialogGeneralConfig::setMinPlayers(int m)
{ d->mMinPlayers->setValue(m); }
int KGameDialogGeneralConfig::minPlayers() const
{ return d->mMinPlayers->value(); }
int KGameDialogGeneralConfig::maxPlayers() const
{ return d->mMaxPlayers->value(); }
QString KGameDialogGeneralConfig::playerName() const
{ return d->mName->text(); }
QGridLayout* KGameDialogGeneralConfig::layout() const
{ return d->mTopLayout; }

void KGameDialogGeneralConfig::setOwner(KPlayer* p) 
{
 if (!p) {
	// TODO
	// can this config be used at all?
	// maybe call hide()
	return;
 }
 setPlayerName(p->name());
 //TODO: connect signalPropertyChanged and check for playername changes!
}

void KGameDialogGeneralConfig::setKGame(KGame* g) 
{
 if (!g) {
	// TODO
	// can this config be used at all?
	// maybe call hide()
	return;
 }
 setMaxPlayers(g->maxPlayers());
 setMinPlayers(g->minPlayers());
}

void KGameDialogGeneralConfig::setMaster(bool master)
{
	d->mMaxPlayers->setEnabled(master);
	d->mMinPlayers->setEnabled(master);
}

void KGameDialogGeneralConfig::submitToKGame(KGame* g, KPlayer* p)
{
 if (p) {
	p->setName(playerName());
 }
 if (g) {
	if (g->gameMaster()) {// AB maybe g->admin()
		g->setMaxPlayers(maxPlayers());
		g->setMinPlayers(minPlayers());
	}
 }
}

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

	}

	QVBox* mGamePage;
	QVBox* mNetworkPage;
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
	KGame* g, KPlayer* owner, const QString& title, QWidget* parent, bool modal, int chatMsgid)
	: KDialogBase(Tabbed, title, Ok|Default|Apply, Ok, parent, 0, modal, true)
{
 init(config, netConf, g, title, owner, chatMsgid);
}


KGameDialog::KGameDialog(KGame* g, KPlayer* owner, const QString& title, QWidget* parent, bool modal, int chatMsgid)
	: KDialogBase(Tabbed, title, Ok|Default|Apply,
	Ok, parent, 0, modal, true)
{
 init(new KGameDialogGeneralConfig(0), new KGameDialogNetworkConfig(0), g, title, owner, chatMsgid);

}

void KGameDialog::init(KGameDialogGeneralConfig* conf, KGameDialogNetworkConfig* netConf,
	KGame* g, const QString& title, KPlayer* owner, int chatMsgid)
{
 kdDebug(11001) << "CONSTRUCT KGameDialog" << this << endl;
 d = new KGameDialogPrivate;

 setOwner(owner);
 setKGame(g);
 
 if (conf) {
	addGameConfig(conf);
 }
 if (netConf) {
	addNetworkConfig(netConf);
 }

//AB: do we need a "Cancel" Button? currently removed

// addConfigPages();
 addChatWidget(chatMsgid);

// add the connection management system - ie the widget where the MASTER can
// kick players out
 addConnectionList();

// the owner of the dialog
 if (owner) {
	setOwner(owner);
 }
}

KGameDialog::~KGameDialog()
{
 kdDebug(11001) << "DESTRUCT KGameDialog" << this << endl;
 delete d;
}

void KGameDialog::addGameConfig(KGameDialogGeneralConfig* conf)
{
 if (!conf) {
	return;
 }
 kdDebug(11001) << "adding game config" << endl;
 d->mGamePage = addVBoxPage(i18n("Game")); //TODO game specific

 QHGroupBox* b = new QHGroupBox(i18n("Game Configuration"), d->mGamePage);
 d->mGameConfig = conf;
 addConfigWidget(d->mGameConfig, b);
}

void KGameDialog::addNetworkConfig(KGameDialogNetworkConfig* netConf)
{
 if (!netConf) {
	return;
 }
 kdDebug(11001) << "adding network config" << endl;
 d->mNetworkPage = addVBoxPage(i18n("Network"));

 d->mNetworkConfig = netConf;
 if (networkConfig()) {
	connect(networkConfig(), SIGNAL(signalInitConnection(bool&, bool&)), this,
			SLOT(slotInitConnection(bool&, bool&)));// is this mGame dependant?
 }
 addConfigWidget(d->mNetworkConfig, d->mNetworkPage);
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
 kdDebug(11001) << "reparentin widget" << endl;
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
 kdDebug(11001) << "adding chat widget" << endl;
 if (!d->mGamePage) {
	kdError(11001) << "cannot add chat widget without game page" << endl;
 }
 QHGroupBox* b = new QHGroupBox(i18n("Chat"), d->mGamePage);
 d->mChat = new KGameChat(d->mGame, chatMsgId, b);
 d->mChat->setGame(d->mGame);
}

void KGameDialog::addConnectionList()
{
 if (!d->mNetworkPage) {
	kdWarning(11001) << "Cannot add connection list without network page" << endl;
	return;
 }
 kdDebug(11001) << "adding connection list" << endl;
 //TODO: prevent player to ban himself

 QHGroupBox* b = new QHGroupBox(i18n("Connected Players"), d->mNetworkPage);
 d->mPlayers = new KListBox(b);
// h->addWidget(b, 1);
 if (d->mGame->gameMaster()) {
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

	connect(p, SIGNAL(signalPropertyChanged(KPlayerDataBase*, KPlayer*)),
			this, SLOT(slotPropertyChanged(KPlayerDataBase*, KPlayer*)));
 }
}

void KGameDialog::slotKickPlayerOut(QListBoxItem* item)
{
 KPlayer* p = d->mItem2Player[item];
 if (!d->mGame || !d->mGame->gameMaster() || !p) {
	return;
 }

 if (p == d->mOwner) { // you wanna ban the MASTER??
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

void KGameDialog::slotPropertyChanged(KPlayerDataBase* prop, KPlayer* player)
{
 if(prop->id() == KPlayerDataBase::IdName) {
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
	d->mConfigWidgets.at(i)->submitToKGame(d->mGame, d->mOwner);
 }
}

