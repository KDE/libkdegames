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
#include <qstring.h>
#include <klistview.h>
#include <klistbox.h>

//#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include "kgame.h"
#include "kplayer.h"

#include "kgamedebugdialog.h"

class KGameDebugDialogPrivate
{
public:
	KGameDebugDialogPrivate()
	{
		mGame = 0;

		mGamePage = 0;
		mGameProperties = 0;
		mGameAddress = 0;
		mGameId = 0;
		mGameCookie = 0;
		mGameMaster = 0;
		mGameAdmin = 0;
		mGameOffering = 0;
		mGameStatus = 0;
		mGameRunning = 0;
		mGameMaxPlayers = 0;
		mGameMinPlayers = 0;
		mGamePlayerCount = 0;

		mPlayerPage = 0;
		mPlayerList = 0;
		mPlayerProperties = 0;
		mPlayerAddress = 0;
		mPlayerId = 0;
		mPlayerName = 0;
		mPlayerGroup = 0;
		mPlayerUserId = 0;
		mPlayerMyTurn = 0;
		mPlayerAsyncInput= 0;
		mPlayerKGameAddress = 0;
		mPlayerVirtual = 0;
		mPlayerActive = 0;
		mPlayerRtti = 0;
		mPlayerNetworkPriority = 0;
	}

	const KGame* mGame;

	QFrame* mGamePage;
	KListView* mGameProperties;
	QListViewItem* mGameAddress;
	QListViewItem* mGameId;
	QListViewItem* mGameCookie;
	QListViewItem* mGameMaster;
	QListViewItem* mGameAdmin;
	QListViewItem* mGameOffering;
	QListViewItem* mGameStatus;
	QListViewItem* mGameRunning;
	QListViewItem* mGameMaxPlayers;
	QListViewItem* mGameMinPlayers;
	QListViewItem* mGamePlayerCount;
	
	QFrame* mPlayerPage;
	KListBox* mPlayerList;
	KListView* mPlayerProperties;
	QListViewItem* mPlayerAddress;
	QListViewItem* mPlayerId;
	QListViewItem* mPlayerName;
	QListViewItem* mPlayerGroup;
	QListViewItem* mPlayerUserId;
	QListViewItem* mPlayerMyTurn;
	QListViewItem* mPlayerAsyncInput;
	QListViewItem* mPlayerKGameAddress;
	QListViewItem* mPlayerVirtual;
	QListViewItem* mPlayerActive;
	QListViewItem* mPlayerRtti;
	QListViewItem* mPlayerNetworkPriority;
};

KGameDebugDialog::KGameDebugDialog(KGame* g, QWidget* parent, bool modal) :
		KDialogBase(Tabbed, i18n("KGame Debug Dialog"), Close, Close, 
		parent, 0, modal, true)
{
 d = new KGameDebugDialogPrivate;

 initGamePage();
 initPlayerPage();

 setKGame(g);
}

KGameDebugDialog::~KGameDebugDialog()
{
 delete d;
}

void KGameDebugDialog::initGamePage()
{
 d->mGamePage = addPage(i18n("Debug KGame"));
 QVBoxLayout* layout = new QVBoxLayout(d->mGamePage, marginHint(), spacingHint());
 QHBoxLayout* l = new QHBoxLayout(layout);

 KListView* v = new KListView(d->mGamePage);
 v->addColumn(i18n("Data"));
 v->addColumn(i18n("Value"));
 l->addWidget(v);

 d->mGameProperties = new KListView(d->mGamePage);
 d->mGameProperties->addColumn(i18n("Property"));
 d->mGameProperties->addColumn(i18n("Value"));
 l->addWidget(d->mGameProperties);
 
 QPushButton* b = new QPushButton(i18n("Update"), d->mGamePage);
 connect(b, SIGNAL(pressed()), this, SLOT(updateGameData()));
 layout->addWidget(v);

// game data
 d->mGameAddress = new QListViewItem(v, i18n("KGame Pointer"));
 d->mGameId = new QListViewItem(v, i18n("Game ID"));
 d->mGameCookie = new QListViewItem(v, i18n("Game Cookie"));
 d->mGameMaster = new QListViewItem(v, i18n("Is Master"));
 d->mGameAdmin = new QListViewItem(v, i18n("Is Admin"));
 d->mGameOffering = new QListViewItem(v, i18n("Is Offering Connections"));
 d->mGameStatus = new QListViewItem(v, i18n("Game Status"));
 d->mGameRunning = new QListViewItem(v, i18n("Game is Running"));
 d->mGameMaxPlayers = new QListViewItem(v, i18n("Maximal Players"));
 d->mGameMinPlayers = new QListViewItem(v, i18n("Minimal Players"));
 d->mGamePlayerCount = new QListViewItem(v, i18n("Players"));
}

