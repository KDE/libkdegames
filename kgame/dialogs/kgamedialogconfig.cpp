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

#include "kgamedialogconfig.h"

#include "kgame.h"
#include "kplayer.h"
#include "kgamechat.h"
#include "kgameconnectdialog.h"

#include <klocale.h>
#include <knuminput.h>
#include <kdialog.h>
#include <klistbox.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qptrdict.h>

#include "kgamedialogconfig.moc"

class KGameDialogConfigPrivate
{
public:
	KGameDialogConfigPrivate()
	{
		mOwner = 0;
		mGame = 0;

		mAdmin = false;
	}

	bool mAdmin;
	KGame* mGame;
	KPlayer* mOwner;
};

KGameDialogConfig::KGameDialogConfig(QWidget* parent) : QWidget(parent)
{
 d = new KGameDialogConfigPrivate;
}

KGameDialogConfig::~KGameDialogConfig()
{
 kdDebug(11001) << k_funcinfo << endl;
 delete d;
}

void KGameDialogConfig::setKGame(KGame* g)
{
 d->mGame = g;
}

void KGameDialogConfig::setOwner(KPlayer* p)
{
 d->mOwner = p;
}

void KGameDialogConfig::setAdmin(bool a)
{
 d->mAdmin = a;
}

KGame* KGameDialogConfig::game() const
{ return d->mGame; }
bool KGameDialogConfig::admin() const
{ return d->mAdmin; }
KPlayer* KGameDialogConfig::owner() const
{ return d->mOwner; }

/////////////////////////// KGameDialogNetworkConfig /////////////////////////
class KGameDialogNetworkConfigPrivate
{
public:
	KGameDialogNetworkConfigPrivate()
	{
		mInitConnection = 0;
		mNetworkLabel = 0;
		mDisconnectButton = 0;
		mConnect = 0;
		mDefaultServer=true;

	}

	// QPushButton* mInitConnection;
	QHGroupBox* mInitConnection;
	QLabel* mNetworkLabel;
	QPushButton *mDisconnectButton;

	bool mDefaultServer;
	QString mDefaultHost;
	unsigned short int mDefaultPort;
	KGameConnectWidget *mConnect;
};


KGameDialogNetworkConfig::KGameDialogNetworkConfig(QWidget* parent)
		: KGameDialogConfig(parent)
{
// kdDebug(11001) << k_funcinfo << ": this=" << this << endl;
 d = new KGameDialogNetworkConfigPrivate();

 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint(), "toplayout");

 QHBoxLayout *hb = new QHBoxLayout(topLayout, KDialog::spacingHint());

 d->mNetworkLabel = new QLabel(this);
 hb->addWidget(d->mNetworkLabel);

 d->mDisconnectButton=new QPushButton(i18n("Disconnect"),this);
 connect(d->mDisconnectButton, SIGNAL(clicked()), this, SLOT(slotExitConnection()));
 hb->addWidget(d->mDisconnectButton);

 d->mInitConnection = new QHGroupBox(i18n("Network Configuration"), this);
 topLayout->addWidget(d->mInitConnection);

 d->mConnect = new KGameConnectWidget(d->mInitConnection);
 connect(d->mConnect, SIGNAL(signalNetworkSetup()), this, SLOT(slotInitConnection()));
 connect(d->mConnect, SIGNAL(signalServerTypeChanged(int)),
         this, SIGNAL(signalServerTypeChanged(int)));

 // Needs to be AFTER the creation of the dialogs
 setConnected(false);
 setDefaultNetworkInfo("localhost", 7654,true);
}

KGameDialogNetworkConfig::~KGameDialogNetworkConfig()
{
 kdDebug(11001) << k_funcinfo << endl;
 delete d;
}

void KGameDialogNetworkConfig::slotExitConnection()
{
 kdDebug(11001) << k_funcinfo << " !!!!!!!!!!!!!!!!!!!!!!!" << endl;
  if (game()) game()->disconnect();
  setConnected(false,false);
}

