/*
    This file is part of the KDE games library
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2003 Martin Heni (kde at heni-online.de)

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

#ifndef __KGAMESEQUENCE_H_
#define __KGAMESEQUENCE_H_

#include <QObject>

#include "libkdegamesprivate_export.h"

class KPlayer;
class KGame;

/**
 * \class KGameSequence kgamesequence.h <KGame/KGameSequence>
 *
 * This class takes care of round or move management as well of the gameover
 * condition. It is especially used for round based games. For these games @ref
 * nextPlayer and @ref checkGameOver are the most important methods.
 *
 * You can subclass KGameSequence and use @ref KGame::setGameSequence to use
 * your own rules. Note that @ref KGame will take ownership and therefore will
 * delete the object on destruction.
 * @short Round/move management class
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KDEGAMESPRIVATE_EXPORT KGameSequence : public QObject
{
	Q_OBJECT
public:
	KGameSequence();
	virtual ~KGameSequence();

	/**
	 * Select the next player in a turn based game. In an asynchronous game this
	 * function has no meaning. Overwrite this function for your own game sequence.
	 * Per default it selects the next player in the playerList
	 */
	virtual KPlayer* nextPlayer(KPlayer *last, bool exclusive = true);

	virtual void setCurrentPlayer(KPlayer* p);

	/**
	 * @return The @ref KGame object this sequence is for, or NULL if none.
	 **/
	KGame* game() const;

	KPlayer* currentPlayer() const;

	/**
	 * Set the @ref KGame object for this sequence. This is called
	 * automatically by @ref KGame::setGameSequence and you should not call
	 * it.
	 **/
	void setGame(KGame* game);

	/**
	 * Check whether the game is over. The default implementation always
	 * returns 0.
	 *
	 * @param player the player who made the last move
	 * @return anything else but 0 is considered as game over
	**/
	virtual int checkGameOver(KPlayer *player);

private:
	class KGameSequencePrivate;
	KGameSequencePrivate *const d;

	Q_DISABLE_COPY(KGameSequence)
};

#endif

