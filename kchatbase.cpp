/* **************************************************************************
                             KChatBase Class
                           -------------------
    begin                : 14 March 2001
    copyright            : (C) 2001 by Andreas Beckermann and Martin Heni
    email                : b_mann@gmx.de and martin@heni-online.de
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Additional license: Any of the above copyright holders can add an     *
 *   enhanced license which complies with the license of the KDE core      *
 *   libraries so that this file resp. this library is compatible with     *
 *   the KDE core libraries.                                               *
 *   The user of this program shall have the choice which license to use   *
 *                                                                         *
 ***************************************************************************/
#include <qlistbox.h>
#include <qlayout.h>
#include <qcombobox.h>

#include <klineedit.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kdebug.h>

#include "kchatbase.h"

class KChatBasePrivate
{
public:
	KChatBasePrivate() 
	{ 
		mBox = 0;
		mEdit = 0;
		mCombo = 0;

		mAcceptMessage = true;
	}
	QListBox* mBox;
	KLineEdit* mEdit;
	QComboBox* mCombo;
	bool mAcceptMessage;

	QValueList<int> mIndex2Id;
};

KChatBase::KChatBase(QWidget* parent, bool noComboBox) : QFrame(parent)
{
 init(noComboBox); 
}

KChatBase::~KChatBase()
{
 kdDebug(11000) << "KChatBase: DESTRUCT (" << this << ")" << endl;
 delete d;
}

void KChatBase::init(bool noComboBox)
{
 kdDebug(11000) << "KChatBase: DESTRUCT (" << this << ")" << endl;

 d = new KChatBasePrivate;

 setMinimumWidth(100);
 setMinimumHeight(100);
 
 QVBoxLayout* l = new QVBoxLayout(this);

 d->mBox = new QListBox(this);
 connect(d->mBox, SIGNAL(rightButtonClicked(QListBoxItem*, const QPoint&)), 
		this, SIGNAL(rightButtonClicked(QListBoxItem*, const QPoint&)));
 l->addWidget(d->mBox);
 d->mBox->setVScrollBarMode(QScrollView::AlwaysOn);
 d->mBox->setHScrollBarMode(QScrollView::AlwaysOff);
 d->mBox->setFocusPolicy(QWidget::NoFocus);
// d->mBox->setSelectionMode(QListBox::NoSelection);
 d->mBox->setSelectionMode(QListBox::Single);

 l->addSpacing(5);

 QHBoxLayout* h = new QHBoxLayout(l);
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
	addSendingEntry(i18n("Send to all players"), SendToAll);//FIXME: where to put the id?
 }

 d->mAcceptMessage = true; // by default
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
// d->mCombo->insertItem(i18n("Send to all players"), SendToAll);
 return insertSendingEntry(text, id);
}

bool KChatBase::insertSendingEntry(const QString& text, int id, int index)
{
 if (!d->mCombo) {
	kdWarning(11000) << "KChatBase: Cannot add an entry to the combo box" << endl;
	return false;
 }
 if (d->mIndex2Id.findIndex(id) != -1) {
	kdError(11000) << "KChatBase: Cannot add more than one entry with the same ID! " << endl;
	kdError(11000) << "KChatBase: Text="<<text<<endl;
	return false;
 }
 d->mCombo->insertItem(text, index);
 if (index < 0) {
	d->mIndex2Id.append(id);
 } else {
	d->mIndex2Id.insert(d->mIndex2Id.at(index), id);
 }
 if (d->mIndex2Id.count() != (uint)d->mCombo->count()) {
	kdError(11000) << "KChatBase: internal ERROR - local IDs do not match combo box entries!" << endl;
 }
 return true;
}

int KChatBase::sendingEntry() const
{
 int index = d->mCombo->currentItem();
 if (!d->mIndex2Id.contains(index)) {
	kdWarning(11000) << "could not find the selected sending entry!" << endl;
	return -1;
 }
 return d->mIndex2Id[index];
}

void KChatBase::removeSendingEntry(int id)
{
 if (!d->mCombo) {
	kdWarning(11000) << "KChatBase: Cannot remove an entry from the combo box" << endl;
	return;
 }
 d->mIndex2Id.remove(id);
 d->mCombo->removeItem(findIndex(id));
}

void KChatBase::changeSendingEntry(const QString& text, int id)
{
 if (!d->mCombo) {
	kdWarning(11000) << "KChatBase: Cannot change an entry in the combo box" << endl;
	return;
 }
 int index = findIndex(id);
 d->mCombo->changeItem(text, index);
}

void KChatBase::setSendingEntry(int id)
{
 if (!d->mCombo) {
	kdWarning(11000) << "KChatBase: Cannot set an entry in the combo box" << endl;
	return;
 }
 d->mCombo->setCurrentItem(findIndex(id));
}
 
int KChatBase::findIndex(int id) const
{
 return d->mIndex2Id.findIndex(id);
}

int KChatBase::nextId() const
{
 int i = SendToAll + 1;
 while (d->mIndex2Id.findIndex(i) != -1) {
	i++;
 }
 return i;
}

void KChatBase::addItem(const QListBoxItem* text)
{
 d->mBox->insertItem(text); 
 int index = d->mBox->count() -1;
 d->mBox->setBottomItem(index);//FIXME: don't scroll to bottom if user scrolled down manually
}

void KChatBase::addMessage(const QString& fromName, const QString& text)
{
//maybe "%1 says: %2" or so
 addItem(layoutMessage(fromName, text));
}

QListBoxItem* KChatBase::layoutMessage(const QString& fromName, const QString& text)
{
 QListBoxItem* message;
 if (text.startsWith("/me ")) {
	// replace "/me" by a nice star. leave one space after the star
	QPixmap pix;
	pix.load(locate("data", QString::fromLatin1("kdegames/pics/star.png")));
	message = (QListBoxItem*)new QListBoxPixmap(pix, text.mid(3));
 } else {
	// not edited in any way. just return a text item
	message = (QListBoxItem*)new QListBoxText(i18n("%1: %2").arg(fromName).arg(text));
 }
 return message;
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
// connect(d->mEdit, SIGNAL(returnPressed(const QString&)), comp, SLOT(addItem(const QString&)));
 d->mEdit->clear();
 returnPressed(text);
}

QString KChatBase::comboBoxItem(const QString& name) const
{ // TODO: such a function for "send to all" and "send to my group"
 return i18n("Send to %1").arg(name);
}

void KChatBase::slotClear()
{
 d->mBox->clear();
}

void KChatBase::setCompletionMode(KGlobalSettings::Completion mode)
{ d->mEdit->setCompletionMode(mode); }

#include "kchatbase.moc"
