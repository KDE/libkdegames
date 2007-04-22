/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (kde at heni-online.de)

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

// NAMING
// please follow these naming rules if you add/change classes:
// the main dialog is named KGameDialog and the base config widget
// KGameDialogConfig. All config widgets are named KGameDialogXYZConfig (where
// XYZ = the name of the config widget, like "general" or "network") and are
// inherited from KGameDialogConfig.

#ifndef __KGAMEDIALOG_H__
#define __KGAMEDIALOG_H__

#include <kpagedialog.h>
#include <libkdegames_export.h>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
class QGridLayout;
class QVBoxLayout;
class KVBox;

class KGame;
class KPlayer;
class KGamePropertyBase;

class KGameDialogConfig;
class KGameDialogGeneralConfig;
class KGameDialogNetworkConfig;
class KGameDialogMsgServerConfig;
class KGameDialogChatConfig;
class KGameDialogConnectionConfig;

class KGameDialogPrivate;
/**
 * TODO: rewrite entire documentation. Nearly nothing is valid anymore.
 * The main configuration dialog for KGame. Here all players meat each other,
 * every player can see how many players connected (and their names) and the
 * ADMIN can even "kick" players out. You can talk to each other (using 
 * KGameChat and the ADMIN can define the maxPlayers/minPlayers as well as the
 * number of computer players.
 *
 *
 * AB: setDefaultXYZ is obsolete!!
 * You will usually create an instance of KGameDialog or any derived class and
 * call setDefaultXYZ methods. Example (maybe
 * obsoleted parameters - docu is currently changing very fast):
 * \code
 * 	KGameDialog dlg(kgame, i18n("New Game"), localPlayer, this, true,
 * 	ID_CHAT);
 * 	dlg.setDefaultNetworkInfo(port, host); // AB: obsolete!
 * 	dlg.exec();
 * \endcode
 * This will create a default modal dialog with the title "New Game". You don't
 * have to do more than this. 
 *
 * @short Main configuration dialog for KGame
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KDEGAMES_EXPORT KGameDialog : public KPageDialog
{
	Q_OBJECT
public:

	enum ConfigOptions
	{
		NoConfig = 0,
		ChatConfig = 1,
		GameConfig = 2,
		NetworkConfig = 4,
		MsgServerConfig = 8,
		BanPlayerConfig = 16,
		AllConfig = 0xffff
	};

	/**
	 * Create an empty KGameDialog. You can add widgets using
	 * addConfigPage.
	 * @param g The KGame object of this game
	 * @param owner The KPlayer object who is responsible for this
	 * dialog, aka "the local player"
	 * @param title The title of the dialog - see KDialog::setCaption
	 * @param parent The parent of the dialog
	 * @param modal Whether the dialog is modal or not
	 **/
	KGameDialog(KGame* g, KPlayer* owner, const QString& title, 
			QWidget* parent, bool modal = false);
	
	/**
	 * Create a KGameDialog with the standard configuration widgets. This
	 * creates the following widgets:
	 * <ul>
	 * <li> KGameDialogGeneralConfig
	 * <li> KGameDialogNetworkConfig
	 * <li> KGameDialogMsgServerConfig
	 * <li> KGameDialogChatConfig
	 * <li> KGameDialogConnectionConfig
	 * </ul>
	 * If you want to use your own implementations (or none) of the widgets
	 * above you should subclass KGameDialog. Use addGameConfig, 
	 * addNetworkConfig, addMsgConfig, addChatWidget and 
	 * addConnectionList in this case.
	 *
	 * If you want to add further configuration widget you can simply use
	 * addConfigPage
	 * @param g The KGame object of this game
	 * @param owner The KPlayer object who is responsible for this
	 * dialog, aka "the local player"
	 * @param title The title of the dialog - see KDialog::setCaption
	 * @param parent The parent of the dialog
	 * @param modal Whether the dialog is modal or not
	 * @param initConfigs whether the default KGameDialogConfig widgets
	 * shall be created using initDefaultDialog. Use false if you want
	 * to use custom widgets.
	 * @param chatMsgId The ID of Chat messages. See KGameChat. Unused
	 * if initConfigs = false
	 **/
	KGameDialog(KGame* g, KPlayer* owner, const QString& title, 
			QWidget* parent, long initConfigs = AllConfig, 
			int chatMsgId = 15432, bool modal = false);

	virtual ~KGameDialog();


	/**
	 * Change the owner of the dialog. This will be used as the fromPlayer in
	 * KGameChat and will receive the entered player name.
	 * @param owner The owner of the dialog. It must already be added to the
	 * KGame object!
	 *
	 * Calls the KGameDialogConfig::setOwner implementation of all
	 * widgets that have been added by addConfigWidget
	 * @param owner The new owner player of this dialog must already be
	 * added to the KGame object. Can even be NULL (then no player
	 * configuration is made)
	 **/
	void setOwner(KPlayer* owner);

	/**
	 * Change the KGame object this dialog is used for.
	 *
	 * Calls the KGameDialogConfig::setKGame implementation of all
	 * widgets that have been added by addConfigWidget
	 * @param g The new KGame object
	 **/
	void setKGame(KGame* g);

	/**
	 * This will submit all configuration data to the KGame object.
	 * Automatically called by slotApply and slotOk
	 * There is no need to replace this unless you
	 * want to add widgets which are not derived from those classes
	 **/
	virtual void submitToKGame();

	/**
	 * Adds a KGameChat to the dialog. If no parent is specified the
	 * game page will be used.
	 * @param chat The chat widget
	 * @param parent The parent of the chat widget. This MUST be an
	 * already added config widget. Note that the game page will be used
	 * if parent is 0.
	 **/
	void addChatWidget(KGameDialogChatConfig* chat, KVBox* parent = 0);

	/**
	 * Add a connection list to the dialog. The list consists of a
	 * KLisBox containing all players in the current game (see
	 * KGame::playerList). The admin can "ban" players, ie kick them out of
	 * the game.
	 *
	 * This is another not-really-config-config-widget. It just displays the
	 * connections and lets you ban players.
	 * @param c The KGameDialogConnectionConfig object
	 * @param parent The parent of the widget. If 0 the networkConfig
	 * page is used.
	 **/
	void addConnectionList(KGameDialogConnectionConfig* c, KVBox* parent = 0);

	/**
	 * Add a new page to the dialog. The page will contain you new config
	 * widget and will have your provided title.
	 *
	 * The widget will be reparented to this dialog. This also calls
	 * KGameDialogConfig::setKGame and KGameDialogConfig::setOwner.
	 * @param widget The new config widget
	 * @param title The title of the newly added page.
	 * @return The newly added page which contains your config widget.
	 **/
	KVBox* addConfigPage(KGameDialogConfig* widget, const QString& title);

	/**
	 * @return The QVBox of the given key, The key is from ConfigOptions
	 * Note that not all are supported yet
	 **/
	KVBox *configPage(ConfigOptions which);

	/**
	 * @return The default netowrk config. Note that this always returns 0 if
	 * you did not specify NetworkConfig in the constructor!
	 **/
	KGameDialogNetworkConfig* networkConfig() const;

	/**
	 * @return The default game config. Note that this always returns 0 if
	 * you did not specify GameConfig in the constructor!
	 **/
	KGameDialogGeneralConfig* gameConfig() const;

	/**
	 * Add a config widget to the specified parent. Usually you call
	 * addConfigPage for one widget and addConfigWidget for another to add
	 * it to the same page. Just use the returned page of
	 * addConfigPage.
	 **/
	void addConfigWidget(KGameDialogConfig* widget, QWidget* parent);

	/**
	 * Used to add the main network config widget in a new page. Use this to
	 * make networkConfig return something useful.
	 **/
	void addNetworkConfig(KGameDialogNetworkConfig* netConf);

	/**
	 * Add the main game config widget in a new page. Use this to make 
	 * gameConfig return something useful.
	 **/
	void addGameConfig(KGameDialogGeneralConfig* conf);

	/**
	 * Used to add the message server config widget in a new page.
	 **/
	void addMsgServerConfig(KGameDialogMsgServerConfig* conf);

