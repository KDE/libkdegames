/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)

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

// NAMING
// please follow these naming rules if you add/change classes:
// the main dialog is named KGameDialog and the base config widget
// KGameDialogConfig. All config widgets are named KGameDialogXYZConfig (where
// XYZ = the name of the config widget, like "general" or "network") and are
// inherited from KGameDialogConfig.

#ifndef __KGAMEDIALOGCONFIG_H__
#define __KGAMEDIALOGCONFIG_H__

#include <qwidget.h>
#include <kdebug.h>

class QGridLayout;
class QVBoxLayout;
class QListBoxItem;

class KGame;
class KPlayer;
class KGamePropertyBase;

class KGameDialogConfigPrivate;
/**
 * Base class for configuration widgets.
 *
 * You can inherit from this and implement @ref submitToKGame, @ref
 * setOwner and @ref setKGame to create your personal @ref KGame configuration widget :-)
 * @short Base class for configuration widgets
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameDialogConfig : public QWidget
{
	Q_OBJECT
public:
	KGameDialogConfig(QWidget* parent = 0);
	virtual ~KGameDialogConfig();

	/**
	 * Called by @ref KGameDialog to submit all settings to the KGame
	 * Object.
	 * You have to replace this if you add your own widgets!
	 * @param g A pointer to your KGame.
	 * @param p A pointer to the player owning this dialog
	 **/
	virtual void submitToKGame(KGame* g, KPlayer* p) = 0;

	/**
	 * The owner player of the dialog has been changed. The default
	 * changes the pointer for @ref owner so don't forget to call the
	 * default implementation if you overwrite this!
	 *
	 * You can use this e.g. to change a line edit widget containing the 
	 * player name.
	 *
	 * Note: even NULL players are allowed!
	 * @param p The new owner player of the dialog
	 **/
	virtual void setOwner(KPlayer* p);

	/**
	 * The KGame object of the dialog has been changed. The default
	 * implementation changes the pointer for @ref game so don't forget to
	 * call the default implementation if you overwrite this!
	 *
	 * You can use this e.g. to re-read the min/max player settings.
	 * @param p The new owner player of the dialog
	 **/
	virtual void setKGame(KGame* g);

	/**
	 * The admin status has been changed.
	 * If the KGame object of this config widget is the
	 * admin the user is allowed to configure it. Otherwise most
	 * widgets will have to be disabled. Note that you don't necessarily
	 * need to deactivate all widget - e.g. the player name must be
	 * configured by the player. Mainly the KGame configuration can be done
	 * by the admin only.
	 *
	 * By default this does nothing. Changes the value for @ref admin so 
	 * don't forget to call the default implementation in derived classes!
	 * @param admin Whether the KGame object of this dialog can be
	 * configured
	 **/
	virtual void setAdmin(bool admin);

	/**
	 * A pointer to the @ref KGame object that has been set by @ref setKGame.
	 *
	 * Note that NULL is allowed!
	 * @return The @ref KGame object assigned to this dialog
	 **/
	KGame* game() const;

	/**
	 * A pointer to the @ref KPlayer object that has been set by @ref
	 * setOwner.
	 *
	 * Note that NULL is allowed!
	 * @return The owner of the dialog
	 **/
	KPlayer* owner() const;

	/**
	 * @return True if the @ref owner is ADMIN otherwise FALSE. See also
	 * @ref setAdmin
	 **/
	bool admin() const;


protected:

private:
	KGameDialogConfigPrivate* d;
};

/**
 * The main game configuration widget.
 * 
 * It currently contains a line edit for the name of the player only. You can
 * add widgets by using the KGameDialogGeneralConfig as parent parameter as it
 * uses @ref QLayout::autoAdd == true.
 * @shot The main @ref KGameDialog configuration widget
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameDialogGeneralConfigPrivate;
class KGameDialogGeneralConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	/**
	 * Construct a KGameDialogGeneralConfig. Currently it contains a line
	 * edit widget to change the player name only.
	 *
	 * @param initializeGUI If you really don't want to use the 
	 * predefined widget and/or layout use FALSE here. Note that then none
	 * of the predefined widgets (currently only the name of the player) 
	 * will exist anymore.
	 *
	 * If you just want to add more widgets you can just create your widgets
	 * with the KGameDialogGeneralConfig as parent as it uses @ref
	 * QLayout::setAutoAdd(true).
	 **/
	KGameDialogGeneralConfig(QWidget* parent = 0, bool initializeGUI = true);
	virtual ~KGameDialogGeneralConfig();

	/**
	 * Called by @ref KGameDialog to submit all settings to the KGame
	 * Object.
	 * You have to replace this if you add your own widgets!
	 * @param g A pointer to your KGame.
	 * @param p A pointer to the player owning this dialog
	 **/
	virtual void submitToKGame(KGame* g, KPlayer* p);

	/**
	 * Change the owner of the config widget.
	 *
	 * Changes the playername in the line edit
	 * @param p The new owner player
	 **/
	virtual void setOwner(KPlayer* p);

	/**
	 * See @ref KGameDialogConfig::setKGame
	 *
	 * Sets the default values of all KGame related predefined widgets
	 * (currently none)
	 **/
	virtual void setKGame(KGame* g);

	/**
	 * See @ref KGameDialogConfig::setAdmin
	 *
	 * This deactivates the min/max player widgets
	 **/
	virtual void setAdmin(bool admin);

