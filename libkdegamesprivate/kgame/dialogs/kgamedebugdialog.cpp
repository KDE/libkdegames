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

#include "kgamedebugdialog.h"

#include "kgamemessage.h"
#include "kgame.h"
#include "kplayer.h"
#include "kgamepropertyhandler.h"

#include <klistwidget.h>
#include <klocale.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <KStandardGuiItem>

#include <QTreeWidget>
#include <QLayout>

#include <QLabel>
#include <QDateTime>
#include <QVBoxLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>

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
	QTreeWidget* mGameProperties;
	QTreeWidgetItem* mGameAddress;
	QTreeWidgetItem* mGameId;
	QTreeWidgetItem* mGameCookie;
	QTreeWidgetItem* mGameMaster;
	QTreeWidgetItem* mGameAdmin;
	QTreeWidgetItem* mGameOffering;
	QTreeWidgetItem* mGameStatus;
	QTreeWidgetItem* mGameRunning;
	QTreeWidgetItem* mGameMaxPlayers;
	QTreeWidgetItem* mGameMinPlayers;
	QTreeWidgetItem* mGamePlayerCount;

	QFrame* mPlayerPage;
	KListWidget* mPlayerList;
	QTreeWidget* mPlayerProperties;
	QTreeWidgetItem* mPlayerAddress;
	QTreeWidgetItem* mPlayerId;
	QTreeWidgetItem* mPlayerName;
	QTreeWidgetItem* mPlayerGroup;
	QTreeWidgetItem* mPlayerUserId;
	QTreeWidgetItem* mPlayerMyTurn;
	QTreeWidgetItem* mPlayerAsyncInput;
	QTreeWidgetItem* mPlayerKGameAddress;
	QTreeWidgetItem* mPlayerVirtual;
	QTreeWidgetItem* mPlayerActive;
	QTreeWidgetItem* mPlayerRtti;
	QTreeWidgetItem* mPlayerNetworkPriority;

	QFrame* mMessagePage;
	QTreeWidget* mMessageList;
	KListWidget* mHideIdList;
};

KGameDebugDialog::KGameDebugDialog(KGame* g, QWidget* parent, bool modal)
    : KPageDialog(parent),
      d( new KGameDebugDialogPrivate )
{
 setCaption(i18n("KGame Debug Dialog"));
 setButtons(Close);
 setDefaultButton(Close);
 setModal(modal);
 showButtonSeparator(true);
 setFaceType(KPageDialog::Tabbed);

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
 d->mGamePage = new QFrame();
 addPage(d->mGamePage,i18n("Debug &KGame"));
 QVBoxLayout* topLayout = new QVBoxLayout(d->mGamePage);
 topLayout->setMargin( marginHint() );
 topLayout->setSpacing( spacingHint() );
 QHBoxLayout* layout = new QHBoxLayout;
 topLayout->addLayout(layout);

 QTreeWidget* v = new QTreeWidget(d->mGamePage);
 QTreeWidgetItem* vheader = new QTreeWidgetItem();
 vheader->setText(0, tr("Data"));
 vheader->setText(1, tr("Value"));
 v->setHeaderItem(vheader);
 layout->addWidget(v);

 d->mGameProperties = new QTreeWidget(d->mGamePage);
 QTreeWidgetItem* mGamePropertiesHeader = new QTreeWidgetItem();
 mGamePropertiesHeader->setText(0, tr("Property"));
 mGamePropertiesHeader->setText(1, tr("Value"));
 mGamePropertiesHeader->setText(2, tr("Policy"));
 d->mGameProperties->setHeaderItem(mGamePropertiesHeader);
 layout->addWidget(d->mGameProperties);

 QPushButton* b = new QPushButton(i18n("Update"), d->mGamePage);
 connect(b, SIGNAL(pressed()), this, SLOT(slotUpdateGameData()));
 topLayout->addWidget(b);

// game data
 d->mGameAddress = new QTreeWidgetItem(v, QStringList(i18n("KGame Pointer")));
 d->mGameId = new QTreeWidgetItem(v, QStringList(i18n("Game ID")));
 d->mGameCookie = new QTreeWidgetItem(v, QStringList(i18n("Game Cookie")));
 d->mGameMaster = new QTreeWidgetItem(v, QStringList(i18n("Is Master")));
 d->mGameAdmin = new QTreeWidgetItem(v, QStringList(i18n("Is Admin")));
 d->mGameOffering = new QTreeWidgetItem(v, QStringList(i18n("Is Offering Connections")));
 d->mGameStatus = new QTreeWidgetItem(v, QStringList(i18n("Game Status")));
 d->mGameRunning = new QTreeWidgetItem(v, QStringList(i18n("Game is Running")));
 d->mGameMaxPlayers = new QTreeWidgetItem(v, QStringList(i18n("Maximal Players")));
 d->mGameMinPlayers = new QTreeWidgetItem(v, QStringList(i18n("Minimal Players")));
 d->mGamePlayerCount = new QTreeWidgetItem(v, QStringList(i18n("Players")));
}

