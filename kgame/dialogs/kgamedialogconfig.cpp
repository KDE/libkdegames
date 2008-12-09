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

#include "kgamedialogconfig.h"

#include "kgame.h"
#include "kplayer.h"
#include "kgamechat.h"
#include "kgameconnectdialog.h"
#include "kgameproperty.h"

#include <klistwidget.h>
#include <klocale.h>
#include <knuminput.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

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

KGameDialogConfig::KGameDialogConfig(QWidget* parent) 
    : QWidget( parent ),
      d( new KGameDialogConfigPrivate )
{
}

KGameDialogConfig::~KGameDialogConfig()
{
 kDebug(11001) ;
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
	QGroupBox* mInitConnection;
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
// kDebug(11001) << ": this=" << this;
 d = new KGameDialogNetworkConfigPrivate();

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->setMargin( KDialog::marginHint() );
 topLayout->setSpacing( KDialog::spacingHint() );

 QHBoxLayout *hb = new QHBoxLayout;
 hb->setSpacing( KDialog::spacingHint() );
 topLayout->addLayout(hb);

 d->mNetworkLabel = new QLabel(this);
 hb->addWidget(d->mNetworkLabel);

 d->mDisconnectButton=new QPushButton(i18n("Disconnect"),this);
 connect(d->mDisconnectButton, SIGNAL(clicked()), this, SLOT(slotExitConnection()));
 hb->addWidget(d->mDisconnectButton);

 d->mInitConnection = new QGroupBox(i18n("Network Configuration"), this);
 QHBoxLayout* gboxLay = new QHBoxLayout(d->mInitConnection);
 topLayout->addWidget(d->mInitConnection);

 d->mConnect = new KGameConnectWidget(d->mInitConnection);
 gboxLay->addWidget(d->mConnect);
 connect(d->mConnect, SIGNAL(signalNetworkSetup()), this, SLOT(slotInitConnection()));
 connect(d->mConnect, SIGNAL(signalServerTypeChanged(int)),
         this, SIGNAL(signalServerTypeChanged(int)));

 // Needs to be AFTER the creation of the dialogs
 setConnected(false);
 setDefaultNetworkInfo("localhost", 7654,true);
}

KGameDialogNetworkConfig::~KGameDialogNetworkConfig()
{
 kDebug(11001) ;
 delete d;
}

void KGameDialogNetworkConfig::slotExitConnection()
{
 kDebug(11001) << " !!!!!!!!!!!!!!!!!!!!!!!";
  if (game()) game()->disconnect();
  setConnected(false,false);
}

