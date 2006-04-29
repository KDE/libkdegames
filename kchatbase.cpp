/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

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

#include <klineedit.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qcombobox.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3Frame>
#include <QHBoxLayout>
#include <QList>
#include <QVBoxLayout>
#include <kglobal.h>

class KChatBaseTextPrivate
{
public:
	KChatBaseTextPrivate()
	{
		mNameFont = 0;
		mMessageFont = 0;
	}

	QString mName;
	QString mMessage;

	const QFont* mNameFont;
	const QFont* mMessageFont;
};


KChatBaseText::KChatBaseText(const QString& name, const QString& message) : Q3ListBoxText()
{
 init();
 setName(name);
 setMessage(message);
}

KChatBaseText::KChatBaseText(const QString& message) : Q3ListBoxText()
{
 init();
 setMessage(message);
}

KChatBaseText::~KChatBaseText()
{
 delete d;
}

void KChatBaseText::init()
{
 d = new KChatBaseTextPrivate;
}

void KChatBaseText::setName(const QString& n)
{
// d->mName = n;
 d->mName = QString("%1: ").arg(n);
 setText(QString("%1: %2").arg(name()).arg(message())); // esp. for sorting
}

void KChatBaseText::setMessage(const QString& m)
{
 d->mMessage = m;
 setText(QString("%1: %2").arg(name()).arg(message())); // esp. for sorting
}

const QString& KChatBaseText::name() const
{ return d->mName; }

const QString& KChatBaseText::message() const
{ return d->mMessage; }

QFont KChatBaseText::nameFont() const
{
 if (d->mNameFont) {
	return *d->mNameFont;
 } else if (listBox()) {
	return listBox()->font();
 } else {
	return QFont();
 }
}

QFont KChatBaseText::messageFont() const
{
 if (d->mMessageFont) {
	return *d->mMessageFont;
 } else if (listBox()) {
	return listBox()->font();
 } else {
	return QFont();
 }
}

void KChatBaseText::setNameFont(const QFont* f)
{ d->mNameFont = f; }

void KChatBaseText::setMessageFont(const QFont* f)
{ d->mMessageFont = f; }

void KChatBaseText::paint(QPainter* painter)
{
 QFontMetrics fm = painter->fontMetrics();
 painter->setFont(nameFont());
 painter->drawText(3, fm.ascent() + fm.leading()/2, name());
 painter->setFont(messageFont());
 painter->drawText(3 + QFontMetrics(nameFont()).width(name()), fm.ascent() + fm.leading()/2, message());
}

int KChatBaseText::width(const Q3ListBox* lb) const
{
 int w = 0;
 if (lb) {
	w += 6;
	w += QFontMetrics(nameFont()).width(name());
	w += QFontMetrics(messageFont()).width(message());
 }
// int w = lb ? lb->fontMetrics().width( text() ) + 6 : 0; // QT orig
 return qMax(w, QApplication::globalStrut().width());
}

int KChatBaseText::height(const Q3ListBox* lb) const
{
 int h = 0;
 if (lb) {
	h += 2;
	// AB: is lineSpacing still correct?
	if (QFontMetrics(nameFont()).lineSpacing() > QFontMetrics(messageFont()).lineSpacing()) {
		h += QFontMetrics(nameFont()).lineSpacing();
	} else {
		h += QFontMetrics(messageFont()).lineSpacing();
	}
 }
// int h = lb ? lb->fontMetrics().lineSpacing() + 2 : 0; // QT orig
 return qMax(h, QApplication::globalStrut().height());
}



class KChatBasePrivate
{
public:
	KChatBasePrivate()
	{
		mBox = 0;
		mEdit = 0;
		mCombo = 0;

		mAcceptMessage = true;
		mMaxItems = -1;
	}
	Q3ListBox* mBox;
	KLineEdit* mEdit;
	QComboBox* mCombo;
	bool mAcceptMessage;
	int mMaxItems;

	QList<int> mIndex2Id;

	QFont mNameFont;
	QFont mMessageFont;
	QFont mSystemNameFont;
	QFont mSystemMessageFont;
};