void KGameDebugDialog::initPlayerPage()
{
 d->mPlayerPage = new QFrame();
 addPage(d->mPlayerPage,i18n("Debug &Players"));
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
 d->mPlayerList = new KListWidget(d->mPlayerPage);
 connect(d->mPlayerList, SIGNAL(executed(QListWidgetItem*)), this, SLOT(slotUpdatePlayerData(QListWidgetItem*)));
 listLayout->addWidget(d->mPlayerList);
 d->mPlayerList->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));

 QTreeWidget* v = new QTreeWidget(d->mPlayerPage);
 layout->addWidget(v);
 QTreeWidgetItem* vheader = new QTreeWidgetItem();
 vheader->setText(0, tr("Data"));
 vheader->setText(1, tr("Value"));
 v->setHeaderItem(vheader);

 d->mPlayerProperties = new QTreeWidget(d->mPlayerPage);
 QTreeWidgetItem* mPlayerPropertiesHeader = new QTreeWidgetItem();
 mPlayerPropertiesHeader->setText(0, tr("Property"));
 mPlayerPropertiesHeader->setText(1, tr("Value"));
 mPlayerPropertiesHeader->setText(2, tr("Policy")); d->mPlayerProperties->setHeaderItem(mPlayerPropertiesHeader);
 layout->addWidget(d->mPlayerProperties);

 QPushButton* b = new QPushButton(i18n("Update"), d->mPlayerPage);
 connect(b, SIGNAL(pressed()), this, SLOT(slotUpdatePlayerList()));
 topLayout->addWidget(b);

 d->mPlayerAddress = new QTreeWidgetItem(v, QStringList(i18n("Player Pointer")));
 d->mPlayerId = new QTreeWidgetItem(v, QStringList(i18n("Player ID")));
 d->mPlayerName = new QTreeWidgetItem(v, QStringList(i18n("Player Name")));
 d->mPlayerGroup = new QTreeWidgetItem(v, QStringList(i18n("Player Group")));
 d->mPlayerUserId = new QTreeWidgetItem(v, QStringList(i18n("Player User ID")));
 d->mPlayerMyTurn = new QTreeWidgetItem(v, QStringList(i18n("My Turn")));
 d->mPlayerAsyncInput = new QTreeWidgetItem(v, QStringList(i18n("Async Input")));
 d->mPlayerKGameAddress = new QTreeWidgetItem(v, QStringList(i18n("KGame Address")));
 d->mPlayerVirtual = new QTreeWidgetItem(v, QStringList(i18n("Player is Virtual")));
 d->mPlayerActive = new QTreeWidgetItem(v, QStringList(i18n("Player is Active")));
 d->mPlayerRtti = new QTreeWidgetItem(v, QStringList(i18n("RTTI")));
 d->mPlayerNetworkPriority = new QTreeWidgetItem(v, QStringList(i18n("Network Priority")));
}

