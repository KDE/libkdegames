/*
    SPDX-FileCopyrightText: 1998 Sandro Sigala <ssigala@globalnet.it>
    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2007 Matt Williams <matt@milliams.com>

    SPDX-License-Identifier: ICS
*/

#ifndef KSCOREDIALOG_H
#define KSCOREDIALOG_H

// own
#include <kdegames_export.h>
// Qt
#include <QDialog>
#include <QMap>
// Std
#include <memory>

class KGameDifficulty;

/**
 * \class KScoreDialog kscoredialog.h <KScoreDialog>
 *
 * @short A simple high score implementation
 *
 * This class can be used both for displaying the current high scores
 * and also for adding new highscores. It is the recommended way of
 * implementing a simple highscore table.
 *
 * To display the current highscores it is simply a case of creating
 * a KScoreDialog object and calling exec(). This example code will
 * display the Name and Score (the score is always added automatically
 * unless hidden @ref hideField since it is used for sorting) of the
 * top 10 players:
 * \code
 * KScoreDialog ksdialog(this);
 * ksdialog.exec();
 * \endcode
 *
 * To add a new highscore, e.g. at the end of a game you simply create an
 * object with the @ref Fields you want to write (i.e. KScoreDialog::Name |
 * KScoreDialog::Score), call addScore and then (optionally) display
 * the dialog.
 * This code will allow you to add a highscore with a Name and Score
 * field. If it's the first time a player has a score on the table, they
 * will be prompted for their name but subsequent times they will have
 * their name filled in automatically.
 * \code
 * KScoreDialog ksdialog(this);
 * ksdialog.addScore(playersScore);
 * ksdialog.exec();
 * \endcode
 *
 * Or if you want to fill the name in from the code you can pass a default
 * name by doing
 * \code
 * KScoreDialog::FieldInfo scoreInfo;
 * scoreInfo[KScoreDialog::Name] = "Matt";
 * scoreInfo[KScoreDialog::Score].setNum(playersScore);
 * ksdialog.addScore(scoreInfo);
 * \endcode
 *
 * If you want to add an extra field (e.g. the number of moves taken) then
 * do
 * \code
 * KScoreDialog::FieldInfo scoreInfo;
 * scoreInfo[KScoreDialog::Name] = "Matt";
 * scoreInfo[KScoreDialog::Score].setNum(playersScore);
 *
 * ksdialog.addField(KScoreDialog::Custom1, "Num of Moves", "moves");
 * scoreInfo[KScoreDialog::Custom1].setNum(42);
 *
 * ksdialog.addScore(scoreInfo);
 * \endcode
 * You can define up to 5 Custom fields.
 * @author Matt Williams <matt@milliams.com>
 */
class KDEGAMES_EXPORT KScoreDialog : public QDialog
{
    Q_OBJECT

public:
    /// Highscore fields
    enum Fields {
        Name = 1 << 0,
        Level = 1 << 1,
        Date = 1 << 2,
        Time = 1 << 3,
        Score = 1 << 4,

        Custom1 = 1 << 10, ///< Field for custom information
        Custom2 = 1 << 11,
        Custom3 = 1 << 12,
        Custom4 = 1 << 13,
        Custom5 = 1 << 14,

        Max = 1 << 30 ///< Only for setting a maximum
    };

    /// Flags for setting preferences for adding scores
    enum AddScoreFlag {
        AskName = 0x1, /**< Promt the player for their name */
        LessIsMore = 0x2 /**< A lower numerical score means higher placing on the table */
    };
    /**
     * Stores a combination of #AddScoreFlag values.
     */
    Q_DECLARE_FLAGS(AddScoreFlags, AddScoreFlag)

    typedef QMap<int, QString> FieldInfo;

    /**
     * @param fields Bitwise OR of the @ref Fields that should be listed (Score is always present)
     * @param parent passed to parent QWidget constructor.
     */
    explicit KScoreDialog(int fields = Name, QWidget *parent = nullptr);

    ~KScoreDialog() override;

    /**
     * The group name must be passed though i18n() in order for the
     * group name to be translated. i.e.
     * \code ksdialog.setConfigGroup(qMakePair(QByteArray("Easy"), i18n("Easy"))); \endcode
     * If you set a group, it will be prefixed in the config file by
     * 'KHighscore_' otherwise the group will simply be 'KHighscore'.
     *
     * @param group to use for reading/writing highscores from/to.
     */
    void setConfigGroup(const QPair<QByteArray, QString> &group);

