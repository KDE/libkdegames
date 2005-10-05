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
#ifndef KSTDGAMEACTION_H
#define KSTDGAMEACTION_H

class KAction;
class KToggleAction;
class QObject;
class KActionCollection;
class KRecentFilesAction;
class KSelectAction;
#include <kdemacros.h>

//-----------------------------------------------------------------------------
/**
 * Replacement for KStdAction for KDE Games
 *
 * This class is an extension to the usual KStdAction class which provides
 * easy access to often used KDE actions
 *
 * Games often use different menu entries than other programs, e.g. games use
 * the menu "game" instead of "file". This class provides the entries which
 * differ from the usual KStdAction entries.
 *
 * @see KStdAction
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
// #### KDE4: transform in namespace
class KDE_EXPORT KStdGameAction
{
public:
  /**
   * The standard menubar and toolbar actions.
   **/
    enum StdGameAction {
        // Game menu
        New=1, Load, LoadRecent, Save, SaveAs, End, Pause, Highscores,
        Print, Quit,
        // Move menu
        Repeat, Undo, Redo, Roll, EndTurn,
        // Settings menu
        Carddecks,
        ChooseGameType, // @since 3.2
        ConfigureHighscores, // @since 3.2

        Restart, // @since 3.2
        Hint, // @since 3.2
        Demo, // @since 3.2
        Solve, // @since 3.2
        ActionNone // @since 3.2
    };

    KStdGameAction();
    ~KStdGameAction();

    /**
     * Creates an action corresponding to the
     * KStdAction::StdAction enum.
     * @since 3.2
     */
    static KAction* create( StdGameAction id, const char *name,
			    const QObject *recvr, const char *slot,
			    KActionCollection* parent );

    /**
     * @since 3.2
     */
	static KAction* create( StdGameAction id,
		const QObject *recvr, const char *slot,
		KActionCollection* parent )
		{ return create( id, 0, recvr, slot, parent ); }


    /**
     * Retrieve the action corresponding to the
     * KStdGameAction::StdGameAction enum.
     * @deprecated
     */
    static KAction *action(StdGameAction act_enum, const QObject *recvr = 0,
                           const char *slot = 0, KActionCollection *parent = 0,
                           const char *name = 0L );

    /**
     * This will return the internal name of a given standard action.
     * @since 3.2
     */
    static const char* name( StdGameAction id );

    /**
     * This will return the internal name of a given standard action.
     * @deprecated
     */
    static const char* stdName(StdGameAction act_enum);

    /**
     * Start a new game
     **/
    static KAction *gameNew(const QObject *recvr = 0, const char *slot = 0,
                            KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Load a previousely saved game
     */
    static KAction *load(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Load a recently loaded game.
     */
    static KRecentFilesAction *loadRecent(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Save the current game.
     */
    static KAction *save(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Save the current game under a different filename.
     */
    static KAction *saveAs(const QObject *recvr = 0, const char *slot = 0,
                           KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Pause the game
     **/
    static KToggleAction *pause(const QObject *recvr = 0, const char *slot = 0,
                           KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Show the highscores.
     */
    static KAction *highscores(const QObject *recvr = 0, const char *slot = 0,
                           KActionCollection *parent = 0, const char *name = 0L );


    /**
     * End the current game, but do not quit the program. Think of a "close"
     * entry.
     */
    static KAction *end(const QObject *recvr = 0, const char *slot = 0,
                          KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Print the current screen? Game? Whatever - hardly used in games but there
     * is at least one example (ktuberling)
     */
    static KAction *print(const QObject *recvr = 0, const char *slot = 0,
                          KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Quit the game.
     */
    static KAction *quit(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );



    /**
     * Repeat the last move.
     **/
    static KAction *repeat(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Undo the last move
     **/
    static KAction *undo(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Redo the last move (which has been undone)
     **/
    static KAction *redo(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Roll die or dice
     **/
    static KAction *roll(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * End the current turn (not the game). Usually to let the next player
     * start
     **/
    static KAction *endTurn(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );


    /**
     * Display configure carddecks dialog.
     */
    static KAction *carddecks(const QObject *recvr = 0, const char *slot = 0,
                                KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Display configure highscores dialog.
     * @since 3.2
     */
    static KAction *configureHighscores(const QObject *recvr = 0, const char *slot = 0,
                                KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Give an advice/hint.
     * @since 3.2
     */
    static KAction *hint(const QObject *recvr = 0, const char *slot = 0,
                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Show a demo.
     * @since 3.2
     */
    static KToggleAction *demo(const QObject *recvr = 0, const char *slot = 0,
                               KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Solve the game.
     * @since 3.2
     */
    static KAction *solve(const QObject *recvr = 0, const char *slot = 0,
                          KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Choose game type.
     * @since 3.2
     */
    static KSelectAction *chooseGameType(const QObject *recvr = 0, const char *slot = 0,
                                         KActionCollection *parent = 0, const char *name = 0L );

    /**
     * Restart game.
     * @since 3.2
     */
    static KAction *restart(const QObject *recvr = 0, const char *slot = 0,
                            KActionCollection *parent = 0, const char *name = 0L );

};

#endif