void KGameDebugDialog::initMessagePage()
{
 d->mMessagePage = new QFrame();
 addPage(d->mMessagePage,i18n("Debug &Messages"));
 QGridLayout* layout = new QGridLayout(d->mMessagePage);
 layout->setMargin(marginHint());
 layout->setSpacing(spacingHint());
 d->mMessageList = new QTreeWidget(d->mMessagePage);
 layout->addWidget(d->mMessageList, 0, 0, 10, 4);

 QTreeWidgetItem* mMessageListHeader = new QTreeWidgetItem();
 mMessageListHeader->setText(0, tr("Time"));
 mMessageListHeader->setText(1, tr("ID"));
 mMessageListHeader->setText(2, tr("Receiver")); mMessageListHeader->setText(2, tr("Sender"));
 mMessageListHeader->setText(2, tr("ID - Text"));
 d->mMessageList->setHeaderItem(mMessageListHeader);

 QPushButton* hide = new QPushButton(i18n("&>>"), d->mMessagePage);
 connect(hide, SIGNAL(pressed()), this, SLOT(slotHideId()));
 layout->addWidget(hide, 4, 4);

 QPushButton* show = new QPushButton(i18n("&<<"), d->mMessagePage);
 connect(show, SIGNAL(pressed()), this, SLOT(slotShowId()));
 layout->addWidget(show, 6, 4);

 QLabel* l = new QLabel(i18n("Do not show IDs:"), d->mMessagePage);
 layout->addWidget(l, 0, 5, 1, 2);
 d->mHideIdList = new KListWidget(d->mMessagePage);
 layout->addWidget(d->mHideIdList, 1, 5, 8, 2);

 QPushButton* clear = new KPushButton(KStandardGuiItem::clear(), d->mMessagePage);
 connect(clear, SIGNAL(pressed()), this, SLOT(slotClearMessages()));
 layout->addWidget(clear, 10, 0, 1, 7);
 //TODO: "show all but..." and "show nothing but..."
}

void KGameDebugDialog::clearPlayerData()
{
 d->mPlayerAddress->setText(1, QLatin1String( "" ));
 d->mPlayerId->setText(1, QLatin1String( "" ));
 d->mPlayerName->setText(1, QLatin1String( "" ));
 d->mPlayerGroup->setText(1, QLatin1String( "" ));
 d->mPlayerUserId->setText(1, QLatin1String( "" ));
 d->mPlayerMyTurn->setText(1, QLatin1String( "" ));
 d->mPlayerAsyncInput->setText(1, QLatin1String( "" ));
 d->mPlayerKGameAddress->setText(1, QLatin1String( "" ));
 d->mPlayerVirtual->setText(1, QLatin1String( "" ));
 d->mPlayerActive->setText(1, QLatin1String( "" ));
 d->mPlayerRtti->setText(1, QLatin1String( "" ));
 d->mPlayerNetworkPriority->setText(1, QLatin1String( "" ));

 d->mPlayerProperties->clear();
}

void KGameDebugDialog::clearGameData()
{
 d->mGameAddress->setText(1, QLatin1String( "" ));
 d->mGameId->setText(1, QLatin1String( "" ));
 d->mGameCookie->setText(1, QLatin1String( "" ));
 d->mGameMaster->setText(1, QLatin1String( "" ));
 d->mGameAdmin->setText(1, QLatin1String( "" ));
 d->mGameOffering->setText(1, QLatin1String( "" ));
 d->mGameStatus->setText(1, QLatin1String( "" ));
 d->mGameRunning->setText(1, QLatin1String( "" ));
 d->mGameMaxPlayers->setText(1, QLatin1String( "" ));
 d->mGameMinPlayers->setText(1, QLatin1String( "" ));

 d->mGameProperties->clear();
}

void KGameDebugDialog::slotUpdatePlayerData()
{
 if (!d->mGame || d->mPlayerList->currentRow() == -1) {
	return;
 }
 slotUpdatePlayerData(d->mPlayerList->item(d->mPlayerList->currentRow()));
}

