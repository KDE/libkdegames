/*
    SPDX-FileCopyrightText: 2007 Nicolas Roffet <nicolas-kde@roffet.com>
    SPDX-FileCopyrightText: 2007 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2011-2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMEDIFFICULTY_H
#define KGAMEDIFFICULTY_H

// own
#include "kdegames_export.h"
// Qt
#include <QMetaType>
#include <QObject>
// Std
#include <memory>

/**
 * @class KGameDifficultyLevel kgamedifficulty.h <KGameDifficultyLevel>
 * @see KGameDifficulty
 */
class KDEGAMES_EXPORT KGameDifficultyLevel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KGameDifficultyLevel)
    Q_PROPERTY(bool default READ isDefault)
    Q_PROPERTY(int hardness READ hardness)
    Q_PROPERTY(QByteArray key READ key)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(StandardLevel standardLevel READ standardLevel)

public:
    enum StandardLevel {
        Custom = -1, ///< standardLevel() returns this for custom levels.
        RidiculouslyEasy = 10,
        VeryEasy = 20,
        Easy = 30,
        Medium = 40,
        Hard = 50,
        VeryHard = 60,
        ExtremelyHard = 70,
        Impossible = 80
    };
    Q_ENUM(StandardLevel)

    /// Refer to the getters' documentation for details on the params.
    KGameDifficultyLevel(int hardness, const QByteArray &key, const QString &title, bool isDefault = false);
    explicit KGameDifficultyLevel(StandardLevel level, bool isDefault = false);
    ~KGameDifficultyLevel() override;

    /// @return whether this level is the default level when no selection has
    ///        been stored (e.g. on first startup)
    bool isDefault() const;
    /// @return a numeric key which is used to sort the levels by difficulty
    ///        (smaller values mean easier levels)
    /// @note For standard levels, this equals the numeric value of the level
    ///      in the StandardLevel enumeration.
    int hardness() const;
    /// @return a @b non-localized key for this level
    QByteArray key() const;
    /// @return a @b localized title for this level
    QString title() const;
    /// @return the standard level which was used to create this level, or
    ///        KGameDifficultyLevel::Custom for custom levels
    StandardLevel standardLevel() const;

private:
    std::unique_ptr<class KGameDifficultyLevelPrivate> const d;
};

/**
 * @class KGameDifficulty kgamedifficulty.h <KGameDifficulty>
 * @brief KGameDifficulty manages difficulty levels of a game in a standard way.
 *
 * The difficulty can be a type of game (like in KMines: small or big field) or
 * the AI skills (like in Bovo: how deep should the computer search to find the
 * best move) or a combination of both of them. On the user point of view, it's
 * not really different: either is the game easy or hard to play.
 *
 * KGameDifficulty contains a list of KGameDifficultyLevel instances. One of
 * these levels is selected; this selection will be recorded when the
 * application is closed. A set of standard difficulty levels is provided by
 * KGameDifficultyLevel, but custom levels can be defined at the same time.
 */
class KDEGAMES_EXPORT KGameDifficulty : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KGameDifficulty)
    // Use currentLevel in game logic and selectedLevel in level selection UI.
    Q_PROPERTY(const KGameDifficultyLevel *currentLevel READ currentLevel WRITE select NOTIFY currentLevelChanged)
    Q_PROPERTY(const KGameDifficultyLevel *selectedLevel READ currentLevel WRITE select NOTIFY selectedLevelChanged)
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable NOTIFY editableChanged)
    Q_PROPERTY(bool gameRunning READ isGameRunning WRITE setGameRunning NOTIFY gameRunningChanged)

public:
    /// @return a singleton instance of KGameDifficulty
    static KGameDifficulty *global();
    /// A shortcut for KGameDifficulty::global()->currentLevel()->standardLevel().
    static KGameDifficultyLevel::StandardLevel globalLevel();

