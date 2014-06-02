/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

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

#include "kchatbase.h"
#include "kchatbasemodel.h"
#include "kchatbaseitemdelegate.h"

#include <klineedit.h>
#include <klocale.h>
#include <kconfig.h>
#include <QLayout>
#include <QComboBox>
#include <QPixmap>
#include <QList>
#include <QApplication>
#include <QListView>

Q_LOGGING_CATEGORY(GAMES_BACKGAMMON, "games.backgammon")

class KChatBasePrivate
{
public:
  KChatBasePrivate(KChatBaseModel* model, KChatBaseItemDelegate* delegate)
  {
    mBox = 0;
    mEdit = 0;
    mCombo = 0;
  
    mAcceptMessage = true;

    mModel = model;
    mDelegate = delegate;
  }
  QListView* mBox;
  KLineEdit* mEdit;
  QComboBox* mCombo;
  bool mAcceptMessage;

  QList<int> mIndex2Id;

  KChatBaseModel* mModel;
  KChatBaseItemDelegate* mDelegate;
};

void KChatBase::setModel(KChatBaseModel* m)
{
    //delete d->mModel;
    d->mModel=m;
}

KChatBaseModel* KChatBase::model()
{
    return d->mModel;
}

KChatBase::KChatBase(QWidget* parent, KChatBaseModel* model, KChatBaseItemDelegate* delegate, bool noComboBox) : QFrame(parent)
{
  QLoggingCategory::setFilterRules(QLatin1Literal("games.lib.debug = true"));
  QLoggingCategory::setFilterRules(QLatin1Literal("games.private.kgame.debug = true"));
  QLoggingCategory::setFilterRules(QLatin1Literal("games.backgammon.debug = true"));
  
  KChatBaseModel* mod = model;
  if (mod==0)
    mod = new KChatBaseModel(parent);
  KChatBaseItemDelegate* del = delegate;
  if (del == 0)
    del = new KChatBaseItemDelegate(parent);
  
  d = new KChatBasePrivate(mod, del);

 setMinimumWidth(100);
 setMinimumHeight(150);

 QVBoxLayout* l = new QVBoxLayout(this);

 d->mBox = new QListView();
 d->mBox->setModel(d->mModel);
 d->mBox->setItemDelegate(d->mDelegate);
 l->addWidget(d->mBox);

 connect(d->mModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
          d->mBox, SLOT(scrollToBottom()));

 connect(d->mBox, SIGNAL(customContextMenuRequested(QPoint)),
         this, SLOT(customMenuHandler(QPoint)));

 d->mBox->setContextMenuPolicy ( Qt::CustomContextMenu );
 d->mBox->setFocusPolicy(Qt::NoFocus);
 d->mBox->setSelectionMode(QAbstractItemView::SingleSelection);

 l->addSpacing(5);

 QHBoxLayout* h = new QHBoxLayout;
 l->addLayout(h);
 d->mEdit = new KLineEdit(this);
 d->mEdit->setHandleSignals(false);
 d->mEdit->setTrapReturnKey(true);
 d->mEdit->completionObject(); // add the completion object
 d->mEdit->setCompletionMode(KCompletion::CompletionNone);
 connect(d->mEdit, SIGNAL(returnPressed(QString)), this, SLOT(slotReturnPressed(QString)));
 h->addWidget(d->mEdit);

 if (!noComboBox) {
	d->mCombo = new QComboBox(this);
	h->addWidget(d->mCombo);
	addSendingEntry(i18n("Send to All Players"), SendToAll);//FIXME: where to put the id?
 }

 d->mAcceptMessage = true; // by default
 setMaxItems(-1); // unlimited

 readConfig();
}

const QModelIndex KChatBase::indexAt(const QPoint& pos) const
{
    return d->mBox->indexAt(pos);
}

void KChatBase::customMenuHandler(const QPoint &pos)
{
    qCDebug(GAMES_BACKGAMMON) << "custom menu has been requested at position="<<pos<<". Implement handler at subclass if you need it.";
}

KChatBase::~KChatBase()
{
// qCDebug(GAMES_LIB) << "KChatBase: DESTRUCT (" << this << ")";
 saveConfig();
 delete d;
}

bool KChatBase::acceptMessage() const
{ return d->mAcceptMessage; }

void KChatBase::setAcceptMessage(bool a)
{ d->mAcceptMessage = a; }

bool KChatBase::addSendingEntry(const QString& text, int id)
{
//FIXME: is ID used correctly?
// do we need ID at all?
// what the hell should be here?
// d->mCombo->insertItem(i18n("Send to All Players"), SendToAll);
 return insertSendingEntry(text, id);
}