void KGameDebugDialog::slotUpdatePlayerList()
{
 QListWidgetItem* i = d->mPlayerList->item(0);
 for (; d->mPlayerList->count() > 0; i = d->mPlayerList->item(0)) {
	removePlayer(i);
 }

 for ( QList<KPlayer*>::const_iterator it = d->mGame->playerList()->begin(); it!=d->mGame->playerList()->end(); ++it )
 {
  addPlayer(*it);
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
 buf.sprintf("%p", (void*)d->mGame);
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
 QHashIterator<int, KGamePropertyBase*> it(handler->dict());
 while (it.hasNext()) {
	it.next();
	QString policy;
	switch (it.value()->policy()) {
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
  QStringList items;
  items << handler->propertyName(it.value()->id()) << handler->propertyValue(it.value()) << policy;
	new QTreeWidgetItem(d->mGameProperties,items);
//	kDebug(11001) << ": checking for all game properties: found property name" << name;
 }
}

void KGameDebugDialog::slotUpdatePlayerData(QListWidgetItem* item)
{
 if (!item || !d->mGame) {
	return;
 }

 KPlayer* p = d->mGame->findPlayer(item->text().toInt());

 if (!p) {
	kError(11001) << ": cannot find player";
	return;
 }

 clearPlayerData();

 QString buf;
 buf.sprintf("%p", (void*)p);
 d->mPlayerAddress->setText(1, buf);
 d->mPlayerId->setText(1, QString::number(p->id()));
 d->mPlayerName->setText(1, p->name());
 d->mPlayerGroup->setText(1, p->group());
 d->mPlayerUserId->setText(1, QString::number(p->userId()));
 d->mPlayerMyTurn->setText(1, p->myTurn() ? i18n("True") : i18n("False"));
 d->mPlayerAsyncInput->setText(1, p->asyncInput() ? i18n("True") : i18n("False"));
 buf.sprintf("%p", (void*)p->game());
 d->mPlayerKGameAddress->setText(1, buf);
 d->mPlayerVirtual->setText(1, p->isVirtual() ? i18n("True") : i18n("False"));
 d->mPlayerActive->setText(1, p->isActive() ? i18n("True") : i18n("False"));
 d->mPlayerRtti->setText(1, QString::number(p->rtti()));
 d->mPlayerNetworkPriority->setText(1, QString::number(p->networkPriority()));

//TODO ios

// Properties
 KGamePropertyHandler * handler = p->dataHandler();
 QHashIterator<int, KGamePropertyBase*> it((handler->dict()));
 while (it.hasNext()) {
	it.next();
	QString policy;
	switch (it.value()->policy()) {
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
  QStringList items;
  items << handler->propertyName(it.value()->id()) << handler->propertyValue(it.value()) << policy;
	new QTreeWidgetItem(d->mPlayerProperties,items);
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

  for ( QList<KPlayer*>::const_iterator it = d->mGame->playerList()->begin(); it!=d->mGame->playerList()->end(); ++it )
  {
	  addPlayer(*it);
  }

	slotUpdateGameData();

	connect(d->mGame, SIGNAL(signalMessageUpdate(int,quint32,quint32)), this, SLOT(slotMessageUpdate(int,quint32,quint32)));
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
	kError(11001) << "trying to add NULL player";
	return;
 }

 (void) new QListWidgetItem(QString::number(p->id()), d->mPlayerList);
 //TODO connect to signals, like deleted/removed, ...
}

void KGameDebugDialog::removePlayer(QListWidgetItem* i)
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
 QStringList items;
 items << QTime::currentTime().toString() <<
    QString::number(msgid) << QString::number(receiver) <<
    QString::number(sender) << msgidText;
 new QTreeWidgetItem( d->mMessageList, items);
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
 d->mHideIdList->takeItem(d->mHideIdList->currentRow());
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
 (void)new QListWidgetItem(QString::number(msgid), d->mHideIdList);
}

bool KGameDebugDialog::showId(int msgid)
{
 for (int j = 0; j < d->mHideIdList->count(); ++j) {
 	QListWidgetItem* i = d->mHideIdList->item(j);
	if (i->text().toInt() == msgid) {
		return false;
	}
 }
 return true;
}


#include "kgamedebugdialog.moc"
