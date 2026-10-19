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

/*!
 * \class KGameDifficultyLevel
 * \inmodule LibKDEGames
 * \brief Represents a game's difficulty level.
 *
 * This class represents a game's difficulty level.
 * \sa KGameDifficulty
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
    /*!
     * \enum KGameDifficultyLevel::StandardLevel
     * \value Custom
     *        standardLevel() returns this for custom levels.
     * \value RidiculouslyEasy
     * \value VeryEasy
     * \value Easy
     * \value Medium
     * \value Hard
     * \value VeryHard
     * \value ExtremelyHard
     * \value Impossible
     */
    enum StandardLevel {
        Custom = -1,
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

    /*!
     * Constructs a KGameDifficultyLevel with \a hardness, \a key, \a title, \a isDefault.
     * \sa hardness, key, title, isDefault
     */
    KGameDifficultyLevel(int hardness, const QByteArray &key, const QString &title, bool isDefault = false);
    explicit KGameDifficultyLevel(StandardLevel level, bool isDefault = false);
    ~KGameDifficultyLevel() override;

    /*!
     * \property KGameDifficultyLevel::default
     * \brief Whether this level is the default level when no selection has
     * been stored (for example on first startup).
     */
    bool isDefault() const;

    /*!
     * \property KGameDifficultyLevel::hardness
     * \brief A numeric key used to sort the levels by difficulty
     * (smaller values mean easier levels).
     *
     * \note For standard levels, this equals the numeric value of the level
     * in the StandardLevel enumeration.
     */
    int hardness() const;

    /*!
     * \property KGameDifficultyLevel::key
     * \brief A \b non-localized key for this level.
     */
    QByteArray key() const;

    /*!
     * \property KGameDifficultyLevel::title
     * \brief A \b localized title for this level.
     */
    QString title() const;

    /*!
     * \property KGameDifficultyLevel::standardLevel
     * \brief The standard level which was used to create this level, or
     * KGameDifficultyLevel::Custom for custom levels.
     */
    StandardLevel standardLevel() const;

private:
    std::unique_ptr<class KGameDifficultyLevelPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KGameDifficultyLevel)
};

/*!
 * \class KGameDifficulty
 * \inmodule LibKDEGames
 * \brief KGameDifficulty manages difficulty levels of a game in a standard way.
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
    /*!
     * \property KGameDifficulty::selectedLevel
     * \brief The selected level in the UI.
     */
    Q_PROPERTY(const KGameDifficultyLevel *selectedLevel READ currentLevel WRITE select NOTIFY selectedLevelChanged)
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable NOTIFY editableChanged)
    Q_PROPERTY(bool gameRunning READ isGameRunning WRITE setGameRunning NOTIFY gameRunningChanged)

public:
    /*!
     * Returns a singleton instance of KGameDifficulty.
     */
    static KGameDifficulty *global();
    /*!
     * Returns KGameDifficulty::global()->currentLevel()->standardLevel().
     * \sa KGameDifficultyLevel::standardLevel
     */
    static KGameDifficultyLevel::StandardLevel globalLevel();

public:
    explicit KGameDifficulty(QObject *parent = nullptr);
    /*!
     * Destroys this instance and all DifficultyLevel instances in it.
     */
    ~KGameDifficulty() override;

    /*!
     * Adds the difficulty level \a level to this instance. This will not affect the
     * currentLevel() if there is one.
     */
    void addLevel(KGameDifficultyLevel *level);
    /*!
     * A shortcut for addLevel(new KGameDifficultyLevel(\a level \a isDefault)).
     */
    void addStandardLevel(KGameDifficultyLevel::StandardLevel level, bool isDefault = false);
    /*!
     * Adds a range of standard levels from \a from to \a to
     * to this instance (including the boundaries). For example:
     * \code
     * difficulty.addStandardLevelRange(
     *     KGameDifficultyLevel::Easy,
     *     KGameDifficultyLevel::VeryHard
     *);
     * \endcode
     * This adds the levels "Easy", "Medium", "Hard" and "Very hard".
     */
    void addStandardLevelRange(KGameDifficultyLevel::StandardLevel from, KGameDifficultyLevel::StandardLevel to);
    /*!
     * \overload
     * Adds a range of standard levels from \a from to \a to inclusive,
     * with a default of \a defaultLevel.
     */
    void
    addStandardLevelRange(KGameDifficultyLevel::StandardLevel from, KGameDifficultyLevel::StandardLevel to, KGameDifficultyLevel::StandardLevel defaultLevel);

    /*!
     * Returns a list of all difficulty levels, sorted by hardness.
     */
    QList<const KGameDifficultyLevel *> levels() const;
    /*!
     * \property KGameDifficulty::currentLevel
     * \brief The current difficulty level.
     *
     * After the KGameDifficulty object has been created, the current
     * difficulty level will not be determined until currentLevel() is called
     * for the first time. This allows the application developer to set up
     * the difficulty levels before KGameDifficulty retrieves the last
     * selected level from the configuration file.
     */
    const KGameDifficultyLevel *currentLevel() const;

    /*!
     * \property KGameDifficulty::editable
     * \brief Whether the difficulty level selection may be edited.
     *
     * The default is true.
     */
    bool isEditable() const;
    /*!
     * \property KGameDifficulty::gameRunning
     * \brief Whether a game has been marked as running.
     *
     * If a game is running and select() is called to select a new difficulty level,
     * the user will be asked for confirmation.
     */
    bool isGameRunning() const;
    void setEditable(bool editable);
    void setGameRunning(bool running);
Q_SIGNALS:
    void editableChanged(bool editable);
    void gameRunningChanged(bool gameRunning);
    /*!
     * Emitted when a new difficulty \a level has been selected.
     */
    void currentLevelChanged(const KGameDifficultyLevel *level);
    /*!
     * Emitted after every call to select(), even when the user has rejected
     * the change. This is useful to reset a difficulty level selection UI
     * after a rejected change. \a level is the selected level.
     */
    void selectedLevelChanged(const KGameDifficultyLevel *level);
public Q_SLOTS:
    /*!
     * Selects a new difficulty level. The given \a level must already have been
     * added to this instance.
     * \note This does nothing if isEditable() is false. If a game is
     * running (according to setGameRunning()), the user will be asked for
     * confirmation before the new difficulty level is selected.
     */
    void select(const KGameDifficultyLevel *level);

private:
    std::unique_ptr<class KGameDifficultyPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KGameDifficulty)
};

Q_DECLARE_METATYPE(const KGameDifficultyLevel *)

class KXmlGuiWindow;

/*!
 * \namespace KGameDifficultyGUI
 * \inmodule LibKDEGames
 * \brief Methods to integrate KGameDifficulty into the UI.
 *
 * The namespace for methods to integrate KGameDifficulty into the UI.
 *
 * \sa KGameDifficulty
 */
// TODO KDE5: move this into a separate QtWidgets support library
namespace KGameDifficultyGUI
{
/*!
 * Install standard GUI components for the manipulation of the
 * KGameDifficulty instance \a difficulty in the given \a window.
 *
 * Without a second parameter, the KGameDifficulty::global() singleton is used.
 */
KDEGAMES_EXPORT void init(KXmlGuiWindow *window, KGameDifficulty *difficulty = nullptr);
}

#endif // KGAMEDIFFICULTY_H
