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
#include "kgameproperty.h"
#include "kgamedialog.h"


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
 if (p) {
	p->setName(playerName());
 }
 if (g) {
 return;
	if (g->isAdmin()) {
kdDebug(11001) << "setmax player" << endl;
		g->setMaxPlayers(maxPlayers());
kdDebug(11001) << "setmin player" << endl;
		g->setMinPlayers(minPlayers());
kdDebug(11001) << "setmin player done" << endl;
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

