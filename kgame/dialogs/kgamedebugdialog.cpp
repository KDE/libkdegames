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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgamedebugdialog.h"

#include "kgamemessage.h"
#include "kgame.h"
#include "kplayer.h"
#include "kgamepropertyhandler.h"

#include <k3listview.h>
#include <klistbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include <qlayout.h>
#include <qstring.h>
#include <q3intdict.h>
#include <qlabel.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3Frame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <Q3PtrList>

#include <typeinfo>


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

		mMessagePage = 0;
		mMessageList = 0;
		mHideIdList = 0;
	}

	const KGame* mGame;

	QFrame* mGamePage;
	K3ListView* mGameProperties;
	Q3ListViewItem* mGameAddress;
	Q3ListViewItem* mGameId;
	Q3ListViewItem* mGameCookie;
	Q3ListViewItem* mGameMaster;
	Q3ListViewItem* mGameAdmin;
	Q3ListViewItem* mGameOffering;
	Q3ListViewItem* mGameStatus;
	Q3ListViewItem* mGameRunning;
	Q3ListViewItem* mGameMaxPlayers;
	Q3ListViewItem* mGameMinPlayers;
	Q3ListViewItem* mGamePlayerCount;
	
	QFrame* mPlayerPage;
	KListBox* mPlayerList;
	K3ListView* mPlayerProperties;
	Q3ListViewItem* mPlayerAddress;
	Q3ListViewItem* mPlayerId;
	Q3ListViewItem* mPlayerName;
	Q3ListViewItem* mPlayerGroup;
	Q3ListViewItem* mPlayerUserId;
	Q3ListViewItem* mPlayerMyTurn;
	Q3ListViewItem* mPlayerAsyncInput;
	Q3ListViewItem* mPlayerKGameAddress;
	Q3ListViewItem* mPlayerVirtual;
	Q3ListViewItem* mPlayerActive;
	Q3ListViewItem* mPlayerRtti;
	Q3ListViewItem* mPlayerNetworkPriority;

	QFrame* mMessagePage;
	K3ListView* mMessageList;
	KListBox* mHideIdList;
};

KGameDebugDialog::KGameDebugDialog(KGame* g, QWidget* parent, bool modal) :
		KDialogBase(Tabbed, i18n("KGame Debug Dialog"), Close, Close, 
		parent, 0, modal, true)
{
 d = new KGameDebugDialogPrivate;

 initGamePage();
 initPlayerPage();
 initMessagePage();

 setKGame(g);
}

KGameDebugDialog::~KGameDebugDialog()
{
 delete d;
}

void KGameDebugDialog::initGamePage()
{
 d->mGamePage = addPage(i18n("Debug &KGame"));
 QVBoxLayout* topLayout = new QVBoxLayout(d->mGamePage);
 topLayout->setMargin( marginHint() );
 topLayout->setSpacing( spacingHint() );
 QHBoxLayout* layout = new QHBoxLayout;
 topLayout->addLayout(layout);

 K3ListView* v = new K3ListView(d->mGamePage);
 v->addColumn(i18n("Data"));
 v->addColumn(i18n("Value"));
 layout->addWidget(v);

 d->mGameProperties = new K3ListView(d->mGamePage);
 d->mGameProperties->addColumn(i18n("Property"));
 d->mGameProperties->addColumn(i18n("Value"));
 d->mGameProperties->addColumn(i18n("Policy"));
 layout->addWidget(d->mGameProperties);
 
 QPushButton* b = new QPushButton(i18n("Update"), d->mGamePage);
 connect(b, SIGNAL(pressed()), this, SLOT(slotUpdateGameData()));
 topLayout->addWidget(b);

// game data
 d->mGameAddress = new Q3ListViewItem(v, i18n("KGame Pointer"));
 d->mGameId = new Q3ListViewItem(v, i18n("Game ID"));
 d->mGameCookie = new Q3ListViewItem(v, i18n("Game Cookie"));
 d->mGameMaster = new Q3ListViewItem(v, i18n("Is Master"));
 d->mGameAdmin = new Q3ListViewItem(v, i18n("Is Admin"));
 d->mGameOffering = new Q3ListViewItem(v, i18n("Is Offering Connections"));
 d->mGameStatus = new Q3ListViewItem(v, i18n("Game Status"));
 d->mGameRunning = new Q3ListViewItem(v, i18n("Game is Running"));
 d->mGameMaxPlayers = new Q3ListViewItem(v, i18n("Maximal Players"));
 d->mGameMinPlayers = new Q3ListViewItem(v, i18n("Minimal Players"));
 d->mGamePlayerCount = new Q3ListViewItem(v, i18n("Players"));
}