void KGameDialogNetworkConfig::slotInitConnection()
{
 kdDebug(11001) << k_funcinfo << endl;
 bool connected = false;
 bool master = true;
 unsigned short int port = d->mConnect->port();
 QString host = d->mConnect->host();

 if (host.isNull()) {
	master = true;
	if (game()) {
		connected = game()->offerConnections(port);
	}
 } else {
	master = false;
	if (game()) {
		connected = game()->connectToServer(host, port);
	}
  // We need to learn about failed connections
  if (game()) {
     connect(game(), SIGNAL(signalConnectionBroken()),
      this, SLOT(slotConnectionBroken()));
  }
 }
 setConnected(connected, master);
}

void KGameDialogNetworkConfig::slotConnectionBroken()
{
  kdDebug(11001) << k_funcinfo << endl;
  setConnected(false,false);
  KMessageBox::error(this, i18n("Cannot connect to the network"));
}

void KGameDialogNetworkConfig::setConnected(bool connected, bool master)
{
 if (!connected) {
	d->mNetworkLabel->setText(i18n("Network status: No Network"));
	d->mInitConnection->setEnabled(true);
  d->mDisconnectButton->setEnabled(false);
	return;
 }
 if (master) {
	d->mNetworkLabel->setText(i18n("Network status: You are MASTER"));
 } else {
	d->mNetworkLabel->setText(i18n("Network status: You are connected"));
 }
 d->mInitConnection->setEnabled(false);
 d->mDisconnectButton->setEnabled(true);
}

void KGameDialogNetworkConfig::submitToKGame(KGame* , KPlayer* )
{
}

void KGameDialogNetworkConfig::setKGame(KGame* g)
{
 KGameDialogConfig::setKGame(g);
 if (!game()) {
	setConnected(false);
	return;
 }
 setConnected(game()->isNetwork(), game()->isMaster());
}

void KGameDialogNetworkConfig::setDefaultNetworkInfo(const QString& host, unsigned short int port,bool server)
{
 d->mDefaultPort = port;
 d->mDefaultHost = host;
 d->mDefaultServer = server;

 d->mConnect->setHost(host);
 d->mConnect->setPort(port);
 if (server) {
	d->mConnect->setDefault(0);
 } else {
	d->mConnect->setDefault(1);
 }
}

/////////////////////////// KGameDialogGeneralConfig /////////////////////////
class KGameDialogGeneralConfigPrivate
{
public:
	KGameDialogGeneralConfigPrivate()
	{
		mTopLayout = 0;
		mName = 0;
	}

	QLineEdit* mName;

	QVBoxLayout* mTopLayout;
};

KGameDialogGeneralConfig::KGameDialogGeneralConfig(QWidget* parent, bool initializeGUI)
		: KGameDialogConfig(parent)
{
// kdDebug(11001) << k_funcinfo << ": this=" << this << endl;
 d = new KGameDialogGeneralConfigPrivate;

 if (initializeGUI) {
	d->mTopLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
	d->mTopLayout->setAutoAdd(true);

	QWidget* nameWidget = new QWidget(this);
	QHBoxLayout* l = new QHBoxLayout(nameWidget);
	QLabel* nameLabel = new QLabel(i18n("Your name:"), nameWidget);
	l->addWidget(nameLabel);
	d->mName = new QLineEdit(nameWidget);
	l->addWidget(d->mName);
 }
}

KGameDialogGeneralConfig::~KGameDialogGeneralConfig()
{
 kdDebug(11001) << k_funcinfo << endl;
 delete d;
}

void KGameDialogGeneralConfig::setPlayerName(const QString& name)
{
 if (d->mName) {
	d->mName->setText(name);
 }
}

QString KGameDialogGeneralConfig::playerName() const
{
 return d->mName ? d->mName->text() : QString::null;
}

void KGameDialogGeneralConfig::setOwner(KPlayer* p)
{
 if (owner()) {
	owner()->disconnect(this);
 }
 KGameDialogConfig::setOwner(p);
 if (!owner()) {
	// can this config be used at all?
	// maybe call hide()
	return;
 }
 connect(owner(), SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*)));
 setPlayerName(p->name());
 //TODO: connect signalPropertyChanged and check for playername changes!
}

void KGameDialogGeneralConfig::setKGame(KGame* g)
{
 KGameDialogConfig::setKGame(g);
 if (!g) {
	// TODO
	// can this config be used at all?
	// maybe call hide()
	return;
 }
}

void KGameDialogGeneralConfig::setAdmin(bool admin)
{
 KGameDialogConfig::setAdmin(admin);
// enable/disable widgets

}

