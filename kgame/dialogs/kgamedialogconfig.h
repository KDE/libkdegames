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
	 * The admin status has been changed.
	 * If the KGame object of this config widget is the
	 * admin the user is allowed to configure it. Otherwise most
	 * widgets will have to be disabled. Note that you don't necessarily
	 * need to deactivate all widget - e.g. the player name must be
	 * configured by the player. Mainly the KGame configuration can be done
	 * by the admin only.
	 *
	 * By default this does nothing. You have to overwrite it in derived
	 * classes.
	 * @param admin Whether the KGame object of this dialog can be
	 * configured
	 **/
	virtual void setAdmin(bool admin);

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
	 * See @ref KGameDialogConfig::setAdmin
	 *
	 * This deactivates the min/max player widgets
	 **/
	virtual void setAdmin(bool admin);

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

class KGameDialogMsgServerConfigPrivate;
class KGameDialogMsgServerConfig : public KGameDialogConfig
{
	Q_OBJECT
public:
	KGameDialogMsgServerConfig(QWidget* parent);
	~KGameDialogMsgServerConfig();

	virtual void submitToKGame(KGame*, KPlayer*);

	void changeMaxClients();
	void changeAdmin();
	void removeClient();
	void setHasMsgServer(bool);

	virtual void setKGame(KGame*);
	virtual void setAdmin(bool);

protected:
	void removeClient(Q_UINT32 id);


private:
	KGameDialogMsgServerConfigPrivate* d;
};
#endif
