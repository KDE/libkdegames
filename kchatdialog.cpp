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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kchatdialog.h"

#include "kchatbase.h"

#include <klocale.h>
#include <kfontdialog.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

class KChatDialogPrivate
{
 public:
	KChatDialogPrivate()
	{
		mMessagePage = 0;

		mMessagePreview = 0;
		mSystemMessagePreview = 0;

		mChat = 0;
	}

	QFrame* mMessagePage;

	QLabel* mMessagePreview;
	QLabel* mSystemMessagePreview;

	KChatBase* mChat;

};

KChatDialog::KChatDialog(KChatBase* chat, QWidget* parent, bool modal) 
	: KDialogBase(Tabbed, i18n("Configure Chat"), Ok|Default|Apply|Cancel, Ok, parent, 0, modal, true)
{
 init();
 plugChatWidget(chat);
}

KChatDialog::KChatDialog(QWidget* parent, bool modal) 
	: KDialogBase(Tabbed, i18n("Configure Chat"), Ok|Default|Apply|Cancel, Ok, parent, 0, modal, true)
{
 init();
}

KChatDialog::~KChatDialog()
{
 delete d;
}

void KChatDialog::init()
{
 d = new KChatDialogPrivate;
 d->mMessagePage = addPage(i18n("&Messages"));// not a good name - game Messages?
 QGridLayout* layout = new QGridLayout(d->mMessagePage, 6, 2, KDialog::marginHint(), KDialog::spacingHint());

 QPushButton* nameFont = new QPushButton(i18n("Name Font..."), d->mMessagePage);
 connect(nameFont, SIGNAL(pressed()), this, SLOT(slotGetNameFont()));
 layout->addWidget(nameFont, 0, 0);
 QPushButton* textFont = new QPushButton(i18n("Text Font..."), d->mMessagePage);
 connect(textFont, SIGNAL(pressed()), this, SLOT(slotGetTextFont()));
 layout->addWidget(textFont, 0, 1);

 d->mMessagePreview = new QLabel(i18n("Player: This is a player message"), d->mMessagePage);
 d->mMessagePreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
 layout->addMultiCellWidget(d->mMessagePreview, 1, 1, 0, 1);
 layout->addRowSpacing(2, 10);
 

 QLabel* systemMessages = new QLabel(i18n("System Messages - Messages directly sent from the game"), d->mMessagePage);
 layout->addMultiCellWidget(systemMessages, 3, 3, 0, 1);
 QPushButton* systemNameFont = new QPushButton(i18n("Name Font..."), d->mMessagePage);
 connect(systemNameFont, SIGNAL(pressed()), this, SLOT(slotGetSystemNameFont()));
 layout->addWidget(systemNameFont, 4, 0);
 QPushButton* systemTextFont = new QPushButton(i18n("Text Font..."), d->mMessagePage);
 connect(systemTextFont, SIGNAL(pressed()), this, SLOT(slotGetSystemTextFont()));
 layout->addWidget(systemTextFont, 4, 1);

 d->mSystemMessagePreview = new QLabel(i18n("--- Game: This is a system message"), d->mMessagePage);
 d->mSystemMessagePreview->setFrameShadow(QFrame::Sunken);
 d->mSystemMessagePreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
 layout->addMultiCellWidget(d->mSystemMessagePreview, 5, 5, 0, 1);
 

}

void KChatDialog::slotGetNameFont()
{
//TODO
 slotGetTextFont();
}

void KChatDialog::slotGetTextFont()
{
 QFont font = d->mMessagePreview->font();
 KFontDialog::getFont(font);
 d->mMessagePreview->setFont(font);
}

void KChatDialog::slotGetSystemNameFont()
{
//TODO
 slotGetSystemTextFont();
}

void KChatDialog::slotGetSystemTextFont()
{
 QFont font = d->mSystemMessagePreview->font();
 KFontDialog::getFont(font);
 d->mSystemMessagePreview->setFont(font);
}

QFont KChatDialog::nameFont() const
{
 //TODO
 return textFont();
}

QFont KChatDialog::textFont() const
{
 return d->mMessagePreview->font();
}

QFont KChatDialog::systemNameFont() const
{
 //TODO
 return systemTextFont();
}

QFont KChatDialog::systemTextFont() const
{
 return d->mSystemMessagePreview->font();
}

void KChatDialog::plugChatWidget(KChatBase* widget, bool applyFonts)
{
 d->mChat = widget;
 if (applyFonts && d->mChat) {
	setNameFont(d->mChat->nameFont());
	setTextFont(d->mChat->messageFont());
	setSystemNameFont(d->mChat->systemNameFont());
	setSystemTextFont(d->mChat->systemMessageFont());
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

// widget->repaintBox();
}

void KChatDialog::slotOk()
{
 slotApply();
 KDialogBase::slotOk();
}

void KChatDialog::slotApply()
{
 configureChatWidget(d->mChat);
}

void KChatDialog::setNameFont(QFont f)
{
 //TODO
 setTextFont(f);
}

void KChatDialog::setTextFont(QFont f)
{
 d->mMessagePreview->setFont(f);
}

void KChatDialog::setSystemNameFont(QFont f)
{
 //TODO
 setSystemTextFont(f);
}

void KChatDialog::setSystemTextFont(QFont f)
{
 d->mSystemMessagePreview->setFont(f);
}

#include "kchatdialog.moc"