void KGameDebugDialog::initPlayerPage()
{
 d->mPlayerPage = addPage(i18n("Debug Players"));
 QVBoxLayout* topLayout = new QVBoxLayout(d->mPlayerPage, marginHint(), spacingHint());
 QHBoxLayout* layout = new QHBoxLayout(topLayout);

 //TODO: connect to the KGame signals for joined/removed players!!!
 d->mPlayerList = new KListBox(d->mPlayerPage);
 connect(d->mPlayerList, SIGNAL(executed(QListBoxItem*)), this, SLOT(updatePlayerData(QListBoxItem*)));
 layout->addWidget(d->mPlayerList);

 d->mPlayerProperties = new KListView(d->mPlayerPage);
 d->mPlayerProperties->addColumn(i18n("Property"));
 d->mPlayerProperties->addColumn(i18n("Value"));
 layout->addWidget(d->mPlayerProperties);
 
 QPushButton* b = new QPushButton(i18n("Update"), d->mPlayerPage);
 connect(b, SIGNAL(pressed()), this, SLOT(updatePlayerList()));
 topLayout->addWidget(b);

 KListView* v = new KListView(d->mPlayerPage);
 layout->addWidget(v);
 v->addColumn(i18n("Property"));
 v->addColumn(i18n("Value"));
 
 d->mPlayerAddress = new QListViewItem(v, i18n("Player Pointer"));
 d->mPlayerId = new QListViewItem(v, i18n("Player ID"));
 d->mPlayerName = new QListViewItem(v, i18n("Player Name"));
 d->mPlayerGroup = new QListViewItem(v, i18n("Player Group"));
 d->mPlayerUserId = new QListViewItem(v, i18n("Player User ID"));
 d->mPlayerMyTurn = new QListViewItem(v, i18n("My Turn"));
 d->mPlayerAsyncInput = new QListViewItem(v, i18n("Async Input"));
 d->mPlayerKGameAddress = new QListViewItem(v, i18n("Player's KGame Object Address"));
 d->mPlayerVirtual = new QListViewItem(v, i18n("Player is Virtual"));
 d->mPlayerActive = new QListViewItem(v, i18n("Player is Active"));
 d->mPlayerRtti = new QListViewItem(v, i18n("RTTI"));
 d->mPlayerNetworkPriority = new QListViewItem(v, i18n("Network Priority"));
}

void KGameDebugDialog::clearPlayerData()
{
 d->mPlayerAddress->setText(1, "");
 d->mPlayerId->setText(1, "");
 d->mPlayerName->setText(1, "");
 d->mPlayerGroup->setText(1, "");
 d->mPlayerUserId->setText(1, "");
 d->mPlayerMyTurn->setText(1, "");
 d->mPlayerAsyncInput->setText(1, "");
 d->mPlayerKGameAddress->setText(1, "");
 d->mPlayerVirtual->setText(1, "");
 d->mPlayerActive->setText(1, "");
 d->mPlayerRtti->setText(1, "");
 d->mPlayerNetworkPriority->setText(1, "");

 d->mPlayerProperties->clear();
}

void KGameDebugDialog::clearGameData()
{
 d->mGameAddress->setText(1, "");
 d->mGameId->setText(1, "");
 d->mGameCookie->setText(1, "");
 d->mGameMaster->setText(1, "");
 d->mGameAdmin->setText(1, "");
 d->mGameOffering->setText(1, "");
 d->mGameStatus->setText(1, "");
 d->mGameRunning->setText(1, "");
 d->mGameMaxPlayers->setText(1, "");
 d->mGameMinPlayers->setText(1, "");

 d->mGameProperties->clear();
}

void KGameDebugDialog::updatePlayerData()
{
 if (!d->mGame || d->mPlayerList->currentItem() == -1) {
	return;
 }
 updatePlayerData(d->mPlayerList->item(d->mPlayerList->currentItem()));
}

void KGameDebugDialog::updatePlayerList()
{
 QListBoxItem* i = d->mPlayerList->firstItem(); 
 for (; i; i = d->mPlayerList->firstItem()) {
	removePlayer(i);
 }

 QList<KPlayer> list = *d->mGame->playerList();
 for (KPlayer* p = list.first(); p; p = list.next()) {
	addPlayer(p);
 }
}

void KGameDebugDialog::updateGameData()
{
 if (!d->mGame) {
	d->mGameAddress->setText(1, i18n("NULL pointer"));
	return;
 }
 QString buf;
 buf.sprintf("%p", d->mGame);
 d->mGameAddress->setText(1, buf);
 d->mGameId->setText(1, QString::number(d->mGame->gameId()));
 d->mGameCookie->setText(1, QString::number(d->mGame->cookie()));
 d->mGameMaster->setText(1, d->mGame->isMaster() ? i18n("True") : i18n("False"));
 d->mGameAdmin->setText(1, d->mGame->isAdmin() ? i18n("True") : i18n("False"));
 d->mGameOffering->setText(1, d->mGame->isOfferingConnections() ? i18n("True") : i18n("False"));
 d->mGameStatus->setText(1, QString::number(d->mGame->gameStatus()));
 d->mGameRunning->setText(1, d->mGame->isRunning() ? i18n("True") : i18n("False"));
 d->mGameMaxPlayers->setText(1, QString::number(d->mGame->maxPlayers()));
 d->mGameMinPlayers->setText(1, QString::number(d->mGame->minPlayers()));
 d->mGamePlayerCount->setText(1, QString::number(d->mGame->playerCount()));

// Properties
// KGamePropertyHandlerBase* b = d->mGame->dataHandler();

//TODO ios
//TODO properties?
}

