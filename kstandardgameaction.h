/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann <b_mann@gmx.de>
    Copyright (C) 2007 Simon Hürlimann <simon.huerlimann@huerlisi.ch>

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
// this class was shamelessy stolen from kdelibs/kdeui/kstdction.[cpp|h] and
// after that just edited for our needs
#ifndef KSTANDARDGAMEACTION_H
#define KSTANDARDGAMEACTION_H

#include <libkdegames_export.h>

class KAction;
class KToggleAction;
class QObject;
class KRecentFilesAction;
class KSelectAction;

/**
 * Extension for KStandardAction in KDE Games
 *
 * This class is an extension to the usual KStandardAction class which provides
 * easy access to often used KDE actions.
 *
 * Using these actions helps maintaining consistency among the games.
 *
 * Games often use different menu entries than other programs, e.g. games use
 * the menu "game" instead of "file". This class provides the entries which
 * differ from the usual KStandardAction entries.
 *
 * @see <tt>KStandardAction</tt>
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
namespace KStandardGameAction
{
  /**
   * The standard menubar and toolbar actions.
   **/
    enum StandardGameAction {
        // Game menu
        New=1, Load, LoadRecent, Save, SaveAs, End, Pause, Highscores, Statistics,
        Print, Quit,
        // Move menu
        Repeat, Undo, Redo, Roll, EndTurn,
        // Settings menu
        Carddecks,
        ChooseGameType,
        ConfigureHighscores,
        ClearHighscores,
        ClearStatistics,
        Restart,
        Hint,
        Demo,
        Solve,
        ActionNone
    };

    /**
     * Creates an action corresponding to the
     * KStandardAction::StandardAction enum.
     */
    KDEGAMES_EXPORT KAction* create( StandardGameAction id, const QObject *recvr, const char *slot,
                            QObject* parent );


    /**
     * This will return the internal name of a given standard action.
     */
    KDEGAMES_EXPORT const char* name( StandardGameAction id );

    /**
     * Start a new game.
     **/
    KDEGAMES_EXPORT KAction *gameNew(const QObject *recvr, const char *slot,
                            QObject *parent );

    /**
     * Load a previousely saved game.
     */
    KDEGAMES_EXPORT KAction *load(const QObject *recvr, const char *slot,
                         QObject *parent );

    // FIXME why not to delete this and use just KStandardAction::openRecent???
    // loadRecent seems to mimic its behaviour
    /**
     * Load a recently loaded game.
     * The signature of slot is of the form slotURLSelected(const KUrl&)
     */
    KDEGAMES_EXPORT KRecentFilesAction *loadRecent(const QObject *recvr, const char *slot,
                                          QObject *parent );

    /**
     * Save the current game.
     */
    KDEGAMES_EXPORT KAction *save(const QObject *recvr, const char *slot,
                         QObject *parent);

    /**
     * Save the current game under a different filename.
     */
    KDEGAMES_EXPORT KAction *saveAs(const QObject *recvr, const char *slot,
                           QObject *parent );

    /**
     * Pause the game.
     **/
    KDEGAMES_EXPORT KToggleAction *pause(const QObject *recvr, const char *slot,
                                QObject *parent );

    /**
     * Show the highscores.
     */
    KDEGAMES_EXPORT KAction *highscores(const QObject *recvr, const char *slot,
                               QObject *parent );

    /**
     * Clear highscores.
     */
    KDEGAMES_EXPORT KAction *clearHighscores(const QObject *recvr, const char *slot,
                               QObject *parent );

    /**
     * Show the statistics.
     */
    KDEGAMES_EXPORT KAction *statistics(const QObject *recvr, const char *slot,
                               QObject *parent );

    /**
     * Clear statistics.
     */
    KDEGAMES_EXPORT KAction *clearStatistics(const QObject *recvr, const char *slot,
                               QObject *parent );


    /**
     * End the current game, but do not quit the program.
     * Think of a "close" entry.
     */
    KDEGAMES_EXPORT KAction *end(const QObject *recvr, const char *slot,
                        QObject *parent );

    /**
     * Print current game.
     * Not useful in all games.
     */
    KDEGAMES_EXPORT KAction *print(const QObject *recvr, const char *slot,
                          QObject *parent );

    /**
     * Quit the game.
     */
    KDEGAMES_EXPORT KAction *quit(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * Repeat the last move.
     **/
    KDEGAMES_EXPORT KAction *repeat(const QObject *recvr, const char *slot,
                           QObject *parent );

    /**
     * Undo the last move.
     **/
    KDEGAMES_EXPORT KAction *undo(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * Redo the last move (which has been undone).
     **/
    KDEGAMES_EXPORT KAction *redo(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * Roll die or dice.
     **/
    KDEGAMES_EXPORT KAction *roll(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * End the current turn (not the game).
     * Usually to let the next player start.
     **/
    KDEGAMES_EXPORT KAction *endTurn(const QObject *recvr, const char *slot,
                            QObject *parent );

    /**
     * Display configure carddecks dialog.
     */
    KDEGAMES_EXPORT KAction *carddecks(const QObject *recvr, const char *slot,
                              QObject *parent );

    /**
     * Display configure highscores dialog.
     */
    KDEGAMES_EXPORT KAction *configureHighscores(const QObject *recvr, const char *slot,
                                        QObject *parent );

    /**
     * Give an advice/hint.
     */
    KDEGAMES_EXPORT KAction *hint(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * Show a demo.
     */
    KDEGAMES_EXPORT KToggleAction *demo(const QObject *recvr, const char *slot,
                               QObject *parent );

    /**
     * Solve the game.
     */
    KDEGAMES_EXPORT KAction *solve(const QObject *recvr, const char *slot,
                          QObject *parent );

    /**
     * Choose game type.
     * The signature of slot is of the form slotGameTypeChosen(int)
     */
    KDEGAMES_EXPORT KSelectAction *chooseGameType(const QObject *recvr, const char *slot,
                                         QObject *parent );

    /**
     * Restart the game.
     */
    KDEGAMES_EXPORT KAction *restart(const QObject *recvr, const char *slot,
                            QObject *parent );

}

#endif
