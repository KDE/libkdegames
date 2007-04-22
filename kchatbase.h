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
#ifndef __KCHATBASE_H__
#define __KCHATBASE_H__

#include <QtGui/QFrame>

#include <kglobalsettings.h>
#include <libkdegames_export.h>

class QListWidgetItem;

class KConfig;

class KChatBasePrivate;
class KChatBaseModel;
class KChatBaseItemDelegate;

/**
 * @short The base class for chat widgets
 *
 * This is the base class for both KChat and KGameChat. KGameChat is the class
 * you want to use if you write a KGame based game as it will do most things for
 * you. KChat is more or less the same but not KGame dependant
 *
 * KChatBase provides a complete chat widget, featuring different sending means
 * (e.g. "send to all", "send to player1", "send to group2" and so on - see 
 * addSendingEntry). It also provides full auto-completion capabilities (see
 * KCompletion and KLineEdit) which defaults to disabled. The user can
 * change this by right-clicking on the KLineEdit widget and selecting the
 * desired behaviour. You can also change this manually by calling 
 * setCompletionMode.
 *
 * To make KhatBase useful you have to overwrite at least returnPressed.
 * Here you should send the message to all of your clients (or just some of
 * them, depending on sendingEntry).
 *
 * To add a message just call addMessage with the nickname of the player
 * who sent the message and the message itself. 
 *
 * You probably don't want to use the abstract class KChatBase directly but use
 * one of the derived classes KChat or KGameChat. The latter is the
 * widget of choice if you develop a KGame application as you don't have to
 * do anything but providing a KGame object. If you want to change the kind of
 * elements displayed (using pixmaps for example), then you will also have to
 * derive the KChatBaseModel and KChatBaseItemDelegate classes.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @author Gael de Chalendar (aka Kleag) <kleag@free.fr> for the port to Model/View
 * 
 **/
class KDEGAMES_EXPORT KChatBase : public QFrame
{
	Q_OBJECT
public:
	/**
	 * @param parent The parent widget for this widget.
         * @param model
         * @param delegate
	 * @param noComboBox If true then the combo box where the player can
	 * choose where to send messages to (either globally or just to some
	 * players) will not be added.
	 **/
	KChatBase(QWidget* parent, KChatBaseModel* model=0, 
		   KChatBaseItemDelegate* delegate=0, bool noComboBox = false);

	/**
	 * Destruct the KChatBase object
	 *
	 * Also calls saveConfig
	 **/
	virtual ~KChatBase();

	enum SendingIds {
		SendToAll = 0
	};

	/**
	 * @return The name that will be shown for messages from this widget. Either the
	 * string that was set by setFromName or the name of the player
	 * that was set by setFromPlayer
	 **/
	virtual QString fromName() const = 0;

	/**
	 * Adds a new entry in the combo box. The default is "send to all
	 * players" only. This function is provided for convenience. You can
	 * also call inserSendingEntry with index = -1.
	 * See also nextId!
	 * @param text The text of the new entry
	 * @param id An ID for this entry. This must be unique for this
	 * entry. It has nothing to do with the position of the entry in the
	 * combo box. See nextId
	 * @return True if successful, otherwise false (e.g. if the id is already used)
	 **/
	bool addSendingEntry(const QString& text, int id);

	/**
	 * Inserts a new entry in the combo box.
	 * @param text The entry
	 * @param id An ID for this entry. This must be unique for this
	 * entry. It has nothing to do with the position of the entry in the
	 * combo box! 
	 * @see nextId
	 * @param index The position of the entry. If -1 the entry will be added
	 * at the bottom
	 * @return True if successful, otherwise false (e.g. if the id is already used)
	 **/
	bool insertSendingEntry(const QString& text, int id, int index = -1);

	/**
	 * This changes a combo box entry.
	 * @param text The new text of the entry
	 * @param id The ID of the item to be changed
	 **/
	void changeSendingEntry(const QString& text, int id);

	/**
	 * This selects a combo box entry.
	 * @param id The ID of the item to be selected
	 **/
	void setSendingEntry(int id);

	/**
	 * Removes the entry with the ID id from the combo box. Note that id is
	 * _not_ the index of the entry! 
	 * @see addSendingEntry
	 * @param id The unique id of the entry
	 **/
	void removeSendingEntry(int id);

	/**
	 * @return The _unique ID_ of the sending entry that has been selected.
	 * @see addSendingEntry
	 *
	 * Note that the entry "send to all" _always_ uses
	 * KChatBase::SendToAll, i.e. 0 as id!
	 **/
	int sendingEntry() const;
	
	/**
	 * @return The index of the combo box entry with the given id
	 **/
	int findIndex(int id) const;

	/**
	 * @return An ID that has not yet been used in the combo box.
	 * @see addSendingEntry
	 **/
	int nextId() const;

	/**
	 * @return True if this widget is able to send messages (see 
	 * returnPressed) and false if not. The default implementation returns
	 * the value which has been set by setAcceptMessage (true by
	 * default)
	 **/
	virtual bool acceptMessage() const;

	/**
	 * See KLineEdit::setCompletionMode
	 **/
	void setCompletionMode(KGlobalSettings::Completion mode);

	/**
	 * Set the font that used used for the name part of a message. See also
	 * nameFont and setBothFont
	 **/
	void setNameFont(const QFont& font);
	
	/**
	 * Set the font that used used for the message part of a message.
	 * @see messageFont, setBothFont
	 **/
	void setMessageFont(const QFont& font);