void KGameDialogGeneralConfig::submitToKGame(KGame* g, KPlayer* p)
{
//FIXME
 if (p) {
	p->setName(playerName());
 }
 if (g) {
 }
}

void KGameDialogGeneralConfig::slotPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
 if (!prop || !p || p != owner()) {
	return;
 }
 switch (prop->id()) {
	case KGamePropertyBase::IdName:
		setPlayerName(p->name());
		break;
	default:
		break;
 }
}

class KGameDialogMsgServerConfigPrivate
{
public:
	KGameDialogMsgServerConfigPrivate()
	{
		senderLayout = 0;
		localLayout = 0;

		changeMaxClients = 0;
		changeAdmin= 0;
		removeClient= 0;
		noAdmin = 0;

		noMaster = 0;
	}

	QVBoxLayout* senderLayout;
	QHBoxLayout* localLayout;

	QPushButton* changeMaxClients;
	QPushButton* changeAdmin;
	QPushButton* removeClient;
	QLabel* noAdmin;

	QLabel* noMaster;
};


// TODO: change ADMIN ID, remove CLIENTS, change MAXCLIENTS
// we do everything here with QPushButtons as we want to wait a moment before
// continuing - the message must be sent over network first
KGameDialogMsgServerConfig::KGameDialogMsgServerConfig(QWidget* parent)
		: KGameDialogConfig(parent)
{
 d = new KGameDialogMsgServerConfigPrivate;

 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
 d->senderLayout = new QVBoxLayout(topLayout);
 d->localLayout = new QHBoxLayout(topLayout);
}

KGameDialogMsgServerConfig::~KGameDialogMsgServerConfig()
{
 kdDebug(11001) << k_funcinfo << endl;
 delete d;
}

void KGameDialogMsgServerConfig::setKGame(KGame* g)
{
 KGameDialogConfig::setKGame(g);
 //TODO display the ID of the admin if we aren't
 // connect(g, SIGNAL(signalAdminChanged(int)), this, SLOT(slotChangeIsAdmin(int)));//TODO
 if (!game()) {
	// we cannot do anything without a KGame object!
	setAdmin(false);
	return;
 }
 setAdmin(game()->isAdmin());
 setHasMsgServer(game()->messageServer());
}


void KGameDialogMsgServerConfig::slotChangeMaxClients()
{
 if (!game()) {
	kdError(11001) << k_funcinfo << ": no valid game object available!" << endl;
	return;
 }
 if (!game()->isAdmin()) {
	kdError(11001) << k_funcinfo << ": only ADMIN is allowed to call this!" << endl;
	return;
 }
 int max;
// edit->setText(QString::number()); // current max clients! //TODO

 QDialog* dialog = new QDialog();
 dialog->setCaption(i18n("Maximal Number of Clients"));
 QHBoxLayout* l = new QHBoxLayout(dialog, KDialog::marginHint(), KDialog::spacingHint());
 l->setAutoAdd(true);

 (void) new QLabel(i18n("Maximal number of clients (-1 = infinite):"), dialog);
 QLineEdit* edit = new QLineEdit(dialog);//TODO: use KIntNumInput
// edit->setText(QString::number(max)); // current max clients! //TODO
 if (dialog->exec() == QDialog::Accepted) {
	bool ok;
	max = edit->text().toInt(&ok);
	if (ok) {
		game()->setMaxClients(max);
	}
 }

}

void KGameDialogMsgServerConfig::slotRemoveClient()
{
}

void KGameDialogMsgServerConfig::slotChangeAdmin()
{
 if (!game()) {
	kdError(11001) << k_funcinfo << ": no valid game object available!" << endl;
	return;
 }
 if (!admin()) {
	kdError(11001) << k_funcinfo << ": only ADMIN is allowed to call this!" << endl;
	return;
 }
 //TODO
 Q_UINT32 newAdmin = 0;
// newAdmin = ;
 game()->electAdmin(newAdmin);
}

void KGameDialogMsgServerConfig::removeClient(Q_UINT32 /*id*/)
{
//TODO
}

