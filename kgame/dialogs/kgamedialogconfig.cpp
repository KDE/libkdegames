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

	}

	QPushButton* mInitConnection;
	QLabel* mNetworkLabel;

	QString mDefaultHost;
	unsigned short int mDefaultPort;
};


KGameDialogNetworkConfig::KGameDialogNetworkConfig(QWidget* parent) 
		: KGameDialogConfig(parent)
{
// kdDebug(11001) << "CONSTRUCT KGameDialogNetworkConfig " << this << endl;
 d = new KGameDialogNetworkConfigPrivate();

 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

 d->mNetworkLabel = new QLabel(this);
 topLayout->addWidget(d->mNetworkLabel);

 d->mInitConnection = new QPushButton(this);
 d->mInitConnection->setText(i18n("Start Network game"));
 connect(d->mInitConnection, SIGNAL(clicked()), this, SLOT(slotInitConnection()));
 topLayout->addWidget(d->mInitConnection);
 setConnected(false);
 setDefaultNetworkInfo("localhost", 0);
}

KGameDialogNetworkConfig::~KGameDialogNetworkConfig()
{ delete d; }

void KGameDialogNetworkConfig::slotInitConnection()
{
 bool connected = false;
 bool master = true;
 unsigned short int port = d->mDefaultPort;
 QString host = d->mDefaultHost;
 int result = KGameConnectDialog::initConnection(port, host, this, true);
 if (result != QDialog::Accepted) {
	connected = false;
 } else {
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
	}
 }

 setConnected(connected, master);
}

void KGameDialogNetworkConfig::setConnected(bool connected, bool master)
{
 if (!connected) {
	d->mNetworkLabel->setText(i18n("No Network"));
	d->mInitConnection->setEnabled(true);
	return;
 }
 if (master) {
	d->mNetworkLabel->setText(i18n("You are MASTER"));
 } else {
	d->mNetworkLabel->setText(i18n("You are connected"));
 }
 d->mInitConnection->setEnabled(false);
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

void KGameDialogNetworkConfig::setDefaultNetworkInfo(const QString& host, unsigned short int port)
{
 d->mDefaultPort = port;
 d->mDefaultHost = host;
}

/////////////////////////// KGameDialogGeneralConfig /////////////////////////
class KGameDialogGeneralConfigPrivate
{
public:
	KGameDialogGeneralConfigPrivate() 
	{
		mMaxPlayers = 0;
		mMinPlayers = 0;
		mName = 0;
	}
	
	KIntNumInput* mMaxPlayers;
	KIntNumInput* mMinPlayers;
	QLineEdit* mName;

	QGridLayout* mTopLayout; //TODO
};

KGameDialogGeneralConfig::KGameDialogGeneralConfig(QWidget* parent) 
		: KGameDialogConfig(parent)
{
// kdDebug(11001) << "CONSTRUCT KGameDialogGeneralConfig " << this << endl;
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
void KGameDialogGeneralConfig::setMinPlayers(int m)
{ d->mMinPlayers->setValue(m); }
int KGameDialogGeneralConfig::minPlayers() const
{ return d->mMinPlayers->value(); }
int KGameDialogGeneralConfig::maxPlayers() const
{ return d->mMaxPlayers->value(); }
void KGameDialogGeneralConfig::setPlayerName(const QString& name)
{ d->mName->setText(name); }
QString KGameDialogGeneralConfig::playerName() const
{ return d->mName->text(); }
QGridLayout* KGameDialogGeneralConfig::layout() const
{ return d->mTopLayout; }

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
 setMaxPlayers(g->maxPlayers());
 setMinPlayers(g->minPlayers());
}

void KGameDialogGeneralConfig::setAdmin(bool admin)
{
 KGameDialogConfig::setAdmin(admin);
 d->mMaxPlayers->setEnabled(admin);
 d->mMinPlayers->setEnabled(admin);
}

void KGameDialogGeneralConfig::submitToKGame(KGame* g, KPlayer* p)
{
//FIXME
 if (p) {
	p->setName(playerName());
 }
 if (g) {
 return;
	if (g->isAdmin()) {
		g->setMaxPlayers(maxPlayers());
		g->setMinPlayers(minPlayers());
	}
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
	kdError(11001) << "no valid game object available!" << endl;
	return;
 }
 if (!game()->isAdmin()) {
	kdError(11001) << "changeMaxClients(): only ADMIN is allowed to call this!" << endl;
	return;
 }
 int max;
// edit->setText(QString::number()); // current max clients! //TODO

 QDialog* dialog = new QDialog();
 dialog->setCaption(i18n("Maximal number of clients"));
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
	kdError(11001) << "no valid game object available!" << endl;
	return;
 }
 if (!admin()) {
	kdError(11001) << "changeAdmin(): only ADMIN is allowed to call this!" << endl;
	return;
 }
 Q_UINT32 newAdmin;
// newAdmin = ;
 game()->electAdmin(newAdmin);
}