bool KChatBase::insertSendingEntry(const QString& text, int id, int index)
{
 if (!d->mCombo) {
	qCWarning(GAMES_LIB) << "KChatBase: Cannot add an entry to the combo box";
	return false;
 }
 if (d->mIndex2Id.indexOf(id) != -1) {
	qCCritical(GAMES_LIB) << "KChatBase: Cannot add more than one entry with the same ID! ";
	qCCritical(GAMES_LIB) << "KChatBase: Text="<<text;
	return false;
 }
 d->mCombo->insertItem(index, text);
 if (index < 0) {
	d->mIndex2Id.prepend(id);
 } else {
	d->mIndex2Id.insert(d->mIndex2Id.at(index), id);
 }
 if (d->mIndex2Id.count() != d->mCombo->count()) {
	qCCritical(GAMES_LIB) << "KChatBase: internal ERROR - local IDs do not match combo box entries!";
 }
 return true;
}

int KChatBase::sendingEntry() const
{
 if (!d->mCombo) {
	qCWarning(GAMES_PRIVATE_KGAME) << "Cannot retrieve index from NULL combo box";
	return -1;
 }
 const int index = d->mCombo->currentIndex();
 if ( index >= 0 && index <  d->mIndex2Id.size())
    return d->mIndex2Id[index];

 qCWarning(GAMES_LIB) << "could not find the selected sending entry!";
 return -1;
}

void KChatBase::removeSendingEntry(int id)
{
 if (!d->mCombo) {
	qCWarning(GAMES_LIB) << "KChatBase: Cannot remove an entry from the combo box";
	return;
 }
 int idx = findIndex(id);
 //Guard, passing -1 will crash qcombobox
 if (idx>=0) d->mCombo->removeItem(idx);
 d->mIndex2Id.removeAll(id);
}

void KChatBase::changeSendingEntry(const QString& text, int id)
{
 if (!d->mCombo) {
	qCWarning(GAMES_LIB) << "KChatBase: Cannot change an entry in the combo box";
	return;
 }
 int index = findIndex(id);
 d->mCombo->setItemText(index, text);
}

void KChatBase::setSendingEntry(int id)
{
 if (!d->mCombo) {
	qCWarning(GAMES_LIB) << "KChatBase: Cannot set an entry in the combo box";
	return;
 }
 d->mCombo->setCurrentIndex(findIndex(id));
}

int KChatBase::findIndex(int id) const
{
 return d->mIndex2Id.indexOf(id);
}

int KChatBase::nextId() const
{
 int i = SendToAll + 1;
 while (d->mIndex2Id.indexOf(i) != -1) {
	i++;
 }
 return i;
}

void KChatBase::slotReturnPressed(const QString& text)
{
 if (text.length() <= 0) {
	// no text entered - probably hit return by accident
	return;
 } else if (!acceptMessage()) {
	return;
 }
 d->mEdit->completionObject()->addItem(text);
 d->mEdit->clear();
 returnPressed(text);
}

QString KChatBase::comboBoxItem(const QString& name) const
{ // TODO: such a function for "send to all" and "send to my group"
 return i18n("Send to %1", name);
}

void KChatBase::slotClear()
{
 clear();
}


void KChatBase::setCompletionMode(KCompletion::CompletionMode mode)
{ d->mEdit->setCompletionMode(mode); }

void KChatBase::saveConfig(KConfig* conf)
{
  if (conf == 0) {
    return;
  }
  d->mModel->saveConfig(conf);
}

void KChatBase::readConfig(KConfig* conf)
{
  if (conf == 0) {
    return;
  }
  d->mModel->readConfig(conf);
}

void KChatBase::clear()
{
 d->mModel->removeRows(0, d->mModel->rowCount());
}

void KChatBase::setMaxItems(int maxItems)
{
 d->mModel->setMaxItems(maxItems);
 //TODO cut too many messages
 if (maxItems == 0) {
	clear();
 } else if (maxItems > 0) {
  while (d->mModel->rowCount() > (int)maxItems) {
   d->mModel->removeRow(0);
  }
 }
}

int KChatBase::maxItems() const
{ 
  return d->mModel->maxItems(); 
}

QFont KChatBase::nameFont() const
{
  return d->mModel->nameFont();
}

QFont KChatBase::messageFont() const
{
  return d->mModel->messageFont();
}

QFont KChatBase::systemNameFont() const
{
  return d->mModel->systemNameFont();
}

QFont KChatBase::systemMessageFont() const
{
  return d->mModel->systemMessageFont();
}

void KChatBase::setNameFont(const QFont& font)
{
  d->mModel->setNameFont(font);
}

void KChatBase::setMessageFont(const QFont& font)
{
  d->mModel->setMessageFont(font);
}

void KChatBase::setBothFont(const QFont& font)
{
  d->mModel->setBothFont(font);
}

void KChatBase::setSystemNameFont(const QFont& font)
{
  d->mModel->setSystemNameFont(font);
}

void KChatBase::setSystemMessageFont(const QFont& font)
{
  d->mModel->setSystemMessageFont(font);
}

void KChatBase::setSystemBothFont(const QFont& font)
{
  d->mModel->setSystemBothFont(font);
}

void KChatBase::addMessage(const QString& fromName, const QString& text)
{
   d->mModel->addMessage(fromName, text);
}

void KChatBase::addSystemMessage(const QString& fromName, const QString& text)
{
   d->mModel->addSystemMessage(fromName, text);
}

#include "kchatbase.moc"