void KGameDebugDialog::updatePlayerData(QListBoxItem* item)
{
 if (!item || !d->mGame) {
	return;
 }

 KPlayer* p = d->mGame->findPlayer(item->text().toInt());

 if (!p) {
	kdError(11001) << "KGameDebugDialog: cannot find player" << endl;
	return;
 }

 clearPlayerData();

 QString buf;
 buf.sprintf("%p", p);
 d->mPlayerAddress->setText(1, buf);
 d->mPlayerId->setText(1, QString::number(p->id()));
 d->mPlayerName->setText(1, p->name());
 d->mPlayerGroup->setText(1, p->group());
 d->mPlayerUserId->setText(1, QString::number(p->userId()));
 d->mPlayerMyTurn->setText(1, p->myTurn() ? i18n("True") : i18n("False"));
 d->mPlayerAsyncInput->setText(1, p->asyncInput() ? i18n("True") : i18n("False"));
 buf.sprintf("%p", p->game());
 d->mPlayerKGameAddress->setText(1, buf);
 d->mPlayerVirtual->setText(1, p->isVirtual() ? i18n("True") : i18n("False"));
 d->mPlayerActive->setText(1, p->isActive() ? i18n("True") : i18n("False"));
 d->mPlayerRtti->setText(1, QString::number(p->rtti()));
 d->mPlayerNetworkPriority->setText(1, QString::number(p->networkPriority()));
//TODO ios

// Properties
 KGamePropertyHandlerBase* handler = p->dataHandler();
 QIntDictIterator<KGamePropertyBase> it(*handler);
 while (it.current()) {
	QListViewItem* prop = new QListViewItem(d->mPlayerProperties,
			i18n(propertyName(it.current())),
			propertyValue(it.current()));
 }
}

void KGameDebugDialog::clearPages()
{
 clearPlayerData();
 clearGameData();
 d->mPlayerList->clear();
}

void KGameDebugDialog::setKGame(const KGame* g)
{
 slotUnsetKGame();
 d->mGame = g;
 if (g) {
 //TODO: connect to the KGame signals for joined/removed players!!!
	connect(d->mGame, SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));
//	connect();

	QList<KPlayer> list = *d->mGame->playerList();
	for (KPlayer* p = list.first(); p; p = list.next()) {
		addPlayer(p);
	}

	updateGameData();
 }
}

void KGameDebugDialog::slotUnsetKGame()
{
 if (d->mGame) {
	disconnect(d->mGame, 0, this, 0);
 }
 d->mGame = 0;
 clearPages();
}

void KGameDebugDialog::addPlayer(KPlayer* p)
{
 if (!p) {
	kdError(11001) << "trying to add NULL player" << endl;
	return;
 }

 (void) new QListBoxText(d->mPlayerList, QString::number(p->id()));
 //TODO connect to signals, like deleted/removed, ...
}

void KGameDebugDialog::removePlayer(QListBoxItem* i)
{
 if (!i || !d->mGame) {
	return;
 }
 KPlayer* p = d->mGame->findPlayer(i->text().toInt());
 if (!p) {
	return;
 }
 disconnect(p, 0, this, 0);
 if (i->selected()) {
	clearPlayerData();
 }
 delete i;
}

QString KGameDebugDialog::propertyName(KGamePropertyBase* prop) const
{
 if (!prop) {
	return i18n("NULL pointer");
 }

 switch (prop->id()) {
	case KGamePropertyBase::IdGroup:
		return i18n("Group");
	case KGamePropertyBase::IdName:
		return i18n("Name");
	case KGamePropertyBase::IdAsyncInput:
		return i18n("AsyncInput");
	case KGamePropertyBase::IdTurn:
		return i18n("Turn");
	case KGamePropertyBase::IdUserId:
		return i18n("User Id");
	case KGamePropertyBase::IdGameStatus:
		return i18n("Game Status");
	case KGamePropertyBase::IdMaxPlayer:
		return i18n("Max Players");
	case KGamePropertyBase::IdMinPlayer:
		return i18n("Min Players");
	default:
		return i18n("User Property");//FIXME should display a name, too
 }

}

QString KGameDebugDialog::propertyValue(KGamePropertyBase* prop) const
{
 if (!prop) {
	return i18n("NULL pointer");
 }
 //TODO

}

#include "kgamedebugdialog.moc"
