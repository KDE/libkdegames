/* **************************************************************************
                           KGameDialog Class
                           -------------------
    begin                : 1 January 2001
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

// NAMING
// please follow these naming rules if you add/change classes:
// the main dialog is named KGameDialog and the base config widget
// KGameDialogConfig. All config widgets are named KGameDialogXYZConfig (where
// XYZ = the name of the config widget, like "general" or "network") and are
// inherited from KGameDialogConfig.

#ifndef KGAMEDIALOG_H
#define KGAMEDIALOG_H

#include <kdialogbase.h>

class QGridLayout;
class QVBoxLayout;
class QListBoxItem;

class KGame;
class KPlayer;
class KPlayerDataBase;

class KGameDialogConfigPrivate;
/**
 * Base class for configuration widgets.
 *
 * You can inherit form this and implement @ref submitToKGame, @ref
 * setOwner and @ref setKGame to create your personal @reg KGame configuration widget :-)
 * @short Base class for configuration widgets
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameDialogConfig : public QWidget
{
	Q_OBJECT
public:
	KGameDialogConfig(QWidget* parent);
	~KGameDialogConfig();

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
	 * implementation does nothing. You can use this e.g. to change a
	 * line edit widget containing the player name.
	 *
	 * Note: even NULL players are allowed!
	 * @param p The new owner player of the dialog
	 **/
	virtual void setOwner(KPlayer* p);

	/**
	 * The KGame object of the dialog has been changed. The default
	 * implementation does nothing. You can use this e.g. to re-read the
	 * min/max player settings.
	 * @param p The new owner player of the dialog
	 **/
	virtual void setKGame(KGame* g);

	/**
	 * Changes the MASTER status (AB: the word "master" might change to
	 * "admin" very soon!). If the KGame object of this config widget is the
	 * master/admin the user is allowed to configure it. Otherwise most
	 * widgets will have to be disabled. Note that you don't necessarily
	 * need to deactivate all widget - e.g. the player name must be
	 * configured by the player. Mainly the KGame configuration can be done
	 * by the master/admin only.
	 *
	 * By default this does nothing. You have to overwrite it in derived
	 * classes.
	 * @param master Whether the KGame object of this dialog can be
	 * configured
	 **/
	virtual void setMaster(bool master);

protected:

private:
	KGameDialogConfigPrivate* d;
};

class KGameDialogNetworkConfigPrivate;
class KGameDialogNetworkConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	KGameDialogNetworkConfig(QWidget* parent);
	~KGameDialogNetworkConfig();


	void disableInitConnection();
	void setNetworkText(const QString& text);

	/**
	 * Called by @ref KGameDialog to submit all settings to the KGame
	 * Object.
	 * You have to replace this if you add your own widgets!
	 * @param g A pointer to your KGame.
	 * @param p A pointer to the player owning this dialog
	 **/
	virtual void submitToKGame(KGame* g, KPlayer* p);

	virtual void setKGame(KGame* g);

signals:
	void signalInitConnection(bool&, bool&);

protected:
//	void setMaxClients(int m);//obsoleted by KMessageServer
//	int maxClients() const;

protected slots:
	void slotInitConnection();


private:
	KGameDialogNetworkConfigPrivate* d;
};

class KGameDialogGeneralConfigPrivate;
class KGameDialogGeneralConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	KGameDialogGeneralConfig(QWidget* parent);
	~KGameDialogGeneralConfig();

	/**
	 * Called by @ref KGameDialog to submit all settings to the KGame
	 * Object.
	 * You have to replace this if you add your own widgets!
	 * @param g A pointer to your KGame.
	 * @param p A pointer to the player owning this dialog
	 **/
	virtual void submitToKGame(KGame* g, KPlayer* p);

	QGridLayout* layout() const;//FIXME: layout for derived classes - this is a workaround

	/**
	 * Change the owner of the config widget.
	 *
	 * Changes the playername in the line edit
	 * @param p The new owner player
	 **/
	virtual void setOwner(KPlayer* p);

	virtual void setKGame(KGame* g);

	/**
	 * See @ref KGameDialogConfig::setMaster
	 *
	 * This deactivates the min/max player widgets
	 **/
	virtual void setMaster(bool master);

protected:
	void setMaxPlayers(int m);
	void setMinPlayers(int m);
	void setPlayerName(const QString& name);

	int maxPlayers() const;
	int minPlayers() const;
	QString playerName() const;

private:
	KGameDialogGeneralConfigPrivate* d;
};

