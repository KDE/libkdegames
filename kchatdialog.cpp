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

#include "kchatdialog.h"

#include "kchatbase.h"

#include <klocale.h>
#include <kfontdialog.h>

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QLineEdit>

class KChatDialogPrivate
{
 public:
	KChatDialogPrivate()
	{
		mTextPage = 0;

		mNamePreview = 0;
		mTextPreview = 0;
		mSystemNamePreview = 0;
		mSystemTextPreview = 0;

		mChat = 0;
	}

	QFrame* mTextPage;

	QLabel* mNamePreview;
	QLabel* mTextPreview;
	QLabel* mSystemNamePreview;
	QLabel* mSystemTextPreview;

	QLineEdit* mMaxMessages;

	KChatBase* mChat;
};

KChatDialog::KChatDialog(KChatBase* chat, QWidget* parent, bool modal) 
	: KDialog(parent)
{
 setCaption(i18n("Configure Chat"));
 setButtons(Ok|Default|Apply|Cancel);
 setModal(modal);
 init();
 plugChatWidget(chat);
}

KChatDialog::KChatDialog(QWidget* parent, bool modal) 
	: KDialog(parent)
{
 setCaption(i18n("Configure Chat"));
 setButtons(Ok|Default|Apply|Cancel);
 setModal(modal);
 init();
//  init();
}

KChatDialog::~KChatDialog()
{
 delete d;
}

void KChatDialog::init()
{
 d = new KChatDialogPrivate;
 d->mTextPage = new QFrame( this );
 setMainWidget( d->mTextPage );
 QGridLayout* layout = new QGridLayout(d->mTextPage);
 layout->setMargin(KDialog::marginHint());
 layout->setSpacing(KDialog::spacingHint());

// General fonts
 QPushButton* nameFont = new QPushButton(i18n("Name Font..."), d->mTextPage);
 connect(nameFont, SIGNAL(pressed()), this, SLOT(slotGetNameFont()));
 layout->addWidget(nameFont, 0, 0);
 QPushButton* textFont = new QPushButton(i18n("Text Font..."), d->mTextPage);
 connect(textFont, SIGNAL(pressed()), this, SLOT(slotGetTextFont()));
 layout->addWidget(textFont, 0, 1);

 QFrame* messagePreview = new QFrame(d->mTextPage);
 messagePreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
 QHBoxLayout* messageLayout = new QHBoxLayout(messagePreview);
 layout->addWidget(messagePreview, 1, 0, 1, 2);

 d->mNamePreview = new QLabel(i18n("Player: "), messagePreview);
 messageLayout->addWidget(d->mNamePreview, 0);
 d->mTextPreview = new QLabel(i18n("This is a player message"), messagePreview);
 messageLayout->addWidget(d->mTextPreview, 1);

 layout->addItem(new QSpacerItem(0, 10), 2, 0);
 
// System Message fonts
 QLabel* systemMessages = new QLabel(i18n("System Messages - Messages directly sent from the game"), d->mTextPage);
 layout->addWidget(systemMessages, 3, 0, 1, 2);
 QPushButton* systemNameFont = new QPushButton(i18n("Name Font..."), d->mTextPage);
 connect(systemNameFont, SIGNAL(pressed()), this, SLOT(slotGetSystemNameFont()));
 layout->addWidget(systemNameFont, 4, 0);
 QPushButton* systemTextFont = new QPushButton(i18n("Text Font..."), d->mTextPage);
 connect(systemTextFont, SIGNAL(pressed()), this, SLOT(slotGetSystemTextFont()));
 layout->addWidget(systemTextFont, 4, 1);

 QFrame* systemMessagePreview = new QFrame(d->mTextPage);
 systemMessagePreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
 QHBoxLayout* systemMessageLayout = new QHBoxLayout(systemMessagePreview);
 layout->addWidget(systemMessagePreview, 5, 0, 1, 2);
 
 d->mSystemNamePreview = new QLabel(i18n("--- Game: "), systemMessagePreview);
 systemMessageLayout->addWidget(d->mSystemNamePreview, 0);
 d->mSystemTextPreview = new QLabel(i18n("This is a system message"), systemMessagePreview);
 systemMessageLayout->addWidget(d->mSystemTextPreview, 1);

// message count
 QLabel* maxMessages = new QLabel(i18n("Maximum number of messages (-1 = unlimited):"), d->mTextPage);
 layout->addWidget(maxMessages, 6, 0);
 d->mMaxMessages = new QLineEdit(d->mTextPage);
 d->mMaxMessages->setText(QString::number(-1));
 layout->addWidget(d->mMaxMessages, 6, 1);
 connect(this, SIGNAL(applyClicked()),this,SLOT(slotApply()));
 connect(this, SIGNAL(okClicked()),this,SLOT(slotOk()));
}

void KChatDialog::slotGetNameFont()
{
 QFont font = nameFont();
 KFontDialog::getFont(font);
 setNameFont(font);
}

void KChatDialog::slotGetTextFont()
{
 QFont font = textFont();
 KFontDialog::getFont(font);
 setTextFont(font);
}

void KChatDialog::slotGetSystemNameFont()
{
 QFont font = systemNameFont();
 KFontDialog::getFont(font);
 setSystemNameFont(font);
}

void KChatDialog::slotGetSystemTextFont()
{
 QFont font = systemTextFont();
 KFontDialog::getFont(font);
 setSystemTextFont(font);
}

QFont KChatDialog::nameFont() const
{
 return d->mNamePreview->font();
}

QFont KChatDialog::textFont() const
{
 return d->mTextPreview->font();
}

QFont KChatDialog::systemNameFont() const
{
 return d->mSystemNamePreview->font();
}

QFont KChatDialog::systemTextFont() const
{
 return d->mSystemTextPreview->font();
}

void KChatDialog::plugChatWidget(KChatBase* widget, bool applyFonts)
{
 d->mChat = widget;
 if (applyFonts && d->mChat) {
	setNameFont(d->mChat->nameFont());
	setTextFont(d->mChat->messageFont());
	setSystemNameFont(d->mChat->systemNameFont());
	setSystemTextFont(d->mChat->systemMessageFont());
	setMaxMessages(d->mChat->maxItems());
 }
}

void KChatDialog::configureChatWidget(KChatBase* widget)
{
 if (!widget) {
	return;
 }
 widget->setNameFont(nameFont());
 widget->setMessageFont(textFont());

 widget->setSystemNameFont(systemNameFont());
 widget->setSystemMessageFont(systemTextFont());
 
 widget->setMaxItems(maxMessages());
 widget->saveConfig();
 kDebug() << "Saved configuration";
}

void KChatDialog::slotOk()
{
 slotApply();
 KDialog::accept();
}

void KChatDialog::slotApply()
{
 configureChatWidget(d->mChat);
}

void KChatDialog::setNameFont(QFont f)
{
 d->mNamePreview->setFont(f);
}

void KChatDialog::setTextFont(QFont f)
{
 d->mTextPreview->setFont(f);
}

void KChatDialog::setSystemNameFont(QFont f)
{
 d->mSystemNamePreview->setFont(f);
}

void KChatDialog::setSystemTextFont(QFont f)
{
 d->mSystemTextPreview->setFont(f);
}

void KChatDialog::setMaxMessages(int max)
{
 d->mMaxMessages->setText(QString::number(max));
}

int KChatDialog::maxMessages() const
{
 bool ok;
 int max = d->mMaxMessages->text().toInt(&ok);
 if (!ok) {
	return -1; // unlimited is default
 }
 return max;
}

#include "kchatdialog.moc"