public:
    explicit KGameDifficulty(QObject *parent = nullptr);
    /// Destroys this instance and all DifficultyLevel instances in it.
    ~KGameDifficulty() override;

    /// Adds a difficulty level to this instance. This will not affect the
    /// currentLevel() if there is one.
    void addLevel(KGameDifficultyLevel *level);
    /// A shortcut for addLevel(new KGameDifficultyLevel(@a level)).
    void addStandardLevel(KGameDifficultyLevel::StandardLevel level, bool isDefault = false);
    /// This convenience method adds a range of standard levels to this
    /// instance (including the boundaries). For example:
    /// @code
    /// difficulty.addStandardLevelRange(
    ///     KGameDifficultyLevel::Easy,
    ///     KGameDifficultyLevel::VeryHard
    ///);
    /// @endcode
    /// This adds the levels "Easy", "Medium", "Hard" and "Very hard".
    void addStandardLevelRange(KGameDifficultyLevel::StandardLevel from, KGameDifficultyLevel::StandardLevel to);
    /// @overload
    /// This overload allows to specify a @a defaultLevel.
    void
    addStandardLevelRange(KGameDifficultyLevel::StandardLevel from, KGameDifficultyLevel::StandardLevel to, KGameDifficultyLevel::StandardLevel defaultLevel);

    /// @return a list of all difficulty levels, sorted by hardness
    QList<const KGameDifficultyLevel *> levels() const;
    /// @return the current difficulty level
    ///
    /// After the KGameDifficulty object has been created, the current
    /// difficulty level will not be determined until this method is called
    /// for the first time. This allows the application developer to set up
    /// the difficulty levels before KGameDifficulty retrieves the last
    /// selected level from the configuration file.
    const KGameDifficultyLevel *currentLevel() const;

    /// @return whether the difficulty level selection may be edited
    bool isEditable() const;
    /// @return whether a running game has been marked @see setGameRunning
    bool isGameRunning() const;
    /// Set whether the difficulty level selection may be edited. The
    /// default value is true.
    void setEditable(bool editable);
    /// KGameDifficulty has optional protection against changing the
    /// difficulty level while a game is running. If setGameRunning(true) has
    /// been called, and select() is called to select a new difficulty level,
    /// the user will be asked for confirmation.
    void setGameRunning(bool running);
Q_SIGNALS:
    /// Emitted when the editability changes. @see setEditable
    void editableChanged(bool editable);
    /// Emitted when a running game has been marked or unmarked. @see setGameRunning
    void gameRunningChanged(bool gameRunning);
    /// Emitted when a new difficulty level has been selected.
    void currentLevelChanged(const KGameDifficultyLevel *level);
    /// Emitted after every call to select(), even when the user has rejected
    /// the change. This is useful to reset a difficulty level selection UI
    /// after a rejected change.
    void selectedLevelChanged(const KGameDifficultyLevel *level);
public Q_SLOTS:
    /// Select a new difficulty level. The given level must already have been
    /// added to this instance.
    /// @note This does nothing if isEditable() is false. If a game is
    /// running (according to setGameRunning()), the user will be asked for
    /// confirmation before the new difficulty level is selected.
    void select(const KGameDifficultyLevel *level);

private:
    std::unique_ptr<class KGameDifficultyPrivate> const d;
};

Q_DECLARE_METATYPE(const KGameDifficultyLevel *)

class KXmlGuiWindow;

/**
 * @namespace KGameDifficultyGUI
 *
 * The namespace for methods to integrate KGameDifficulty into the UI.
 *
 * @see KGameDifficulty
 */
// TODO KDE5: move this into a separate QtWidgets support library
namespace KGameDifficultyGUI
{
/// Install standard GUI components for the manipulation of the given
/// KGameDifficulty instance in the given @a window.
///
/// Without a second parameter, the KGameDifficulty::global() singleton is used.
KDEGAMES_EXPORT void init(KXmlGuiWindow *window, KGameDifficulty *difficulty = nullptr);
}

#endif // KGAMEDIFFICULTY_H
