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
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qptrdict.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdialog.h>
#include <klistbox.h>
#include <kmessagebox.h>

#include "kgame.h"
#include "kplayer.h"
#include "kgamechat.h"

#include "kgamedialogconfig.h"

#include "kgamedialogconfig.moc"

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

void KGameDialogConfig::setAdmin(bool )
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
// kdDebug(11001) << "CONSTRUCT KGameDialogNetworkConfig " << this << endl;

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

void KGameDialogNetworkConfig::submitToKGame(KGame* g, KPlayer* )
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
{ d->mMaxPlayers = (KIntNumInput *)m; }
void KGameDialogGeneralConfig::setPlayerName(const QString& name)
{ d->mName->setText(name); }
void KGameDialogGeneralConfig::setMinPlayers(int m)
{ d->mMinPlayers = (KIntNumInput *)m; }
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

void KGameDialogGeneralConfig::setAdmin(bool admin)
{
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

		game = 0;
	}

	QHBoxLayout* senderLayout;
	QHBoxLayout* localLayout;

	QPushButton* changeMaxClients;
	QPushButton* changeAdmin;
	QPushButton* removeClient;
	QLabel* noAdmin;

	QLabel* noMaster;

	KGame* game;

};


// TODO: change ADMIN ID, remove CLIENTS, change MAXCLIENTS
// we do everything here with QPushButtons as we want to wait a moment before
// continuing - the message must be sent over network first
KGameDialogMsgServerConfig::KGameDialogMsgServerConfig(QWidget* parent) 
		: KGameDialogConfig(parent)
{
 d = new KGameDialogMsgServerConfigPrivate;

 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
 d->senderLayout = new QHBoxLayout(topLayout);
 d->localLayout = new QHBoxLayout(topLayout);
}

KGameDialogMsgServerConfig::~KGameDialogMsgServerConfig()
{
 delete d;
}

void KGameDialogMsgServerConfig::setKGame(KGame* g)
{
 d->game = g;
 //TODO display the ID of the admin if we aren't
 // connect(g, SIGNAL(signalAdminChanged(int)), this, SLOT(slotChangeIsAdmin(int)));//TODO
 if (!g) {
	// we cannot do anything without a KGame object!
	setAdmin(false);
	return;
 }
 setAdmin(g->isAdmin());
 setHasMsgServer(d->game->messageServer());
}


void KGameDialogMsgServerConfig::changeMaxClients()
{
 if (!d->game) {
	kdError(11001) << "no valid game object available!" << endl;
	return;
 }
 if (!d->game->isAdmin()) {
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
		d->game->setMaxClients(max);
	}
 }

}

void KGameDialogMsgServerConfig::removeClient()
{
}

void KGameDialogMsgServerConfig::changeAdmin()
{
 if (!d->game) {
	kdError(11001) << "no valid game object available!" << endl;
	return;
 }
 if (!d->game->isAdmin()) {
	kdError(11001) << "changeAdmin(): only ADMIN is allowed to call this!" << endl;
	return;
 }
 Q_UINT32 newAdmin;
// newAdmin = ;
 d->game->electAdmin(newAdmin);
}

void KGameDialogMsgServerConfig::removeClient(Q_UINT32 id)
{
//TODO
}

void KGameDialogMsgServerConfig::setAdmin(bool admin)
{
 if (admin) {
	if (d->noAdmin) {
		delete d->noAdmin;
		d->noAdmin = 0;
	}
	d->changeMaxClients = new QPushButton(i18n("Change Maximal number of Clients"), this);
	connect(d->changeMaxClients, SIGNAL(pressed()), this, SLOT(changeMaxClients()));
	d->changeAdmin = new QPushButton(i18n("Change Admin"), this);
	connect(d->changeAdmin, SIGNAL(pressed()), this, SLOT(changeAdmin()));
	d->removeClient = new QPushButton(i18n("Remove a Client with all Players"), this);
	connect(d->removeClient, SIGNAL(pressed()), this, SLOT(removeClient()));
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


void KGameDialogMsgServerConfig::submitToKGame(KGame*, KPlayer*)
{
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

void KGameDialogChatConfig::setKGame(KGame* game)
{
 d->mChat->setKGame(game);
 if (!game) {
	hide();
 } else {
	show();
 }
}

void KGameDialogChatConfig::setOwner(KPlayer* owner) 
{
 if (!owner) {
	hide();
	return;
 }
 d->mChat->setFromPlayer(owner);
 show();
}



class KGameDialogConnectionConfigPrivate
{
public:
	KGameDialogConnectionConfigPrivate()
	{
		mPlayers = 0;
		mGame = 0;
		mOwner = 0;
	}

	QPtrDict<KPlayer> mItem2Player;
	KListBox* mPlayers;
	KGame* mGame;
	KPlayer* mOwner;
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
 if (d->mGame) {
	disconnect(d->mGame, 0, this, 0);
 }
 d->mGame = g;
 if (d->mGame) {
// react to changes in KGame::playerList()
	connect(d->mGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
			this, SLOT(slotPlayerChanged(KPlayer*)));
	connect(d->mGame, SIGNAL(signalPlayerLeftGame(KPlayer*)),
			this, SLOT(slotPlayerChanged(KPlayer*)));
 }
 slotPlayerChanged(0);
}

void KGameDialogConnectionConfig::setOwner(KPlayer* p)
{
 d->mOwner = p;
}

void KGameDialogConnectionConfig::setAdmin(bool admin)
{
 if (!d->mGame) {
	return;
 }
 disconnect(d->mGame, SIGNAL(executed(QListBoxItem*)), this, 0);
 if (admin) {
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

 if (!d->mGame) {
	return;
 }

 KGame::KGamePlayerList l = *d->mGame->playerList();
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
 if (!d->mGame || !d->mGame->isAdmin() || !p) {
	return;
 }

 if (p == d->mOwner) { // you wanna ban the ADMIN ??
	return;
 }
		       
 if (KMessageBox::questionYesNo(this, i18n("Do you want to ban player \"%1\" from the game?").arg(
		p->name())) == KMessageBox::Yes) {
	kdDebug(11001) << "will remove player " << p << endl;
	d->mGame->removePlayer(p);
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