void KGameDebugDialog::initPlayerPage()
{
 d->mPlayerPage = addPage(i18n("Debug &Players"));
 QVBoxLayout* topLayout = new QVBoxLayout(d->mPlayerPage);
 topLayout->setMargin( marginHint() );
 topLayout->setSpacing( spacingHint() );
 QHBoxLayout* layout = new QHBoxLayout;
 topLayout->addLayout(layout);

 //TODO: connect to the KGame signals for joined/removed players!!!
 QVBoxLayout* listLayout = new QVBoxLayout;
 layout->addLayout(listLayout);
 QLabel* listLabel = new QLabel(i18n("Available Players"), d->mPlayerPage);
 listLayout->addWidget(listLabel);
 d->mPlayerList = new KListBox(d->mPlayerPage);
 connect(d->mPlayerList, SIGNAL(executed(Q3ListBoxItem*)), this, SLOT(slotUpdatePlayerData(Q3ListBoxItem*)));
 listLayout->addWidget(d->mPlayerList);
 d->mPlayerList->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));

 K3ListView* v = new K3ListView(d->mPlayerPage);
 layout->addWidget(v);
 v->addColumn(i18n("Data"));
 v->addColumn(i18n("Value"));
 
 d->mPlayerProperties = new K3ListView(d->mPlayerPage);
 d->mPlayerProperties->addColumn(i18n("Property"));
 d->mPlayerProperties->addColumn(i18n("Value"));
 d->mPlayerProperties->addColumn(i18n("Policy"));
 layout->addWidget(d->mPlayerProperties);
 
 QPushButton* b = new QPushButton(i18n("Update"), d->mPlayerPage);
 connect(b, SIGNAL(pressed()), this, SLOT(slotUpdatePlayerList()));
 topLayout->addWidget(b);

 d->mPlayerAddress = new Q3ListViewItem(v, i18n("Player Pointer"));
 d->mPlayerId = new Q3ListViewItem(v, i18n("Player ID"));
 d->mPlayerName = new Q3ListViewItem(v, i18n("Player Name"));
 d->mPlayerGroup = new Q3ListViewItem(v, i18n("Player Group"));
 d->mPlayerUserId = new Q3ListViewItem(v, i18n("Player User ID"));
 d->mPlayerMyTurn = new Q3ListViewItem(v, i18n("My Turn"));
 d->mPlayerAsyncInput = new Q3ListViewItem(v, i18n("Async Input"));
 d->mPlayerKGameAddress = new Q3ListViewItem(v, i18n("KGame Address"));
 d->mPlayerVirtual = new Q3ListViewItem(v, i18n("Player is Virtual"));
 d->mPlayerActive = new Q3ListViewItem(v, i18n("Player is Active"));
 d->mPlayerRtti = new Q3ListViewItem(v, i18n("RTTI"));
 d->mPlayerNetworkPriority = new Q3ListViewItem(v, i18n("Network Priority"));
}