KChatBase::KChatBase(QWidget* parent, bool noComboBox) : Q3Frame(parent)
{
 init(noComboBox);
}

KChatBase::~KChatBase()
{
// kDebug(11000) << "KChatBase: DESTRUCT (" << this << ")" << endl;
 saveConfig();
 delete d;
}

void KChatBase::init(bool noComboBox)
{
// kDebug(11000) << "KChatBase: INIT (" << this << ")" << endl;

 d = new KChatBasePrivate;

 setMinimumWidth(100);
 setMinimumHeight(150);

 QVBoxLayout* l = new QVBoxLayout(this);

 d->mBox = new Q3ListBox(this);
 connect(d->mBox, SIGNAL(rightButtonClicked(Q3ListBoxItem*, const QPoint&)),
		this, SIGNAL(rightButtonClicked(Q3ListBoxItem*, const QPoint&)));
 l->addWidget(d->mBox);
 d->mBox->setVScrollBarMode(Q3ScrollView::AlwaysOn);
 d->mBox->setHScrollBarMode(Q3ScrollView::AlwaysOff);
 d->mBox->setFocusPolicy(Qt::NoFocus);
// d->mBox->setSelectionMode(QListBox::NoSelection);
 d->mBox->setSelectionMode(Q3ListBox::Single);

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

 if (kapp) {
	// kapp might be NULL as well - in case we are in Qt designer.
	readConfig();
 }
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
	kWarning(11000) << "KChatBase: Cannot add an entry to the combo box" << endl;
	return false;
 }
 if (d->mIndex2Id.indexOf(id) != -1) {
	kError(11000) << "KChatBase: Cannot add more than one entry with the same ID! " << endl;
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
	kError(11000) << "KChatBase: internal ERROR - local IDs do not match combo box entries!" << endl;
 }
 return true;
}

int KChatBase::sendingEntry() const
{
 if (!d->mCombo) {
	kWarning(11001) << "Cannot retrieve index from NULL combo box" << endl;
	return -1;
 }
 int index = d->mCombo->currentIndex();
 if ( index > 0 && index <  d->mIndex2Id.size()) {
	kWarning(11000) << "could not find the selected sending entry!" << endl;
	return -1;
 }
 return d->mIndex2Id[index];
}

void KChatBase::removeSendingEntry(int id)
{
 if (!d->mCombo) {
	kWarning(11000) << "KChatBase: Cannot remove an entry from the combo box" << endl;
	return;
 }
 d->mCombo->removeItem(findIndex(id));
 d->mIndex2Id.removeAll(id);
}

void KChatBase::changeSendingEntry(const QString& text, int id)
{
 if (!d->mCombo) {
	kWarning(11000) << "KChatBase: Cannot change an entry in the combo box" << endl;
	return;
 }
 int index = findIndex(id);
 d->mCombo->setItemText(index, text);
}