void KGameDialogMsgServerConfig::setAdmin(bool a)
{
 if (admin() == a) {
	// no need to do anything
	return;
 }
 KGameDialogConfig::setAdmin(a);
 if (admin()) {
	if (d->noAdmin) {
		delete d->noAdmin;
		d->noAdmin = 0;
	}
	d->changeMaxClients = new QPushButton(i18n("Change Maximal Number of Clients"), this);
	connect(d->changeMaxClients, SIGNAL(pressed()), this, SLOT(slotChangeMaxClients()));
	d->changeAdmin = new QPushButton(i18n("Change Admin"), this);
	connect(d->changeAdmin, SIGNAL(pressed()), this, SLOT(slotChangeAdmin()));
	d->removeClient = new QPushButton(i18n("Remove Client With All Players"), this);
	connect(d->removeClient, SIGNAL(pressed()), this, SLOT(slotRemoveClient()));
	d->senderLayout->addWidget(d->changeMaxClients);
	d->senderLayout->addWidget(d->changeAdmin);
	d->senderLayout->addWidget(d->removeClient);
 } else {
	if (d->changeMaxClients) {
		delete d->changeMaxClients;
		d->changeMaxClients = 0;
	}
	if (d->changeAdmin) {
		delete d->changeAdmin;
		d->changeAdmin = 0;
	}
	if (d->removeClient) {
		delete d->removeClient;
		d->removeClient = 0;
	}
	d->noAdmin = new QLabel(i18n("Only the admin can configure the message server!"), this);
	d->senderLayout->addWidget(d->noAdmin);
 }
}


void KGameDialogMsgServerConfig::setHasMsgServer(bool has)
{
 if (!has) {
	// delete all inputs
	if (!d->noMaster) {
		d->noMaster = new QLabel(i18n("You don't own the message server"), this);
		d->localLayout->addWidget(d->noMaster);
	}
	return;
 }
 if (d->noMaster) {
	delete d->noMaster;
	d->noMaster = 0;
 }
 //TODO
 // list all connections, data (max clients) and so on
 // cannot be done above (together with QPushButtons) as it is possible that
 // this client is ADMIN but not MASTER (i.e. doesn't own the messageserver)
}


class KGameDialogChatConfigPrivate
{
public:
	KGameDialogChatConfigPrivate()
	{
		mChat = 0;
	}

	KGameChat* mChat;
};

KGameDialogChatConfig::KGameDialogChatConfig(int chatMsgId, QWidget* parent)
		: KGameDialogConfig(parent)
{
 d = new KGameDialogChatConfigPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
 topLayout->setAutoAdd(true);
 QHGroupBox* b = new QHGroupBox(i18n("Chat"), this);
 d->mChat = new KGameChat(0, chatMsgId, b);
}

KGameDialogChatConfig::~KGameDialogChatConfig()
{
 kdDebug(11001) << k_funcinfo << endl;
 delete d;
}

void KGameDialogChatConfig::setKGame(KGame* g)
{
 KGameDialogConfig::setKGame(g);
 d->mChat->setKGame(game());
 if (!game()) {
	hide();
 } else {
	show();
 }
}

void KGameDialogChatConfig::setOwner(KPlayer* p)
{
 KGameDialogConfig::setOwner(p);
 if (!owner()) {
	hide();
	return;
 }
 d->mChat->setFromPlayer(owner());
 show();
}



class KGameDialogConnectionConfigPrivate
{
public:
	KGameDialogConnectionConfigPrivate()
	{
		mPlayerBox = 0;
	}

	QPtrDict<KPlayer> mItem2Player;
	KListBox* mPlayerBox;
};

KGameDialogConnectionConfig::KGameDialogConnectionConfig(QWidget* parent)
		: KGameDialogConfig(parent)
{
 //TODO: prevent player to ban himself
 d = new KGameDialogConnectionConfigPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
 topLayout->setAutoAdd(true);
 QHGroupBox* b = new QHGroupBox(i18n("Connected Players"), this);
 d->mPlayerBox = new KListBox(b);
 setMinimumHeight(100);
}

KGameDialogConnectionConfig::~KGameDialogConnectionConfig()
{
 kdDebug(11001) << k_funcinfo << endl;
 // d->mIem2Player.clear();
 delete d;
}