protected:

	/**
	 * This is used to create a dialog containing all the default widgets. 
	 *
	 * You may want to use this if you just want to use your own
	 * configuration widgets which inherit the standard ones.
	 *
	 * Note that if one of the widgets is NULL the default implementation
	 * will be used! (except the chat widget - you need to create it
	 * yourself as you have to provide a message id)
	 * @param initConfigs The widgets to be created
	 * @param chatMsgId The msgid for the chat config (only if specified in
	 * initConfigs) - see KGameDialogChatConfig
	 **/
	void initDefaultDialog(ConfigOptions initConfigs, int chatMsgId = 15432);

	/**
	 * Go through all config widgets and call their 
	 * KGameDialogConfig::setKGame and KGameDialogConfig::setOwner implementation
	 * 
	 * This function could be private and probably will be very soon.
	 * Don't use it yourself
	 **/
	void configureConfigWidgets();

protected Q_SLOTS:
	/**
	 * Called when the user clicks on Ok. Calls slotApply and
	 * QDialog::accept()
	 **/
	virtual void slotOk();

	/**
	 * Just calls submitToKGame()
	 **/
	virtual void slotApply();

	/**
	 * Sets the default values for the configuration widgets. Set these
	 * values by (e.g.) setDefaultMaxPlayers()
	 * @deprecated
	 **/
	virtual void slotDefault();

	/**
	 * Called when the KGame object is destroyed. Calls setKGame(0) so
	 * that all widgets can disconnect their slots and so on.
	 **/
	void slotUnsetKGame();

	/**
	 * Called when the ADMIN status of this KGame client changes. See 
	 * KGameNetwork::signalAdminStatusChanged
	 * @param isAdmin TRUE if this client is now the ADMIN otherwise FALSE
	 **/
	void setAdmin(bool isAdmin);

	/**
	 * Remove a config widget from the widget list. 
	 * @see QObject::destroyed
	 **/
	void slotRemoveConfigWidget(QObject* configWidget);

private:
	void init(KGame*, KPlayer*);

private:
	KGameDialogPrivate* d;
};

#endif
