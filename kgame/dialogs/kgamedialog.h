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

#ifndef __KGAMEDIALOG_H__
#define __KGAMEDIALOG_H__

#include <kdialogbase.h>

class QGridLayout;
class QVBoxLayout;
class QListBoxItem;

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
 * The main configuration dialog for KGame. Here all players meat each other,
 * every player can see how many players connected (and their names) and the
 * ADMIN can even "kick" players out. You can talk to each other (using @ref
 * KGameChat and the ADMIN can define the maxPlayers/minPlayers as well as the
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
	/**
	 * Just the smae as the other constructor below but just creates the
	 * default widgets.
	 **/
	KGameDialog(KGame* g, KPlayer* owner, const QString& title, 
			QWidget* parent, bool modal = false, 
			int chatMsgId = 15432);

	/**
	 * Construct a very configurable KGameDialog. You will probably want to
	 * use this unless you implement your own constructor.
	 * @param config The game configuration widget. Note that it will be
	 * reparented. 0 will not create any game config widget
	 * @param netConfig The network configuration widget. Note that it will
	 * be reparented. 0 will not create any network configuration widget.
	 * Use "new" to create a config widget for this constructor (use 0 as
	 * parent - it will be reparented to KGameDialog).
	 **/
	KGameDialog(KGameDialogGeneralConfig* config, 
			KGameDialogNetworkConfig* netConfig, KGameDialogMsgServerConfig* msgConf, 
			KGame* g, KPlayer* owner, const QString& title,
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


	/**
	 * Adds a @ref KGameChat to the dialog. If no parent is specified the
	 * game page will be used.
	 * @parent msgid The Chat message ID. See @ref KGameChat::KGameChat for
	 * more information
	 * @parent The parent of the chat widget. Note that the game page will
	 * be used if parent is 0.
	 * @param chat If you sub-class @ref KGameDialogChatConfig you can use
	 * your own chat widget. If chat is 0 the default @ref
	 * KGameDialogChatWidget is used.
	 **/
	void addChatWidget(int msgid, KGameDialogChatConfig* chat = 0, QVBox* parent = 0);


	/**
	 * Add a connection list to the dialog. The list consists of a @ref
	 * KLisBox containing all players in the current game (see @ref
	 * KGame::playerList). The admin can "ban" players, ie kick them out of
	 * the game.
	 *
	 * This is another not-really-config-config-widget. It just displays the
	 * connections and lets you ban players.
	 * @param c If 0 a standard @ref KGameDialogConnectionConfig is created,
	 * otherwise you custom widget is used.
	 * @param parent The parent of the widget. If 0 the @ref networkConfig
	 * page is used.
	 **/
	void addConnectionList(KGameDialogConnectionConfig* c = 0, QVBox* parent = 0);

	
	/**
	 * Add a new page to the dialog. The page will contain you new config
	 * widget and will have your provided title.
	 *
	 * The widget will be reparented to this dialog. This also calls @ref
	 * KGameDialogConfig::setKGame and @ref KGameDialogConfig::setOwner.
	 * @param widget The new config widget
	 * @param title The title of the newly added page.
	 * @return The newly added page which contains your config widget.
	 **/
	QVBox* addConfigPage(KGameDialogConfig* widget, const QString& title);

protected:
	void addConfigWidget(KGameDialogConfig* widget, QWidget* parent);
	void addNetworkConfig(KGameDialogNetworkConfig* netConf);
	void addGameConfig(KGameDialogGeneralConfig* conf);
	void addMsgServerConfig(KGameDialogMsgServerConfig* conf);

	KGameDialogNetworkConfig* networkConfig() const;
	KGameDialogGeneralConfig* gameConfig() const;

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
	 * @param admin Whether this client is ADMIN.
	 **/
	virtual void slotInitConnection(bool& connected, bool& admin);

	/**
	 * Called when the @ref KGame object is destroyed. Calls setKGame(0) so
	 * that all widgets can disconnect their slots and so on.
	 **/
	void slotUnsetKGame();

private:
//	void init(KGameDialogGeneralConfig* conf, KGameDialogNetworkConfig* netConf);
	void init(KGameDialogGeneralConfig* conf, 
			KGameDialogNetworkConfig* netConf, 
			KGameDialogMsgServerConfig* msgServerConfig, 
			KGame*, KPlayer*, int chatMsgid);
	KGameDialogPrivate* d;
};

#endif