void KGameDialogMsgServerConfig::removeClient(Q_UINT32 id)
{
//TODO
}

void KGameDialogMsgServerConfig::setAdmin(bool a)
{
 KGameDialogConfig::setAdmin(a);
 if (admin()) {
	if (d->noAdmin) {
		delete d->noAdmin;
		d->noAdmin = 0;
	}
	d->changeMaxClients = new QPushButton(i18n("Change Maximal number of Clients"), this);
	connect(d->changeMaxClients, SIGNAL(pressed()), this, SLOT(slotChangeMaxClients()));
	d->changeAdmin = new QPushButton(i18n("Change Admin"), this);
	connect(d->changeAdmin, SIGNAL(pressed()), this, SLOT(slotChangeAdmin()));
	d->removeClient = new QPushButton(i18n("Remove a Client with all Players"), this);
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
		mPlayers = 0;
	}

	QPtrDict<KPlayer> mItem2Player;
	KListBox* mPlayers;
};

KGameDialogConnectionConfig::KGameDialogConnectionConfig(QWidget* parent)
		: KGameDialogConfig(parent)
{
 //TODO: prevent player to ban himself 
 d = new KGameDialogConnectionConfigPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
 topLayout->setAutoAdd(true);
 QHGroupBox* b = new QHGroupBox(i18n("Connected Players"), this);
 d->mPlayers = new KListBox(b);

}

KGameDialogConnectionConfig::~KGameDialogConnectionConfig()
{
 delete d;
}

void KGameDialogConnectionConfig::setKGame(KGame* g)
{
 if (game()) {
	disconnect(game(), 0, this, 0);
 }
 KGameDialogConfig::setKGame(g);
 if (game()) {
// react to changes in KGame::playerList()
	connect(game(), SIGNAL(signalPlayerJoinedGame(KPlayer*)),
			this, SLOT(slotPlayerChanged(KPlayer*)));
	connect(game(), SIGNAL(signalPlayerLeftGame(KPlayer*)),
			this, SLOT(slotPlayerChanged(KPlayer*)));
 }
 slotPlayerChanged(0);
}

void KGameDialogConnectionConfig::setOwner(KPlayer* p)
{
 KGameDialogConfig::setOwner(p);
}

void KGameDialogConnectionConfig::setAdmin(bool a)
{
 KGameDialogConfig::setAdmin(a);
 if (!game()) {
	return;
 }
 disconnect(game(), SIGNAL(executed(QListBoxItem*)), this, 0);
 if (admin()) {
	connect(d->mPlayers, SIGNAL(executed(QListBoxItem*)), this,
			SLOT(slotKickPlayerOut(QListBoxItem*)));
 }
}

void KGameDialogConnectionConfig::slotPlayerChanged(KPlayer* )
{
 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	// disconnect everything first
	this->disconnect(it.current());
	++it;
 }
 d->mItem2Player.clear();
 d->mPlayers->clear();

 if (!game()) {
	return;
 }

 KGame::KGamePlayerList l = *game()->playerList();
 for (KPlayer* p = l.first(); p; p = l.next()) {
	QListBoxText* t = new QListBoxText(p->name());
	d->mItem2Player.insert(t, p);
	d->mPlayers->insertItem(t);

	connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)), 
			this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*)));
 }
}

void KGameDialogConnectionConfig::slotKickPlayerOut(QListBoxItem* item)
{
 KPlayer* p = d->mItem2Player[item];
 if (!game()|| !admin() || !p) {
	return;
 }

 if (p == owner()) { // you wanna ban the ADMIN ??
	return;
 }
		       
 if (KMessageBox::questionYesNo(this, i18n("Do you want to ban player \"%1\" from the game?").arg(
		p->name())) == KMessageBox::Yes) {
	kdDebug(11001) << "will remove player " << p << endl;
	game()->removePlayer(p);
	d->mPlayers->removeItem(d->mPlayers->index(item));
	slotPlayerChanged(p);
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
	d->mPlayers->changeItem(t,
	d->mPlayers->index(old));
	d->mItem2Player.remove(old);
	d->mItem2Player.insert(t, player);
 }
}

