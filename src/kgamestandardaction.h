/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2007 Simon Hürlimann <simon.huerlimann@huerlisi.ch>

    SPDX-License-Identifier: LGPL-2.0-only

    This class was shamelessly stolen from kdelibs/kdeui/kstdction.[cpp|h] and
    after that just edited for our needs
*/

#ifndef KGAMESTANDARDACTION_H
#define KGAMESTANDARDACTION_H

// own
#include "kdegames_export.h"
// KF
#include <KRecentFilesAction>
#include <KToggleAction>
// Qt
#include <QAction>
// std
#include <type_traits>

/*!
 * \namespace KGameStandardAction
 * \inmodule LibKDEGames
 * \brief Extension for KStandardAction in KDE Games.
 *
 * This class is an extension to the usual KStandardAction class which provides
 * easy access to often used KDE actions.
 *
 * Using these actions helps in maintaining consistency among the games.
 *
 * Games often use different menu entries than other programs, for example games use
 * the menu "game" instead of "file". This class provides the entries which
 * differ from the usual KStandardAction entries.
 *
 * \sa KStandardAction
 */
namespace KGameStandardAction
{
/*!
 * \enum KGameStandardAction::GameStandardAction
 * The standard menubar and toolbar actions.
 * \value New
 *        Start a new game
 * \value Load
 *        Open a saved game
 * \value LoadRecent
 *        Open a recently saved game
 * \value Save
 *        Save the current game
 * \value SaveAs
 *        Save the current game to another file
 * \value End
 *        End the current game
 * \value Pause
 *        Pause the game
 * \value Highscores
 *        Show high scores
 * \value Statistics
 *        Show statistics
 * \value Print
 *        Print game
 * \value Quit
 *        Quit the program
 * \value Repeat
 *        Repeat the last move
 * \value Undo
 *        Undo the last move
 * \value Redo
 *        Redo the latest move
 * \value Roll
 *        Roll the dice
 * \value EndTurn
 *        End turn
 * \value Carddecks
 *        Configure card decks
 * \value ConfigureHighscores
 *        Configure high scores
 * \value ClearHighscores
 *        Clear high scores
 * \value ClearStatistics
 *        Delete all-time statistics
 * \value Restart
 *        Restart the game
 * \value Hint
 *        Give a hint
 * \value Demo
 *        Play a demo
 * \value Solve
 *        Solve the game
 * \value ActionNone
 */
enum GameStandardAction {
    // Game menu
    New = 1,
    Load,
    LoadRecent,
    Save,
    SaveAs,
    End,
    Pause,
    Highscores,
    Statistics,
    Print,
    Quit,
    // Move menu
    Repeat,
    Undo,
    Redo,
    Roll,
    EndTurn,
    // Settings menu
    Carddecks,
    ConfigureHighscores,
    ClearHighscores,
    ClearStatistics,
    Restart,
    Hint,
    Demo,
    Solve,
    ActionNone
};

/*!
 * \overload primary
 * Creates an action corresponding to the \a id in the KStandardAction::StandardAction enum,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *create(GameStandardAction id, const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \internal
 */
KDEGAMES_EXPORT QAction *_k_createInternal(GameStandardAction id, QObject *parent);

/*!
 * \overload
 * This overloads create() to allow using the new connect syntax.
 *
 * Creates an action corresponding to the \a id in the KStandardAction::StandardAction enum,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 *
 * \note If you use \c LoadRecent as \a id, you should manually connect to the urlSelected(const QUrl &)
 * signal of the returned KRecentFilesAction instead or use KGameStandardAction::loadRecent(Receiver *, Func, QObject*).
 *
 * \since 7.3
 */
template<class Receiver, class Func>
inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
create(GameStandardAction id, const Receiver *recvr, Func slot, QObject *parent)
{
    QAction *action = _k_createInternal(id, parent);
    QObject::connect(action, &QAction::triggered, recvr, slot);
    return action;
}

/*!
 * Returns the internal name of the given standard action \a id.
 */
KDEGAMES_EXPORT QString name(GameStandardAction id);

// we have to disable the templated function for const char* as Func, since it is ambiguous otherwise
// TODO: KF6: unify const char* version and new style by removing std::enable_if
#define KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(name, enumValue)                                                                                            \
    template<class Receiver, class Func>                                                                                                                       \
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *name(const Receiver *recvr, Func slot, QObject *parent)    \
    {                                                                                                                                                          \
        return create(enumValue, recvr, slot, parent);                                                                                                         \
    }
#define KSTANDARDGAMETOGGLEACTION_WITH_NEW_STYLE_CONNECT(name, enumValue)                                                                                      \
    template<class Receiver, class Func>                                                                                                                       \
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, KToggleAction>::type *name(const Receiver *recvr,                          \
                                                                                                               Func slot,                                      \
                                                                                                               QObject *parent)                                \
    {                                                                                                                                                          \
        QAction *ret = create(enumValue, recvr, slot, parent);                                                                                                 \
        Q_ASSERT(qobject_cast<KToggleAction *>(ret));                                                                                                          \
        return static_cast<KToggleAction *>(ret);                                                                                                              \
    }

/*!
 * Returns an action to start a new game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *gameNew(const QObject *recvr, const char *slot, QObject *parent);

// clang-format off

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *gameNew(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to start a new game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(gameNew, New)

/*!
 * Returns an action to load a previously saved game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *load(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *load(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to load a previously saved game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(load, Load)

// FIXME why not to delete this and use just KStandardAction::openRecent???
// loadRecent seems to mimic its behaviour
/*!
 * Returns an action to load a recently loaded game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * The signature of slot is of the form slotURLSelected(const QUrl&).
 */
KDEGAMES_EXPORT KRecentFilesAction *loadRecent(const QObject *recvr, const char *slot, QObject *parent);
/*!
 * Returns an action to load a recently loaded game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
template<class Receiver, class Func>
inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, KRecentFilesAction>::type *
loadRecent(const Receiver *recvr, Func slot, QObject *parent)
{
    QAction *action = _k_createInternal(LoadRecent, parent);
    KRecentFilesAction *recentAction = qobject_cast<KRecentFilesAction *>(action);
    Q_ASSERT(recentAction);
    QObject::connect(recentAction, &KRecentFilesAction::urlSelected, recvr, slot);
    return recentAction;
}

/*!
 * Returns an action to save the current game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *save(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *save(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to save the current game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(save, Save)

/*!
 * Returns an action to save the current game under a different filename,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *saveAs(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *saveAs(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to save the current game under a different filename,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(saveAs, SaveAs)

/*!
 * Returns an action to pause the game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT KToggleAction *pause(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, KToggleAction>::type *pause(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to pause the game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMETOGGLEACTION_WITH_NEW_STYLE_CONNECT(pause, Pause)

/*!
 * Returns an action to show the highscores,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *highscores(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *highscores(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to show the highscores,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(highscores, Highscores)

/*!
 * Returns an action to clear highscores,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *clearHighscores(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *clearHighscores(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to clear highscores,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(clearHighscores, ClearHighscores)

/*!
 * Returns an action to show the statistics,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * Show the statistics.
 */
KDEGAMES_EXPORT QAction *statistics(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *statistics(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to show the statistics,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(statistics, Statistics)

/*!
 * Returns an action to clear statistics,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *clearStatistics(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *clearStatistics(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to clear statistics,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(clearStatistics, ClearStatistics)

/*!
 * Returns an action to end the current game without quitting the program, similar to a "close" entry,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *end(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *end(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to end the current game without quitting the program, similar to a "close" entry,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(end, End)

/*!
 * Returns an action to print the current game, not useful in all games,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *print(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *print(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to print the current game, not useful in all games,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(print, Print)

/*!
 * Returns an action to quit the game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *quit(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *quit(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to quit the game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(quit, Quit)

/*!
 * Returns an action to repeat the last move,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *repeat(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *repeat(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to repeat the last move,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(repeat, Repeat)

/*!
 * Returns an action to undo the last move,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *undo(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *undo(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to undo the last move,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(undo, Undo)

/*!
 * Returns an action to redo the latest move that has been undone,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *redo(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *redo(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to redo the latest move that has been undone,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(redo, Redo)

/*!
 * Returns an action to roll a die or dice,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *roll(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *roll(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to roll a die or dice,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(roll, Roll)

/*!
 * Returns an action to end the current turn (not the game), usually to let the next player start,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *endTurn(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *endTurn(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to end the current turn (not the game), usually to let the next player start,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(endTurn, EndTurn)

/*!
 * Returns an action to display a configure card decks dialog,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *carddecks(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *carddecks(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to display a configure card decks dialog,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(carddecks, Carddecks)

/*!
 * Returns an action to display a configure highscores dialog,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * Display configure highscores dialog.
 */
KDEGAMES_EXPORT QAction *configureHighscores(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *configureHighscores(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to display a configure highscores dialog,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(configureHighscores, ConfigureHighscores)

/*!
 * Returns an action to give advice or a hint,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *hint(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *hint(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to give advice or a hint,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(hint, Hint)

/*!
 * Returns an action to show a demo,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT KToggleAction *demo(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, KToggleAction>::type *demo(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to show a demo,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMETOGGLEACTION_WITH_NEW_STYLE_CONNECT(demo, Demo)

/*!
 * Returns an action to solve the game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *solve(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *solve(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to solve the game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(solve, Solve)

/*!
 * Returns an action to restart the game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 */
KDEGAMES_EXPORT QAction *restart(const QObject *recvr, const char *slot, QObject *parent);

/*!
 * \fn template<class Receiver, class Func> inline typename std ::enable_if<!std ::is_convertible<Func, const char *>::value, QAction>::type *restart(const Receiver *recvr, Func slot, QObject *parent)
 * Returns an action to restart the game,
 * connected to the receiver \a recvr's slot \a slot, with parent \a parent.
 * \since 7.3
 */
KSTANDARDGAMEACTION_WITH_NEW_STYLE_CONNECT(restart, Restart)
}

#endif
