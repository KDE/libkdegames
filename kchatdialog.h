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

#ifndef __KCHATDIALOG_H__
#define __KCHATDIALOG_H__

#include <kdialogbase.h>

class KChatBase;

class KChatDialogPrivate;

class KChatDialog : public KDialogBase
{
	Q_OBJECT
public:
	/**
	 * Construct a KChatDialog widget
	 **/
	KChatDialog(QWidget* parent, bool modal = false);

	/**
	 * Construct a KChatDialog widget which automatically configures the
	 * @ref KChatBase widget. You probably want to use this as you don't
	 * have to care about the configuration stuff yourself.
	 **/
	KChatDialog(KChatBase* chatWidget, QWidget* parent, bool modal = false);

	/**
	 * Destruct the dialog
	 **/
	~KChatDialog();

	/**
	 * @return The font that shall be used as the "name: " part of a normal
	 * message.
	 **/
	QFont nameFont() const;

	/**
	 * @return The font that shall be used for normal messages.
	 **/
	QFont textFont() const;
	
	/**
	 * @return The font that shall be used as the "name: " part of a system
	 * (game) message.
	 **/
	QFont systemNameFont() const;
	
	/**
	 * @return The font that shall be used for a system (game) message.
	 **/
	QFont systemTextFont() const;

	/**
	 * Set the widget that will be configured by the dialog. Use this if you
	 * don't want to configure the widget yourself.
	 * @param widget The chat widget that shall be configured
	 * @param Whether you want to have the current @ref KChatBase fonts as
	 * defaults in the dialog
	 **/
	void plugChatWidget(KChatBase* widget, bool applyFonts = true);

	/**
	 * Used to configure the chat widget according to the user settings.
	 * This is called automatically if @ref plugChatWidget was called
	 * before.
	 * @param widget The chat widget that shall be configured
	 **/
	void configureChatWidget(KChatBase* widget);

protected slots:
	void slotGetNameFont();
	void slotGetTextFont();
	void slotGetSystemNameFont();
	void slotGetSystemTextFont();

	virtual void slotApply();
	virtual void slotOk();

private:
	void setNameFont(QFont);
	void setTextFont(QFont);
	void setSystemNameFont(QFont);
	void setSystemTextFont(QFont);

private:
	void init();

private:
	KChatDialogPrivate* d;
};

#endif