class KGameDialogPrivate;
/**
 * The main configuration dialog for KGame. Here all players meat each other,
 * every player can see how many players connected (and their names) and the
 * MASTER can even "kick" players out. You can talk to each other (using @ref
 * KGameChat and the MASTER can define the maxPlayers/minPlayers as well as the
 * number of computer players.
 *
 *
 * AB: setDefaultXYZ is obsolete!!
 * You will usually create an instance of KGameDialog or any derived class and
 * call setDefaultXYZ methods. Example (maybe
 * obsoleted parameters - docu is currently changing very fast):
 * <pre>
 * 	KGameDialog dlg(kgame, i18n("New Game"), localPlayer, this, true,
 * 	ID_CHAT);
 * 	dlg.setDefaultNetworkInfo(port, host);
 * 	dlg.exec();
 * </pre>
 * This will create a default modal dialog with the title "New Game". You don't
 * have to do more than this. 
 *
 * @short Main configuration dialog for KGame
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameDialog : public KDialogBase
{
	Q_OBJECT
public:
	KGameDialog(KGame* g, KPlayer* owner, const QString& title, 
			QWidget* parent, bool modal = false, 
			int chatMsgId = 15432);

	/**
	 * @param config The game configuration widget. Note that it will be
	 * reparented. 0 will not create any game config widget
	 * @param netConfig The network configuration widget. Note that it will
	 * be reparented. 0 will not create any network configuration widget
	 **/
	KGameDialog(KGameDialogGeneralConfig* config, 
			KGameDialogNetworkConfig* netConfig, KGame* g,
			KPlayer* owner, const QString& title,
			QWidget* parent, bool modal = false, 
			int chatMsgId = 15432);

	~KGameDialog();


	/**
	 * Change the owner of the dialog. This will be used as the fromPlayer in
	 * @ref KGameChat and will receive the entered player name.
	 * @param owner The owner of the dialog. It must already be added to the
	 * @ref KGame object!
	 *
	 * Calls the @ref KGameDialogConfig::setOwner implementation of all
	 * widgets that have been added by @ref addConfigWidget
	 * @param owner The new owner player of this dialog must already be
	 * added to the @ref KGame object. Can even be NULL (then no player
	 * configuration is made)
	 **/
	void setOwner(KPlayer* owner);

	/**
	 * Change the @ref KGame object this dialog is used for.
	 *
	 * Calls the @ref KGameDialogConfig::setKGame implementation of all
	 * widgets that have been added by @ref addConfigWidget
	 * @param g The new @ref KGame object
	 **/
	void setKGame(KGame* g);

	/**
	 * This will set the default port and host used in @ref
	 * KGameConnectDialog. The user will be able to change these defaults!
	 * @param port The default port
	 * @param host The default host to connect to
	 **/
	void setDefaultNetworkInfo(unsigned short int port, const QString& host);

	/**
	 * This will submit all configuration data to the @ref KGame object.
	 * Automatically called by @ref slotApply and @ref slotOk
	 * Tere is no need to replace this unless you
	 * want to add widgets which are not derived from those classes
	 **/
	virtual void submitToKGame();


	void addChatWidget(int msgid);
	void addConnectionList();

	/**
	 * Add a new wiget to the dialog. The widget will be reparented to this
	 * dialog. This also calls @ref KGameDialogConfig::setKGame and @ref
	 * KGameDialogConfig::setOwner.
	 * @param widget The new config widget
	 **/
	void addConfigWidget(KGameDialogConfig* widget) { addConfigWidget(widget, 0); }

protected:
	void addConfigWidget(KGameDialogConfig* widget, QWidget* parent);
	void addNetworkConfig(KGameDialogNetworkConfig* netConf);
	void addGameConfig(KGameDialogGeneralConfig* conf);

	KGameDialogNetworkConfig* networkConfig() const;
	KGameDialogGeneralConfig* gameConfig() const;

	QVBoxLayout* layout() const;//AB: really ugly!

	/**
	 * Go through all config widgets and call their @ref
	 * KGameDialogConfig::setKGame and @ref KGameDialogConfig::setOwner implementation
	 * 
	 * This function could be private and probably will be very soon.
	 * Don't use it yourself
	 **/
	void configureConfigWidgets();


protected slots:
	/**
	 * This slots reacts to all changes in KGame::playerList(), like new
	 * player joined or player left the game.
	 *
	 * Not very nice though, as this just rebuilds the local player list.
	 **/
	void slotPlayerChanged(KPlayer*);

	/**
	 * Ban a player from the game. The player still can connect again!
	 * @param item The entry of the player
	 **/
	void slotKickPlayerOut(QListBoxItem* item);

	/*
	 * @ref KGameChat has its own slotPropertyChanged but we need another
	 * one for the list of all connected players, here
	 * @param prop The property that has changed
	 * @param player The owner of the property
	 **/
	void slotPropertyChanged(KPlayerDataBase* prop, KPlayer* player);

	/**
	 * Called when the user clicks on Ok. Calles @ref slotApply and @ref
	 * QDialog::accept()
	 **/
	virtual void slotOk();

	/**
	 * Just calles @ref submitToKGame()
	 **/
	virtual void slotApply();

	/**
	 * Sets the default values for the configuration widgets. Set these
	 * values by (e.g.) @ref setDefaultMaxPlayers()
	 * OBSOLTETE!!!
	 **/
	virtual void slotDefault();


	/**
	 * Starts KGameConnectDialog by default. Overwrite for a custimzed
	 * behaviour!
	 * @param connected Whether the init connection button should be
	 * disabled.
	 * Set to true after a connection has been initialized to disable the
	 * button!
	 * @master Whether this client is MASTER.
	 **/
	virtual void slotInitConnection(bool& connected, bool& master);

private:
//	void init(KGameDialogGeneralConfig* conf, KGameDialogNetworkConfig* netConf);
	void init(KGameDialogGeneralConfig* conf, KGameDialogNetworkConfig* netConf, KGame*, const QString& title, KPlayer*, int chatMsgid);
	KGameDialogPrivate* d;
};

#endif
