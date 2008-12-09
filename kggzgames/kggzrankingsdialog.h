/*
    This file is part of the kggzgames library.
    Copyright (c) 2007 Josef Spillner <josef@ggzgamingzone.org>

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

#ifndef KGGZRANKINGSDIALOG_H
#define KGGZRANKINGSDIALOG_H

#include <kggzmod/event.h>

#include <QtGui/QWidget>

#include "kggzgames_export.h"

class KGGZRankingsDialogPrivate;

/**
 * \class KGGZRankingsDialog kggzrankingsdialog.h <KGGZRankingsDialog>
 * 
 * @short Dialog which displays GGZ online rankings
 *
 * This class is actually not a dialog by itself.
 * Instead, it applies some magic to use the standard
 * KScoreDialog from libkdegames.
 *
 * The dialog assumes a running GGZ game and will re-use the game's
 * connection to the GGZ core client to fetch data from the GGZ server
 * about the players.
 *
 * @author Josef Spillner (josef@ggzgamingzone.org)
 */
class KGGZGAMES_EXPORT KGGZRankingsDialog : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Constructor.
		 *
		 * Initiates the delayed display of the dialog.
		 * Nothing will be shown if no rankings event occurs.
		 * Therefore, it is essential to have sent a rankings
		 * request to KGGZMod before calling this constructor.
		 */
		KGGZRankingsDialog(QWidget *parent = NULL);

		/**
		 * Destructor.
		 */
		~KGGZRankingsDialog();

	private:
		friend class KGGZRankingsDialogPrivate;
		KGGZRankingsDialogPrivate* const d;
		Q_DISABLE_COPY(KGGZRankingsDialog)

		Q_PRIVATE_SLOT(d, void slotRankings(const KGGZMod::Event& event))
};

#endif