protected slots:
	void slotPropertyChanged(KGamePropertyBase*, KPlayer*);

protected:
	void setPlayerName(const QString& name);

	QString playerName() const;

private:
	KGameDialogGeneralConfigPrivate* d;
};

class KGameDialogNetworkConfigPrivate;
class KGameDialogNetworkConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	KGameDialogNetworkConfig(QWidget* parent = 0);
	virtual ~KGameDialogNetworkConfig();


	void disableInitConnection();

	/**
	 * Called by @ref KGameDialog to submit all settings to the KGame
	 * Object.
	 * You have to replace this if you add your own widgets!
	 * @param g A pointer to your KGame.
	 * @param p A pointer to the player owning this dialog
	 **/
	virtual void submitToKGame(KGame* g, KPlayer* p);

	virtual void setKGame(KGame* g);

	/**
	 * This sets the default port and host used in @ref KGameConnectDialog.
	 * The user will be able to change these defaults!
	 *
	 * If you don't call this then host "localhost" and port "0" is used.
	 * You are strongly encouraged to change at least the port!
	 * @param port The default port to connect to / listen on
	 * @param host The default host to connect to
	 **/
	void setDefaultNetworkInfo(const QString& host, unsigned short int port,bool server=true);

protected:
	void setConnected(bool connected, bool master = false);

protected slots:
	void slotInitConnection();
	void slotExitConnection();
	void slotConnectionBroken();

private:
	KGameDialogNetworkConfigPrivate* d;
};

class KGameDialogMsgServerConfigPrivate;
class KGameDialogMsgServerConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	KGameDialogMsgServerConfig(QWidget* parent = 0);
	virtual ~KGameDialogMsgServerConfig();

	virtual void submitToKGame(KGame*, KPlayer*) {}

	void setHasMsgServer(bool);

	virtual void setKGame(KGame*);
	virtual void setAdmin(bool);

protected slots:
	void slotChangeMaxClients();
	void slotChangeAdmin();
	void slotRemoveClient();

protected:
	void removeClient(Q_UINT32 id);

private:
	KGameDialogMsgServerConfigPrivate* d;
};

class KGameDialogChatConfigPrivate;
/**
 * This is not really a configuration widget but rather a simple chat widget.
 * This widget does nothing but just providing a @ref KGameChat object.
 * @short A chat widget inside a @ref KGameDialog
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameDialogChatConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	KGameDialogChatConfig(int chatMsgId, QWidget* parent = 0);
	virtual ~KGameDialogChatConfig();

	virtual void setKGame(KGame*);
	virtual void setOwner(KPlayer*);

	virtual void submitToKGame(KGame*, KPlayer*) { }

private:
	KGameDialogChatConfigPrivate* d;
};

/**
 * @short Lists all connected players and gives the ability to kick them off the
 * game
 **/
class KGameDialogConnectionConfigPrivate;
class KGameDialogConnectionConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	KGameDialogConnectionConfig(QWidget* parent = 0);
	virtual ~KGameDialogConnectionConfig();

	virtual void setKGame(KGame*);
	virtual void setOwner(KPlayer*);
	virtual void setAdmin(bool admin);

	virtual void submitToKGame(KGame*, KPlayer*) { }

protected:
	/**
	 * @return The @ref QListBoxItem that belongs to the player p
	 **/
	QListBoxItem* item(KPlayer* p) const;

protected slots:
	void slotKickPlayerOut(QListBoxItem* item);
	void slotPropertyChanged(KGamePropertyBase* prop, KPlayer* p);
	void slotPlayerLeftGame(KPlayer* p);
	void slotPlayerJoinedGame(KPlayer* p);
	void slotClearPlayers();

private:
	KGameDialogConnectionConfigPrivate* d;
		
};
#endif