void KChatBase::setSendingEntry(int id)
{
 if (!d->mCombo) {
	kWarning(11000) << "KChatBase: Cannot set an entry in the combo box" << endl;
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

void KChatBase::addItem(const Q3ListBoxItem* text)
{
 d->mBox->insertItem(text);
 int index = d->mBox->count() -1;
 d->mBox->setBottomItem(index);//FIXME: don't scroll to bottom if user scrolled down manually
 if (maxItems() >= 0 && d->mBox->count() > (unsigned int)maxItems()) {
	d->mBox->removeItem(0);
 }
}

void KChatBase::addMessage(const QString& fromName, const QString& text)
{
//maybe "%1 says: %2" or so
 addItem(layoutMessage(fromName, text));
}

void KChatBase::addSystemMessage(const QString& fromName, const QString& text)
{
 addItem(layoutSystemMessage(fromName, text));
}

Q3ListBoxItem* KChatBase::layoutMessage(const QString& fromName, const QString& text)
{
 //TODO: KChatBaseConfigure? - e.g. color
 Q3ListBoxItem* message;
 if (text.startsWith("/me ")) {
	// replace "/me" by a nice star. leave one space after the star
	QPixmap pix;
	pix.load(locate("data", QString::fromLatin1("kdegames/pics/star.png")));

	//TODO KChatBasePixmap? Should change the font here!

	message = (Q3ListBoxItem*)new Q3ListBoxPixmap(pix, i18n("%1 %2", fromName, text.mid(3)));
 } else {
	// the text is not edited in any way. just return an item
	KChatBaseText* m = new KChatBaseText(fromName, text);
	m->setNameFont(&d->mNameFont);
	m->setMessageFont(&d->mMessageFont);
	message = (Q3ListBoxItem*)m;
 }
 return message;
}

Q3ListBoxItem* KChatBase::layoutSystemMessage(const QString& fromName, const QString& text)
{
 //TODO: KChatBaseConfigure? - e.g. color

 // no need to check for /me etc.
 KChatBaseText* m = new KChatBaseText(i18n("--- %1", fromName), text);
 m->setNameFont(&d->mSystemNameFont);
 m->setMessageFont(&d->mSystemMessageFont);
 return (Q3ListBoxItem*)m;
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
 return i18n("Send to %1", name);
}

void KChatBase::slotClear()
{
 d->mBox->clear();
}

void KChatBase::setCompletionMode(KGlobalSettings::Completion mode)
{ d->mEdit->setCompletionMode(mode); }

void KChatBase::setNameFont(const QFont& font)
{
 d->mNameFont = font;
 d->mBox->triggerUpdate(false);
}

void KChatBase::setMessageFont(const QFont& font)
{
 d->mMessageFont = font;
 d->mBox->triggerUpdate(false);
}

void KChatBase::setBothFont(const QFont& font)
{
 setNameFont(font);
 setMessageFont(font);
}

const QFont& KChatBase::nameFont() const
{ return d->mNameFont; }

const QFont& KChatBase::messageFont() const
{ return d->mMessageFont; }

void KChatBase::setSystemNameFont(const QFont& font)
{
 d->mSystemNameFont = font;
 d->mBox->triggerUpdate(false);
}

void KChatBase::setSystemMessageFont(const QFont& font)
{
 d->mSystemMessageFont = font;
 d->mBox->triggerUpdate(false);
}

void KChatBase::setSystemBothFont(const QFont& font)
{
 setSystemNameFont(font);
 setSystemMessageFont(font);
}

const QFont& KChatBase::systemNameFont() const
{ return d->mSystemNameFont; }

const QFont& KChatBase::systemMessageFont() const
{ return d->mSystemMessageFont; }

void KChatBase::saveConfig(KConfig* conf)
{
 QString oldGroup;
 if (!conf) {
	conf = KGlobal::config();
	oldGroup = conf->group();
	conf->setGroup("KChatBase");
 }

 conf->writeEntry("NameFont", nameFont());
 conf->writeEntry("MessageFont", messageFont());
 conf->writeEntry("SystemNameFont", systemNameFont());
 conf->writeEntry("SystemMessageFont", systemMessageFont());
 conf->writeEntry("MaxMessages", maxItems());

 if (!oldGroup.isNull()) {
	conf->setGroup(oldGroup);
 }
}

void KChatBase::readConfig(KConfig* conf)
{
 QString oldGroup;
 if (!conf) {
	conf = KGlobal::config();
	oldGroup = conf->group();
	conf->setGroup("KChatBase");
 }

 setNameFont(conf->readEntry("NameFont", QFont()));
 setMessageFont(conf->readEntry("MessageFont", QFont()));
 setSystemNameFont(conf->readEntry("SystemNameFont", QFont()));
 setSystemMessageFont(conf->readEntry("SystemMessageFont", QFont()));
 setMaxItems(conf->readEntry("MaxMessages", -1));

 if (!oldGroup.isNull()) {
	conf->setGroup(oldGroup);
 }
}

void KChatBase::clear()
{
 d->mBox->clear();
}

void KChatBase::setMaxItems(int maxItems)
{
 d->mMaxItems = maxItems;
 //TODO cut too many messages
 if (maxItems == 0) {
	clear();
 } else if (maxItems > 0) {
	while (d->mBox->count() > (unsigned int)maxItems) {
		d->mBox->removeItem(0);
	}
 }
}

int KChatBase::maxItems() const
{ return d->mMaxItems; }


#include "kchatbase.moc"
