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

#ifndef __KCHATBASE_H__
#define __KCHATBASE_H__

#include <qframe.h>
#include <qstring.h>

#include <kglobalsettings.h>

class QListBoxItem;

class KChatBasePrivate;

/**
 * This is the base class for both @ref KChat and @ref KGameChat. @ref KGameChat is the class
 * you want to use if you write a @ref KGame based game as it will do most things for
 * you. @ref KChat is more or less the same but not KGame dependant
 *
 * KChatBase provides a complete chat widget, featuring different sending means
 * (e.g. "send to all", "send to player1", "send to group2" and so on - see @ref
 * addSendingEntry). It also provides full auto-completion capabilities (see
 * @ref KCompletion and @ref KLineEdit) which defaults to disabled. The user can
 * change this by right-clicking on the @ref KLineEdit widget and selecting the
 * desired behaviour. You can also change this manually by calling @ref
 * setCompletionMode.
 *
 * To make KChatBase useful you have to overwrite at least @ref returnPressed.
 * Here you should send the message to all of your clients (or just some of
 * them, depending on @ref sendingEntry).
 *
 * To add a message just call @ref addMessage with the nickname of the player
 * who sent the message and the message itself. If you don't want to use @ref
 * layoutMessage by any reason you can also call @ref addItem directly. But you
 * should better replace @ref layoutMessage instead.
 *
 * You probably don't want to use the abstract class KChatBase directly but use
 * one of the derived classess @ref KChat or @ref KGameChat. The latter is the
 * widget of choice if you develop a @ref KGame application as you don't have to
 * do anything but providing a @ref KGame object.
 *
 * @short The base class for chat widgets
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KChatBase : public QFrame
{
	Q_OBJECT
public:
	/**
	 * @param noComboBox If true then the combo box where the player can
	 * choose where to send messages to (either globally or just to some
	 * players) will not be added.
	 **/
	KChatBase(QWidget* parent, bool noComboBox = false);

	~KChatBase();

	enum SendingIds {
		SendToAll = 0
	};

	/**
	 * @return The name that will be shown for messages from this widget. Either the
	 * string that was set by @ref setFromName or the name of the player
	 * that was set by @ref setFromPlayer
	 **/
	virtual const QString& fromName() const = 0;

	/**
	 * Adds a new entry in the combo box. The default is "send to all
	 * players" only. This function is provided for convenience. You can
	 * also call @ref inserSendingEntry with index = -1.
	 * See also @ref nextId!
	 * @param text The text of the new entry
	 * @param id An ID for this entry. This must be unique for this
	 * entry. It has nothing to do with the position of the entry in the
	 * combo box! See @ref nextId
	 * @return True if successfull, otherwise false (e.g. if the id is already used)
	 **/
	bool addSendingEntry(const QString& text, int id);

	/**
	 * Inserts a new entry in the combo box.
	 * @param text The entry
	 * @param id An ID for this entry. This must be unique for this
	 * entry. It has nothing to do with the position of the entry in the
	 * combo box! See @ref nextId
	 * @param index The position of the entry. If -1 the entry will be added
	 * at the bottom
	 * @return True if successfull, otherwise false (e.g. if the id is already used)
	 **/
	bool insertSendingEntry(const QString& text, int id, int index = -1);

	/**
	 * This changes a combo box entry.
	 * @param text The new text of the entry
	 * @param id The ID of the item to be changed
	 **/
	void changeSendingEntry(const QString& text, int id);

	/**
	 * Removes the entry with the ID id from the combo box. Note that id is
	 * _not_ the index of the entry! See also @ref addSendingEntry
	 * @param id The unique id of the entry
	 **/
	void removeSendingEntry(int id);

	/**
	 * @return The _unique ID_ of the sending entry that has been selected.
	 * See also @ref addSendingEntry
	 **/
	int sendingEntry() const;
	
	/**
	 * @return The index of the combo box entry with the given id
	 **/
	int findIndex(int id) const;

	/**
	 * @return An ID that has not yet been used in the combo box. See also
	 * @ref addSendingEntry
	 **/
	int nextId() const;

	/**
	 * @return True if this widget is able to send messages (see @ref
	 * returnPressed) and false if not. The default implementation returns
	 * the value which has been set by @ref setAcceptMessage (true by
	 * default)
	 **/
	virtual bool acceptMessage() const;

	/**
	 * See @ref KLineEdit::setCompletionMode
	 **/
	void setCompletionMode(KGlobalSettings::Completion mode);

public slots:
	/**
	 * Adds a text in the listbox. See also @ref signalSendMessage()
	 *
	 * Maybe you want to replace this with a function that creates a nicer text
	 * than "fromName: text"
	 *
	 * Update: the function @ref layoutMessage is called by this now. This
	 * means that you will get user defined outlook on the messages :-)
	 * @param text The text to be added 
	 * @param fromName The player who sent this message
	 **/
	virtual void addMessage(const QString& fromName, const QString& text);


	/**
	 * This member function is mainly internally used to add a message. It
	 * is called by @ref addMessage which creates a single text from a
	 * player name and a text. You will hardly ever use this - but if you
	 * need it it will be here ;-)
	 *
	 * But you may want to replace this in a derived class to create a
	 * non-default (maybe nicer ;-) ) behaviour
	 * @param item The @ref QListBoxItem that is being added
	 **/
	virtual void addItem(const QListBoxItem* item);


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
	
signals:
	/**
	 * Emitted when the user right-clicks on a list item. See also @ref
	 * QListBox::rightButtonClicked
	 **/
	void rightButtonClicked(QListBoxItem*, const QPoint&);

protected:
	/**
	 * This is called whenever the user pushed return ie wants to send a
	 * message.
	 *
	 * Note that you MUST add the message to the widget when this function
	 * is called as it has already been added to the @ref KCompletion object
	 * of the @ref KLineEdit widget!
	 *
	 * Must be implemented in derived classes
	 * @param text The message to be sent
	 **/
	virtual void returnPressed(const QString& text) = 0;

	/**
	 * Replace to costumize the combo box.
	 *
	 * Default: i18n("Send to %1).arg(name)
	 * @param name The name of the player
	 * @return The string as it will be shown in the combo box
	 **/
	virtual QString comboBoxItem(const QString& name) const;

	/**
	 * Create a @ref QListBoxItem for this message. This function is not yet
	 * written usefully - currently just a @ref QListBoxTex object is
	 * created which shows the message in this format: "fromName: text".
	 * This should fit most peoples needs but needs further improvements.
	 **/
	virtual QListBoxItem* layoutMessage(const QString& fromName, const QString& text);

private slots:
	/**
	 * Check if a text was entered and if @ref acceptMessage returns true. 
	 * Then add the message to the @ref KCompletion object of the @ref KLineEdit
	 * widget and call @ref returnPressed
	 **/
	void slotReturnPressed(const QString&);

private:
	void init(bool noComboBox);

	KChatBasePrivate* d;
};

#endif
