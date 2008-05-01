/*
    This file is part of the kggzgames library.
    Copyright (c) 2005 - 2007 Josef Spillner <josef@ggzgamingzone.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef LIBKDEGAMES_KGGZSEATSDIALOG_H
#define LIBKDEGAMES_KGGZSEATSDIALOG_H

#include <kggzmod/event.h>

#include <QtGui/QWidget>

#include "kggzgames_export.h"

/**
 * @mainpage kggzgames
 *
 * The \b kggzgames library helps KDE/Qt game developers to integrate
 * their game with the GGZ Gaming Zone. It is the visual counterpart to
 * the rather low-level \b kggzmod library and contains convenience
 * widgets and dialogs.
 *
 * At the moment, the KGGZSeatsDialog class is the only on in this
 * library. It can be used by game clients to display information
 * about the players and spectators and their associated statistics.
 */

namespace KGGZMod
{
	class Module;
}

class KGGZSeatsDialogPrivate;

/**
 * \class KGGZSeatsDialog kggzseatsdialog.h <KGGZSeatsDialog>
 * 
 * @short Dialog to view and modify the seat configuration.
 *
 * This dialog can be used to view and modify the seat configuration of
 * a table of a game running on the GGZ Gaming Zone. Seats might be
 * occupied by players, bots or spectators. They might also be
 * unoccupied, either just empty or reserved for someone.
 *
 * The dialog assumes a running GGZ game and will re-use the game's
 * connection to the GGZ core client to fetch data from the GGZ server
 * about the players, such as photos or avatar images.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class KGGZGAMES_EXPORT KGGZSeatsDialog : public QWidget
{
	Q_OBJECT
	public:
		/**
		 * Constructor.
		 *
		 * Displays the seats dialog and starts the interaction
		 * with the GGZ core client.
		 */
		KGGZSeatsDialog(QWidget *parent = NULL);

		/**
		 * Destructor.
		 */
		~KGGZSeatsDialog();

		/**
		 * Sets the KGGZMod module object to use.
		 *
		 * Usually, calling this method is not necessary, since the
		 * default single module gets set automatically.
		 *
		 * @param mod Module to use for the interaction
		 */
		void setMod(KGGZMod::Module *mod);

	private:
		friend class KGGZSeatsDialogPrivate;
		KGGZSeatsDialogPrivate* const d;
		Q_DISABLE_COPY(KGGZSeatsDialog)

		Q_PRIVATE_SLOT(d, void slotDisplay(int id))
		Q_PRIVATE_SLOT(d, void slotTaskData(KIO::Job *job, const QByteArray&))
		Q_PRIVATE_SLOT(d, void slotTaskResult(KIO::Job *job))
		Q_PRIVATE_SLOT(d, void slotInfo(const KGGZMod::Event& event))
		Q_PRIVATE_SLOT(d, void slotAction())
		Q_PRIVATE_SLOT(d, void slotMenu(QAction *action))
};

#endif // LIBKDEGAMES_KGGZSEATSDIALOG_H
