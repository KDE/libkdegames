/***************************************************************************
                           KStdGameAction
                           -------------------
    begin                : 13 April 2001
    copyright            : (C) 2001 by Andreas Beckermann
    email                : b_mann@gmx.de
 ***************************************************************************/

/***************************************************************************
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
// this class was shamelessy stolen from kdelibs/kdeui/kstdction.[cpp|h] and
// after that just edited for our needs
#ifndef KSTDGAMEACTION_H
#define KSTDGAMEACTION_H

class KAction;
class QObject;

/**
 * @see KStdAction
 *
 * This class is an extension to the usual @ref KStdAction class which provides
 * easy access to often used KDE actions
 *
 * Games often use different menu entries than other programs, e.g. games use
 * the menu "game" instead of "file". This class provides the entries which
 * differ from the usual @ref KSdtAction entries.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class KStdGameAction
{
public:
  /**
   * The standard menubar and toolbar actions.
   **/
    enum StdGameAction {
        // Game menu
        New=1, Load, Save, SaveAs, End, Pause, Highscores,
        Print, Quit,
	// Move menu
	Repeat, Undo, Redo, EndTurn,
	// Settings menu
	Carddecks
    };

    /**
     * Constructor.
     **/
    KStdGameAction();
    /**
     * Destructor.
     **/
    ~KStdGameAction();

    /**
     * Retrieve the action corresponding to the
     * @ref KStdGameAction::StdGameAction enum.
     */
    static KAction *action(StdGameAction act_enum, const QObject *recvr = 0,
                           const char *slot = 0, QObject *parent = 0,
                           const char *name = 0L );

    /**
     * This will return the internal name of a given standard action.
     */
    static const char* stdName(StdGameAction act_enum);

    /** 
     * Start a new game
     **/
    static KAction *gameNew(const QObject *recvr = 0, const char *slot = 0,
                            QObject *parent = 0, const char *name = 0L );

    /**
     * Load a previousely saved game
     */
    static KAction *load(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Save the current game.
     */
    static KAction *save(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Save the current game under a different filename.
     */
    static KAction *saveAs(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0, const char *name = 0L );

    /**
     * Pause the game
     **/
    static KAction *pause(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0, const char *name = 0L );
    
    /**
     * Show the highscores.
     */
    static KAction *highscores(const QObject *recvr = 0, const char *slot = 0,
                           QObject *parent = 0, const char *name = 0L );


    /**
     * End the current game, but do not quit the program. Think of a "close"
     * entry.
     */
    static KAction *end(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0, const char *name = 0L );

    /**
     * Print the current screen? Game? Whatever - hardly used in games but there
     * is at least one example (ktuberling)
     */
    static KAction *print(const QObject *recvr = 0, const char *slot = 0,
                          QObject *parent = 0, const char *name = 0L );

    /**
     * Quit the game.
     */
    static KAction *quit(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );


   
    /**
     * Repeat the last move.
     **/
    static KAction *repeat(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Undo the last move
     **/
    static KAction *undo(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * Redo the last move (which has been undone)
     **/
    static KAction *redo(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );

    /**
     * End the current turn (not the game). Usually to let the next player
     * start
     **/
    static KAction *endTurn(const QObject *recvr = 0, const char *slot = 0,
                         QObject *parent = 0, const char *name = 0L );


    /**
     * Display configure carddecks dialog.
     */
    static KAction *carddecks(const QObject *recvr = 0, const char *slot = 0,
                                QObject *parent = 0, const char *name = 0L );

};

#endif