void KGameDialogConnectionConfig::setKGame(KGame* g)
{
 if (game()) {
	disconnect(game(), 0, this, 0);
 }
 KGameDialogConfig::setKGame(g);
 slotClearPlayers();
 if (game()) {
// react to changes in KGame::playerList()
	connect(game(), SIGNAL(signalPlayerJoinedGame(KPlayer*)),
			this, SLOT(slotPlayerJoinedGame(KPlayer*)));
	connect(game(), SIGNAL(signalPlayerLeftGame(KPlayer*)),
			this, SLOT(slotPlayerLeftGame(KPlayer*)));

	KGame::KGamePlayerList l = *game()->playerList();
	for (KPlayer* p = l.first(); p; p = l.next()) {
		slotPlayerJoinedGame(p);
	}
 }
}

void KGameDialogConnectionConfig::setOwner(KPlayer* p)
{
 KGameDialogConfig::setOwner(p);
}

void KGameDialogConnectionConfig::setAdmin(bool a)
{
 if (!game()) {// not possible... in theory
	return;
 }
 if (admin()) {
	disconnect(game(), SIGNAL(executed(QListBoxItem*)), this, 0);
 }
 KGameDialogConfig::setAdmin(a);
 if (admin()) {
	connect(d->mPlayerBox, SIGNAL(executed(QListBoxItem*)), this,
			SLOT(slotKickPlayerOut(QListBoxItem*)));
 }
}

QListBoxItem* KGameDialogConnectionConfig::item(KPlayer* p) const
{
 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	if (it.current() == p) {
		return (QListBoxItem*)it.currentKey();
	}
	++it;
 }
 return 0;
}

void KGameDialogConnectionConfig::slotClearPlayers()
{
 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	slotPlayerLeftGame(it.current());
	++it;
 }

 if (d->mItem2Player.count() > 0) {
	kdWarning(11001) << k_funcinfo << ": itemList wasn't cleared properly" << endl;
	d->mItem2Player.clear();
 }
 if (d->mPlayerBox->count() > 0) {
	kdWarning(11001) << k_funcinfo << ": listBox wasn't cleared properly" << endl;
	d->mPlayerBox->clear();
 }

}

void KGameDialogConnectionConfig::slotPlayerJoinedGame(KPlayer* p)
{
 if (!p) {
	kdError(11001) << k_funcinfo << ": Cannot add NULL player" << endl;
 }
 if (d->mItem2Player[p]) {
	kdError(11001) << k_funcinfo << ": attempt to double add player" << endl;
	return;
 }
 kdDebug(11001) << k_funcinfo << ": add player " << p->id() << endl;
 QListBoxText* t = new QListBoxText(p->name());
 d->mItem2Player.insert(t, p);
 d->mPlayerBox->insertItem(t);

 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*)));

}

void KGameDialogConnectionConfig::slotPlayerLeftGame(KPlayer* p)
{
 // disconnect first
 this->disconnect(p);
 if (!item(p)) {
	kdError(11001) << k_funcinfo << ": cannot find " << p->id()
			<< " in list" << endl;
	return;
 }
 d->mPlayerBox->removeItem(d->mPlayerBox->index(item(p)));

}

void KGameDialogConnectionConfig::slotKickPlayerOut(QListBoxItem* item)
{
 kdDebug(11001) << "kick player out" << endl;
 KPlayer* p = d->mItem2Player[item];
 if (!p) {
	kdError(11001) << "invalid item selected - no player found" << endl;
	return;
 }
 if (!game()) {
	kdWarning(11001) << "no game set" << endl;
	return;
 }
 if (!admin()) {
	kdDebug(11001) << "Only the ADMIN can kick players" << endl;
	return;
 }
 if (p == owner()) { // you wanna ban the ADMIN ??
	kdDebug(11001) << "you cannot kick the ADMIN" << endl;
	return;
 }

 if (KMessageBox::questionYesNo(this, i18n("Do you want to ban player \"%1\" from the game?").arg(
		p->name())) == KMessageBox::Yes) {
	kdDebug(11001) << "will remove player " << p << endl;
	game()->removePlayer(p);
//	d->mPlayerBox->removeItem(d->mPlayerBox->index(item)); // should be done by signalPlayerLeftGame
 } else {
	kdDebug(11001) << "will NOT remove player " << p << endl;
 }
}

void KGameDialogConnectionConfig::slotPropertyChanged(KGamePropertyBase* prop, KPlayer* player)
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
	d->mPlayerBox->changeItem(t, d->mPlayerBox->index(old));
	d->mItem2Player.remove(old);
	d->mItem2Player.insert(t, player);
 }
}

