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
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <QLayout>
#include <QComboBox>
#include <QPainter>
#include <QPixmap>
#include <QList>
#include <QApplication>
#include <QListView>



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
    d->mModel=m;
}

KChatBaseModel* KChatBase::model()
{
    return d->mModel;
}

KChatBase::KChatBase(QWidget* parent, KChatBaseModel* model, KChatBaseItemDelegate* delegate, bool noComboBox) : QFrame(parent)
{
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
 connect(d->mBox, SIGNAL(rightButtonClicked(QListWidgetItem*, const QPoint&)),
		this, SIGNAL(rightButtonClicked(QListWidgetItem*, const QPoint&)));
 l->addWidget(d->mBox);

 connect(d->mModel, SIGNAL(rowsInserted (const QModelIndex&,int,int)),
          d->mBox, SLOT(scrollToBottom()));
          

 d->mBox->setFocusPolicy(Qt::NoFocus);
 d->mBox->setSelectionMode(QAbstractItemView::SingleSelection);

 l->addSpacing(5);

 QHBoxLayout* h = new QHBoxLayout;
 l->addLayout(h);
 d->mEdit = new KLineEdit(this);
 d->mEdit->setHandleSignals(false);
 d->mEdit->setTrapReturnKey(true);
 d->mEdit->completionObject(); // add the completion object
 d->mEdit->setCompletionMode(KGlobalSettings::CompletionNone);
 connect(d->mEdit, SIGNAL(returnPressed(const QString&)), this, SLOT(slotReturnPressed(const QString&)));
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

KChatBase::~KChatBase()
{
// kDebug(11000) << "KChatBase: DESTRUCT (" << this << ")";
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
	kWarning(11000) << "KChatBase: Cannot add an entry to the combo box";
	return false;
 }
 if (d->mIndex2Id.indexOf(id) != -1) {
	kError(11000) << "KChatBase: Cannot add more than one entry with the same ID! ";
	kError(11000) << "KChatBase: Text="<<text<<endl;
	return false;
 }
 d->mCombo->insertItem(index, text);
 if (index < 0) {
	d->mIndex2Id.append(id);
 } else {
	d->mIndex2Id.insert(d->mIndex2Id.at(index), id);
 }
 if (d->mIndex2Id.count() != d->mCombo->count()) {
	kError(11000) << "KChatBase: internal ERROR - local IDs do not match combo box entries!";
 }
 return true;
}

int KChatBase::sendingEntry() const
{
 if (!d->mCombo) {
	kWarning(11001) << "Cannot retrieve index from NULL combo box";
	return -1;
 }
 int index = d->mCombo->currentIndex();
 if ( index > 0 && index <  d->mIndex2Id.size()) {
	kWarning(11000) << "could not find the selected sending entry!";
	return -1;
 }
 return d->mIndex2Id[index];
}

void KChatBase::removeSendingEntry(int id)
{
 if (!d->mCombo) {
	kWarning(11000) << "KChatBase: Cannot remove an entry from the combo box";
	return;
 }
 d->mCombo->removeItem(findIndex(id));
 d->mIndex2Id.removeAll(id);
}

void KChatBase::changeSendingEntry(const QString& text, int id)
{
 if (!d->mCombo) {
	kWarning(11000) << "KChatBase: Cannot change an entry in the combo box";
	return;
 }
 int index = findIndex(id);
 d->mCombo->setItemText(index, text);
}

void KChatBase::setSendingEntry(int id)
{
 if (!d->mCombo) {
	kWarning(11000) << "KChatBase: Cannot set an entry in the combo box";
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

void KChatBase::setCompletionMode(KGlobalSettings::Completion mode)
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


const QFont& KChatBase::nameFont() const
{
  return d->mModel->nameFont();
}

const QFont& KChatBase::messageFont() const
{
  return d->mModel->messageFont();
}

const QFont& KChatBase::systemNameFont() const
{
  return d->mModel->systemNameFont();
}

const QFont& KChatBase::systemMessageFont() const
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