    /**
     * You must add the translations of all group names to the dialog. This
     * is best done by passing the name through i18n().
     * The group set through setConfigGroup(const QPair<QByteArray, QString>& group)
     * will be added automatically
     *
     * @param group the translated group name
     */
    void addLocalizedConfigGroupName(const QPair<QByteArray, QString> &group);

    /**
     * You must add the translations of all group names to the dialog. This
     * is best done by passing the name through i18n().
     * The group set through setConfigGroup(const QPair<QByteArray, QString>& group)
     * will be added automatically.
     *
     * This function can be used directly with KGameDifficulty::localizedLevelStrings().
     *
     * @param groups the list of translated group names
     */
    void addLocalizedConfigGroupNames(const QMap<QByteArray, QString> &groups);

    /**
     * Hide some config groups so that they are not shown on the dialog
     * (but are still stored in the configuration file).
     * \code
     * ksdialog.setHiddenConfigGroups(QList<QByteArray>() << "Very Easy" << "Easy");
     * \endcode
     *
     * @param hiddenGroups the list of group names you want to hide
     *
     * @since KDE 4.6
     */
    void setHiddenConfigGroups(const QList<QByteArray> &hiddenGroups);

    /**
     * It is a good idea giving config group weights, otherwise tabs
     * get ordered by their tab name that is not probably what you want.
     *
     * This function can be used directly with KGameDifficulty::levelWeights().
     *
     * @param weights the list of untranslated group weights
     *
     * @since KDE 4.2
     */
    void setConfigGroupWeights(const QMap<int, QByteArray> &weights);

    /**
     * @param comment to add when showing high-scores.
     * The comment is only used once.
     */
    void setComment(const QString &comment);

    /**
     * Define an extra FieldInfo entry.
     * @param field id of this field @ref Fields e.g. KScoreDialog::Custom1
     * @param header text shown in the header in the dialog for this field. e.g. "Number of Moves"
     * @param key unique key used to store this field. e.g. "moves"
     */
    void addField(int field, const QString &header, const QString &key);

    /**
     * Hide a field so that it is not shown on the table (but is still stored in the configuration file).
     * @param field id of this field @ref Fields e.g. KScoreDialog::Score
     */
    void hideField(int field);

    /**
     * Adds a new score to the list.
     *
     * @param newInfo info about the score.
     * @param flags set whether the user should be prompted for their name and how the scores should be sorted
     *
     * @returns The highscore position if the score was good enough to
     * make it into the list (1 being topscore) or 0 otherwise.
     */
    int addScore(const FieldInfo &newInfo = FieldInfo(), const AddScoreFlags &flags = {});

    /**
     * Convenience function for ease of use.
     *
     * @param newScore the score of the player.
     * @param flags set whether the user should be prompted for their name and how the scores should be sorted
     *
     * @returns The highscore position if the score was good enough to
     * make it into the list (1 being topscore) or 0 otherwise.
     */
    int addScore(int newScore, const AddScoreFlags &flags = {});

    /**
     * @returns the current best score in the group
     */
    int highScore();

    /**
     * Assume that config groups (incl. current selection) are equal to
     * difficulty levels, and initialize them. This is usually equal to the
     * following code using KGameDifficulty:
     * @code
     * addLocalizedConfigGroupNames(KGameDifficulty::localizedLevelStrings());
     * setConfigGroupWeights(KGameDifficulty::levelWeights());
     * setConfigGroup(KGameDifficulty::localizedLevelString());
     * @endcode
     */
    void initFromDifficulty(const KGameDifficulty *difficulty, bool setConfigGroup = true);

    /// Display the dialog as non-modal
    virtual void show();
    /// Display the dialog as modal
    int exec() override;

private Q_SLOTS:
    void slotGotReturn();
    void slotGotName();
    void slotForgetScore();

private:
    void keyPressEvent(QKeyEvent *ev) override;

private:
    friend class KScoreDialogPrivate;
    std::unique_ptr<class KScoreDialogPrivate> const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KScoreDialog::AddScoreFlags)

#endif // KSCOREDIALOG_H