void KGameDialogNetworkConfig::slotInitConnection()
{
 kDebug(11001) ;
 bool connected = false;
 bool master = true;
 unsigned short int port = d->mConnect->port();
 QString host = d->mConnect->host();

 if (host.isNull()) {
	master = true;
	if (game()) {
		game()->setDiscoveryInfo(d->mConnect->type(),d->mConnect->gameName());
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
  kDebug(11001) ;
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

void KGameDialogNetworkConfig::setDiscoveryInfo(const QString& type, const QString& name)
{
 d->mConnect->setType(type);
 d->mConnect->setName(name);
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

	KLineEdit* mName;

	QVBoxLayout* mTopLayout;
};

KGameDialogGeneralConfig::KGameDialogGeneralConfig(QWidget* parent, bool initializeGUI)
		: KGameDialogConfig(parent)
{
// kDebug(11001) << ": this=" << this;
 d = new KGameDialogGeneralConfigPrivate;

 if (initializeGUI) {
	d->mTopLayout = new QVBoxLayout(this);
	d->mTopLayout->setMargin( KDialog::marginHint() );
	d->mTopLayout->setSpacing( KDialog::spacingHint() );

	QWidget* nameWidget = new QWidget(this);
        d->mTopLayout->addWidget(nameWidget);
	QHBoxLayout* l = new QHBoxLayout(nameWidget);
	QLabel* nameLabel = new QLabel(i18n("Your name:"), nameWidget);
	l->addWidget(nameLabel);
	d->mName = new KLineEdit(nameWidget);
	l->addWidget(d->mName);
 }
}

KGameDialogGeneralConfig::~KGameDialogGeneralConfig()
{
 kDebug(11001) ;
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
 return d->mName ? d->mName->text() : QString();
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

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->setMargin( KDialog::marginHint() );
 topLayout->setSpacing( KDialog::spacingHint() );
 d->senderLayout = new QVBoxLayout;
 d->localLayout = new QHBoxLayout;
 topLayout->addLayout( d->senderLayout );
 topLayout->addLayout( d->localLayout );
}

KGameDialogMsgServerConfig::~KGameDialogMsgServerConfig()
{
 kDebug(11001) ;
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
	kError(11001) << ": no valid game object available!";
	return;
 }
 if (!game()->isAdmin()) {
	kError(11001) << ": only ADMIN is allowed to call this!";
	return;
 }
 int max;
// edit->setText(QString::number()); // current max clients! //TODO

 QDialog* dialog = new QDialog();
 dialog->setWindowTitle(i18n("Maximal Number of Clients"));
 QHBoxLayout* l = new QHBoxLayout(dialog);
 l->setMargin( KDialog::marginHint() );
 l->setSpacing( KDialog::spacingHint() );

 l->addWidget(new QLabel(i18n("Maximal number of clients (-1 = infinite):"), dialog));
 KLineEdit* edit = new KLineEdit(dialog);//TODO: use KIntNumInput
 l->addWidget(edit);
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
	kError(11001) << ": no valid game object available!";
	return;
 }
 if (!admin()) {
	kError(11001) << ": only ADMIN is allowed to call this!";
	return;
 }
 //TODO
 quint32 newAdmin = 0;
// newAdmin = ;
 game()->electAdmin(newAdmin);
}

void KGameDialogMsgServerConfig::removeClient(quint32 /*id*/)
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
	d->removeClient = new QPushButton(i18n("Remove Client with All Players"), this);
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
		d->noMaster = new QLabel(i18n("You do not own the message server"), this);
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
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->setMargin( KDialog::marginHint() );
 topLayout->setSpacing( KDialog::spacingHint() );
 QGroupBox* b = new QGroupBox(i18n("Chat"), this);
 topLayout->addWidget(b);
 QHBoxLayout* gboxLay = new QHBoxLayout(b);
 d->mChat = new KGameChat(0, chatMsgId, b);
 gboxLay->addWidget(d->mChat);
}

KGameDialogChatConfig::~KGameDialogChatConfig()
{
 kDebug(11001) ;
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

	QHash<QListWidgetItem*, KPlayer*> mItem2Player;
	KListWidget* mPlayerBox;
};

KGameDialogConnectionConfig::KGameDialogConnectionConfig(QWidget* parent)
		: KGameDialogConfig(parent)
{
 //TODO: prevent player to ban himself
 d = new KGameDialogConnectionConfigPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->setMargin( KDialog::marginHint() );
 topLayout->setSpacing( KDialog::spacingHint() );
 QGroupBox* b = new QGroupBox(i18n("Connected Players"), this);
 topLayout->addWidget(b);
 QHBoxLayout* gboxLay = new QHBoxLayout(b);
 d->mPlayerBox = new KListWidget(b);
 gboxLay->addWidget(d->mPlayerBox);
 setMinimumHeight(100);
}

KGameDialogConnectionConfig::~KGameDialogConnectionConfig()
{
 kDebug(11001) ;
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

  const QList<KPlayer*> *playerList = game()->playerList();
  for ( QList<KPlayer*>::const_iterator it = playerList->begin(); it!=playerList->end();it++ )
  {
		slotPlayerJoinedGame(*it);
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
	disconnect(game(), SIGNAL(executed(QListWidgetItem*)), this, 0);
 }
 KGameDialogConfig::setAdmin(a);
 if (admin()) {
	connect(d->mPlayerBox, SIGNAL(executed(QListWidgetItem*)), this,
			SLOT(slotKickPlayerOut(QListWidgetItem*)));
 }
}