	/**
	 * This sets both - nameFont and messageFont to font. You
	 * probably want to use this if you don't wish to distinguish between
	 * these parts of a message.
	 * @param font A font used for both nameFont and messageFont
	 **/
	void setBothFont(const QFont& font);

	/**
	 * Same as setNameFont but applies only to system messages.
	 * @see layoutSystemMessage
	 **/
	void setSystemNameFont(const QFont& font);

	/**
	 * Same as setMessageFont but applies only to system messages.
	 * @see layoutSystemMessage
	 **/
	void setSystemMessageFont(const QFont& font);

	/**
	 * Same as setBothFont but applies only to system messages.
	 * @see layoutSystemMessage
	 **/
	void setSystemBothFont(const QFont& font);

	/**
	 * This font should be used for the name (the "from: " part) of a
	 * message. layoutMessage uses this to set the font using
	 * KChatBaseItemDelegate::setNameFont but if you want to overwrite 
	 * layoutMessage you should do this yourself.
	 * @return The font that is used for the name part of the message.
	 **/
	const QFont& nameFont() const;

	/**
	 * This font should be used for a message. layoutMessage sets the
	 * font of a message using KChatBaseItemDelegate::setMessageFont but if ypu
	 * replace layoutMessage with your own function you should use
	 * messageFont() yourself.
	 * @return The font that is used for a message
	 **/
	const QFont& messageFont() const;

	/**
	 * Same as systemNameFont but applies only to system messages.
	 * @see layoutSystemMessage
	 **/
	const QFont& systemNameFont() const;

	/**
	 * Same as systemMessageFont but applies only to system messages.
	 * @see layoutSystemMessage
	 **/
	const QFont& systemMessageFont() const;

	/**
	 * Save the configuration of the dialog to a KConfig object. If
	 * the supplied KConfig pointer is NULL then KGlobal::config() is used
	 * instead (and the group is changed to "KChatBase") butr the current
	 * group is restored at the end.
	 * @param conf A pointer to the KConfig object to save the config
	 * to. If you use 0 then KGlobal::config() is used and the group is changed
	 * to "KChatBase" (the current group is restored at the end).
	 **/
	virtual void saveConfig(KConfig* conf = 0);

	/**
	 * Read the configuration from a KConfig object. If the pointer is
	 * NULL KGlobal::config() is used and the group is changed to "KChatBase".
	 * The current KConfig::group is restored after this call.
	 **/
	virtual void readConfig(KConfig* conf = 0);

	/**
	 * Set the maximum number of items in the list. If the number of item
	 * exceeds the maximum as many items are deleted (oldest first) as
	 * necessary. The number of items will never exceed this value.
	 * @param maxItems the maximum number of items. -1 (default) for
	 * unlimited.
	 **/
	void setMaxItems(int maxItems);

	/**
	 * Clear all messages in the list.
	 **/
	void clear();

	/**
	 * @return The maximum number of messages in the list. -1 is unlimited. See also
	 * setMaxItems
	 **/
	int maxItems() const;

	 KChatBaseModel* model();
	 void setModel(KChatBaseModel* m);

public Q_SLOTS:
	/**
	 * Add a text in the listbox. See also signalSendMessage()
	 *
	 * Maybe you want to replace this with a function that creates a nicer text
	 * than "fromName: text"
	 *
	 * Update: the function layoutMessage is called by this now. This
	 * means that you will get user defined outlook on the messages :-)
	 * @param fromName The player who sent this message
	 * @param text The text to be added 
	 **/
	virtual void addMessage(const QString& fromName, const QString& text);

	/**
	 * This works just like addMessage but adds a system message. 
	 * layoutSystemMessage is used to generate the displayed item. System
	 * messages will have a different look than player messages.
	 *
	 * You may wish to  use this to display status information from your game.
	 **/
	virtual void addSystemMessage(const QString& fromName, const QString& text);

	/**
	 * This clears all messages in the view. Note that only the messages are
	 * cleared, not the sender names in the combo box!
	 **/
	void slotClear();

	/**
	 * @param a If false this widget cannot send a message until
	 * setAcceptMessage(true) is called
	 **/
	void setAcceptMessage(bool a);
	
Q_SIGNALS:
	/**
	 * Emitted when the user right-clicks on a list item. 
	 * @see QListBox::rightButtonClicked
	 **/
	void rightButtonClicked(QListWidgetItem*, const QPoint&);

protected:
	/**
	 * This is called whenever the user pushed return ie wants to send a
	 * message.
	 *
	 * Note that you MUST add the message to the widget when this function
	 * is called as it has already been added to the KCompletion object
	 * of the KLineEdit widget!
	 *
	 * Must be implemented in derived classes
	 * @param text The message to be sent
	 **/
	virtual void returnPressed(const QString& text) = 0;

	/**
	 * Replace to customize the combo box.
	 *
	 * Default: i18n("Send to %1).arg(name)
	 * @param name The name of the player
	 * @return The string as it will be shown in the combo box
	 **/
	virtual QString comboBoxItem(const QString& name) const;

private Q_SLOTS:
	/**
	 * Check if a text was entered and if acceptMessage returns true. 
	 * Then add the message to the KCompletion object of the KLineEdit
	 * widget and call returnPressed
	 **/
	void slotReturnPressed(const QString&);

private:

	KChatBasePrivate* d;
};

#endif