void KGameDebugDialog::initMessagePage()
{
 d->mMessagePage = addPage(i18n("Debug &Messages"));
 QGridLayout* layout = new QGridLayout(d->mMessagePage);
 layout->setMargin(marginHint());
 layout->setSpacing(spacingHint());
 d->mMessageList = new K3ListView(d->mMessagePage);
 layout->addWidget(d->mMessageList, 0, 0, 10, 4);
 d->mMessageList->addColumn(i18n("Time"));
 d->mMessageList->addColumn(i18n("ID"));
 d->mMessageList->addColumn(i18n("Receiver"));
 d->mMessageList->addColumn(i18n("Sender"));
 d->mMessageList->addColumn(i18n("ID - Text"));

 QPushButton* hide = new QPushButton(i18n("&>>"), d->mMessagePage);
 connect(hide, SIGNAL(pressed()), this, SLOT(slotHideId()));
 layout->addWidget(hide, 4, 4);
 
 QPushButton* show = new QPushButton(i18n("&<<"), d->mMessagePage);
 connect(show, SIGNAL(pressed()), this, SLOT(slotShowId()));
 layout->addWidget(show, 6, 4);

 QLabel* l = new QLabel(i18n("Do not show IDs:"), d->mMessagePage);
 layout->addWidget(l, 0, 5, 1, 2);
 d->mHideIdList = new KListBox(d->mMessagePage);
 layout->addWidget(d->mHideIdList, 1, 5, 8, 2);

 QPushButton* clear = new KPushButton(KStdGuiItem::clear(), d->mMessagePage);
 connect(clear, SIGNAL(pressed()), this, SLOT(slotClearMessages()));
 layout->addWidget(clear, 10, 0, 1, 7);
 //TODO: "show all but..." and "show nothing but..."
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

void KGameDebugDialog::slotUpdatePlayerData()
{
 if (!d->mGame || d->mPlayerList->currentItem() == -1) {
	return;
 }
 slotUpdatePlayerData(d->mPlayerList->item(d->mPlayerList->currentItem()));
}

void KGameDebugDialog::slotUpdatePlayerList()
{
 Q3ListBoxItem* i = d->mPlayerList->firstItem(); 
 for (; i; i = d->mPlayerList->firstItem()) {
	removePlayer(i);
 }

 Q3PtrList<KPlayer> list = *d->mGame->playerList();
 for (KPlayer* p = list.first(); p; p = list.next()) {
	addPlayer(p);
 }
}

void KGameDebugDialog::slotUpdateGameData()
{
 if (!d->mGame) {
	d->mGameAddress->setText(1, i18n("NULL pointer"));
	return;
}

 clearGameData();

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

//TODO ios

 KGamePropertyHandler* handler = d->mGame->dataHandler();
 Q3IntDictIterator<KGamePropertyBase> it(handler->dict());
 while (it.current()) {
	QString policy;
	switch (it.current()->policy()) {
		case KGamePropertyBase::PolicyClean:
			policy = i18n("Clean");
			break;
		case KGamePropertyBase::PolicyDirty:
			policy = i18n("Dirty");
			break;
		case KGamePropertyBase::PolicyLocal:
			policy = i18n("Local");
			break;
		case KGamePropertyBase::PolicyUndefined:
		default:
			policy = i18n("Undefined");
			break;
	}
	(void) new Q3ListViewItem(d->mGameProperties,
			handler->propertyName(it.current()->id()),
			handler->propertyValue(it.current()), 
			policy);
//	kDebug(11001) << k_funcinfo << ": checking for all game properties: found property name " << name << endl;
	++it;
 }
}

void KGameDebugDialog::slotUpdatePlayerData(Q3ListBoxItem* item)
{
 if (!item || !d->mGame) {
	return;
 }

 KPlayer* p = d->mGame->findPlayer(item->text().toInt());

 if (!p) {
	kError(11001) << k_funcinfo << ": cannot find player" << endl;
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
 KGamePropertyHandler * handler = p->dataHandler();
 Q3IntDictIterator<KGamePropertyBase> it((handler->dict()));
 while (it.current()) {
	QString policy;
	switch (it.current()->policy()) {
		case KGamePropertyBase::PolicyClean:
			policy = i18n("Clean");
			break;
		case KGamePropertyBase::PolicyDirty:
			policy = i18n("Dirty");
			break;
		case KGamePropertyBase::PolicyLocal:
			policy = i18n("Local");
			break;
		case KGamePropertyBase::PolicyUndefined:
		default:
			policy = i18n("Undefined");
			break;
	}
	(void)new Q3ListViewItem(d->mPlayerProperties,
			handler->propertyName(it.current()->id()),
			handler->propertyValue(it.current()),
			policy);
	++it;
 }
}

void KGameDebugDialog::clearPages()
{
 clearPlayerData();
 clearGameData();
 d->mPlayerList->clear();
 slotClearMessages();
}

void KGameDebugDialog::setKGame(const KGame* g)
{
 slotUnsetKGame();
 d->mGame = g;
 if (g) {
 //TODO: connect to the KGame signals for joined/removed players!!!
	connect(d->mGame, SIGNAL(destroyed()), this, SLOT(slotUnsetKGame()));
//	connect();

	Q3PtrList<KPlayer> list = *d->mGame->playerList();
	for (KPlayer* p = list.first(); p; p = list.next()) {
		addPlayer(p);
	}

	slotUpdateGameData();

	connect(d->mGame, SIGNAL(signalMessageUpdate(int, quint32, quint32)), this, SLOT(slotMessageUpdate(int, quint32, quint32)));
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
	kError(11001) << "trying to add NULL player" << endl;
	return;
 }

 (void) new Q3ListBoxText(d->mPlayerList, QString::number(p->id()));
 //TODO connect to signals, like deleted/removed, ...
}

void KGameDebugDialog::removePlayer(Q3ListBoxItem* i)
{
 if (!i || !d->mGame) {
	return;
 }
 KPlayer* p = d->mGame->findPlayer(i->text().toInt());
 if (!p) {
	return;
 }
 disconnect(p, 0, this, 0);
 if (i->isSelected()) {
	clearPlayerData();
 }
 delete i;
}

void KGameDebugDialog::slotMessageUpdate(int msgid, quint32 receiver, quint32 sender)
{
 if (!showId(msgid)) {
	return;
 }
 QString msgidText = KGameMessage::messageId2Text(msgid);
 if (msgidText.isNull()) {
	if (msgid > KGameMessage::IdUser) {
		emit signalRequestIdName(msgid-KGameMessage::IdUser, true, msgidText);
	} else {
		emit signalRequestIdName(msgid, false, msgidText);
	}
	if (msgidText.isNull()) {
		msgidText = i18n("Unknown");
	}
 }
 (void) new Q3ListViewItem( d->mMessageList, QTime::currentTime().toString(), 
		QString::number(msgid), QString::number(receiver), 
		QString::number(sender), msgidText);
}

void KGameDebugDialog::slotClearMessages()
{
 d->mMessageList->clear();
}

void KGameDebugDialog::slotShowId()
{
/* QListBoxItem* i = d->mHideIdList->firstItem();
 for (; i; i = i->next()) {
	if (i->selected()) {
		d->mHideIdList->removeItem(i->);
	}
 }*/
 if (!d->mHideIdList->currentItem()) {
	return;
 }
 d->mHideIdList->removeItem(d->mHideIdList->currentItem());
}

void KGameDebugDialog::slotHideId()
{
 if (!d->mMessageList->currentItem()) {
	return;
 }
 int msgid = d->mMessageList->currentItem()->text(1).toInt();
 if (!showId(msgid)) {
	return;
 }
 (void)new Q3ListBoxText(d->mHideIdList, QString::number(msgid));
}

bool KGameDebugDialog::showId(int msgid)
{
 Q3ListBoxItem* i = d->mHideIdList->firstItem();
 for (; i; i = i->next()) {
	if (i->text().toInt() == msgid) {
		return false;
	}
 }
 return true;
}


#include "kgamedebugdialog.moc"
