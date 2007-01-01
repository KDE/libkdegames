/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef KSTANDADDGAMEACTION_H
#define KSTANDARDGAMEACTION_H

class KAction;
class KToggleAction;
class QObject;
class KRecentFilesAction;
class KSelectAction;
#include <libkdegames_export.h>
#include <krecentfilesaction.h>
//-----------------------------------------------------------------------------
/**
 * Replacement for KStandardAction for KDE Games
 *
 * This class is an extension to the usual KStandardAction class which provides
 * easy access to often used KDE actions
 *
 * Games often use different menu entries than other programs, e.g. games use
 * the menu "game" instead of "file". This class provides the entries which
 * differ from the usual KStandardAction entries.
 *
 * @see KStandardAction
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
// #### KDE4: transform in namespace
class KDEGAMES_EXPORT KStandardGameAction
{
public:
  /**
   * The standard menubar and toolbar actions.
   **/
    enum StandardGameAction {
        // Game menu
        New=1, Load, LoadRecent, Save, SaveAs, End, Pause, Highscores,
        Print, Quit,
        // Move menu
        Repeat, Undo, Redo, Roll, EndTurn,
        // Settings menu
        Carddecks,
        ChooseGameType,
        ConfigureHighscores,
        Restart,
        Hint,
        Demo,
        Solve,
        ActionNone
    };

    KStandardGameAction();
    ~KStandardGameAction();

    /**
     * Creates an action corresponding to the
     * KStandardAction::StandardAction enum.
     */
    static KAction* create( StandardGameAction id, const QObject *recvr, const char *slot,
			    QObject* parent );


    /**
     * This will return the internal name of a given standard action.
     */
    static const char* name( StandardGameAction id );

    /**
     * Start a new game
     **/
    static KAction *gameNew(const QObject *recvr, const char *slot,
                            QObject *parent );

    /**
     * Load a previousely saved game
     */
    static KAction *load(const QObject *recvr, const char *slot,
                         QObject *parent );

    // FIXME why not to delete this and use just KStandardGameAction::openRecent???
    // loadRecent seems to mimic its behaviour
    /**
     * Load a recently loaded game.
     * The signature of slot is of the form slotURLSelected(const KUrl&)
     */
    static KRecentFilesAction *loadRecent(const QObject *recvr, const char *slot,
                                          QObject *parent );

    /**
     * Save the current game.
     */
    static KAction *save(const QObject *recvr, const char *slot,
                         QObject *parent);

    /**
     * Save the current game under a different filename.
     */
    static KAction *saveAs(const QObject *recvr, const char *slot,
                           QObject *parent );

    /**
     * Pause the game
     **/
    static KToggleAction *pause(const QObject *recvr, const char *slot,
                                QObject *parent );

    /**
     * Show the highscores.
     */
    static KAction *highscores(const QObject *recvr, const char *slot,
                               QObject *parent );


    /**
     * End the current game, but do not quit the program. Think of a "close"
     * entry.
     */
    static KAction *end(const QObject *recvr, const char *slot,
                        QObject *parent );

    /**
     * Print the current screen? Game? Whatever - hardly used in games but there
     * is at least one example (ktuberling)
     */
    static KAction *print(const QObject *recvr, const char *slot,
                          QObject *parent );

    /**
     * Quit the game.
     */
    static KAction *quit(const QObject *recvr, const char *slot,
                         QObject *parent );



    /**
     * Repeat the last move.
     **/
    static KAction *repeat(const QObject *recvr, const char *slot,
                           QObject *parent );

    /**
     * Undo the last move
     **/
    static KAction *undo(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * Redo the last move (which has been undone)
     **/
    static KAction *redo(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * Roll die or dice
     **/
    static KAction *roll(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * End the current turn (not the game). Usually to let the next player
     * start
     **/
    static KAction *endTurn(const QObject *recvr, const char *slot,
                            QObject *parent );


    /**
     * Display configure carddecks dialog.
     */
    static KAction *carddecks(const QObject *recvr, const char *slot,
                              QObject *parent );

    /**
     * Display configure highscores dialog.
     */
    static KAction *configureHighscores(const QObject *recvr, const char *slot,
                                        QObject *parent );

    /**
     * Give an advice/hint.
     */
    static KAction *hint(const QObject *recvr, const char *slot,
                         QObject *parent );

    /**
     * Show a demo.
     */
    static KToggleAction *demo(const QObject *recvr, const char *slot,
                               QObject *parent );

    /**
     * Solve the game.
     */
    static KAction *solve(const QObject *recvr, const char *slot,
                          QObject *parent );

    /**
     * Choose game type. The signature of slot is of the form slotGameTypeChosen(int)
     */
    static KSelectAction *chooseGameType(const QObject *recvr, const char *slot,
                                         QObject *parent );

    /**
     * Restart game.
     */
    static KAction *restart(const QObject *recvr, const char *slot,
                            QObject *parent );

};

#endif
