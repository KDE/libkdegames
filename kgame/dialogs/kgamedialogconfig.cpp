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
//#include <qhgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qvbox.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdialog.h>

#include "kgame.h"
#include "kplayer.h"

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
		changeMaxClients = 0;
		changeAdmin= 0;
		removeClient= 0;
	}

	QPushButton* changeMaxClients;
	QPushButton* changeAdmin;
	QPushButton* removeClient;

	QLabel* noAdmin;

	KGame* game;

};


// TODO: change ADMIN ID, remove CLIENTS, change MAXCLIENTS
// we do everything here with QPushButtons as we want to wait a moment before
// continuing - the message must be sent over network first
KGameDialogMsgServerConfig::KGameDialogMsgServerConfig(QWidget* parent) 
		: KGameDialogConfig(parent)
{
 d = new KGameDialogMsgServerConfigPrivate;

 QVBoxLayout* layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
 layout->setAutoAdd(true);
}

KGameDialogMsgServerConfig::~KGameDialogMsgServerConfig()
{
 delete d;
}


void KGameDialogMsgServerConfig::setKGame(KGame* g)
{
 d->game = g;
 //TODO display the ID of the admin if we aren't:
 // connect(g, SIGNAL(signalAdminChanged(int)), this, SLOT(slotChangeIsAdmin(int)));
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
	delete d->noAdmin;
	d->changeMaxClients = new QPushButton(i18n("Change Maximal number of Clients"), this);
	connect(d->changeMaxClients, SIGNAL(pressed()), this, SLOT(changeMaxClients()));
	d->changeAdmin = new QPushButton(i18n("Change Admin"), this);
	connect(d->changeAdmin, SIGNAL(pressed()), this, SLOT(changeAdmin()));
	d->removeClient = new QPushButton(i18n("Remove a Client with all Players"), this);
	connect(d->removeClient, SIGNAL(pressed()), this, SLOT(removeClient()));
 } else {
	delete d->changeMaxClients;
	delete d->changeAdmin;
	delete d->removeClient;
	d->noAdmin = new QLabel(i18n("Only the admin can configure the message server!"), this);
 }
}