QListWidgetItem* KGameDialogConnectionConfig::item(KPlayer* p) const
{
 QHash<QListWidgetItem*, KPlayer*>::const_iterator it, itEnd;
 it = d->mItem2Player.constBegin();
 itEnd = d->mItem2Player.constEnd();
 for ( ; it != itEnd; ++it ) {
	if (it.value() == p) return it.key();
 }
 return 0;
}

void KGameDialogConnectionConfig::slotClearPlayers()
{
 QHash<QListWidgetItem*, KPlayer*>::const_iterator it, itEnd;
 it = d->mItem2Player.constBegin();
 itEnd = d->mItem2Player.constEnd();
 for ( ; it != itEnd; ++it ) {
	slotPlayerLeftGame(it.value());
 }

 if (d->mItem2Player.count() > 0) {
	kWarning(11001) << ": itemList wasn't cleared properly";
	d->mItem2Player.clear();
 }
 if (d->mPlayerBox->count() > 0) {
	kWarning(11001) << ": listBox wasn't cleared properly";
	d->mPlayerBox->clear();
 }

}

void KGameDialogConnectionConfig::slotPlayerJoinedGame(KPlayer* p)
{
 if (!p) {
	kError(11001) << ": Cannot add NULL player";
        return;
 }
 bool playerFound = false;
 QHash<QListWidgetItem*, KPlayer*>::const_iterator it, itEnd;
 it = d->mItem2Player.constBegin();
 itEnd = d->mItem2Player.constEnd();
 for ( ; !playerFound && it != itEnd; ++it ) playerFound = it.value() == p;
 if (playerFound) {
	kError(11001) << ": attempt to double add player";
	return;
 }
 kDebug(11001) << ": add player" << p->id();
 QListWidgetItem* t = new QListWidgetItem(p->name(), d->mPlayerBox);
 d->mItem2Player.insert(t, p);

 connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*)));

}

void KGameDialogConnectionConfig::slotPlayerLeftGame(KPlayer* p)
{
 // disconnect first
 this->disconnect(p);
 if (!item(p)) {
	kError(11001) << ": cannot find" << p->id()
			<< "in list";
	return;
 }
 d->mPlayerBox->takeItem(d->mPlayerBox->row(item(p)));

}

void KGameDialogConnectionConfig::slotKickPlayerOut(QListWidgetItem* item)
{
 kDebug(11001) << "kick player out";
 KPlayer* p = d->mItem2Player[item];
 if (!p) {
	kError(11001) << "invalid item selected - no player found";
	return;
 }
 if (!game()) {
	kWarning(11001) << "no game set";
	return;
 }
 if (!admin()) {
	kDebug(11001) << "Only the ADMIN can kick players";
	return;
 }
 if (p == owner()) { // you wanna ban the ADMIN ??
	kDebug(11001) << "you cannot kick the ADMIN";
	return;
 }

 if (KMessageBox::questionYesNo(this, i18n("Do you want to ban player \"%1\" from the game?",
		p->name()), QString(), KGuiItem (i18n("Ban Player")), KGuiItem (i18n("Do Not Ban"))) == KMessageBox::Yes) {
	kDebug(11001) << "will remove player" << p;
	game()->removePlayer(p);
//	d->mPlayerBox->removeItem(d->mPlayerBox->index(item)); // should be done by signalPlayerLeftGame
 } else {
	kDebug(11001) << "will NOT remove player" << p;
 }
}

void KGameDialogConnectionConfig::slotPropertyChanged(KGamePropertyBase* prop, KPlayer* player)
{
 if(prop->id() == KGamePropertyBase::IdName) {
	QListWidgetItem* old = item(player);
	QListWidgetItem* t = new QListWidgetItem(player->name());
	int row = d->mPlayerBox->row(old);
	d->mPlayerBox->takeItem( row );
	d->mPlayerBox->insertItem(row, t);
	d->mItem2Player.remove(old);
	d->mItem2Player.insert(t, player);
 }
}

