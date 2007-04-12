/****************************************************************
Copyright (c) 1998 Sandro Sigala <ssigala@globalnet.it>
Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
Copyright (c) 2007 Matt Williams <matt@milliams.com>
All rights reserved.

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of the author not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

The author disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall the author be liable for any
special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
****************************************************************/

#ifndef KSCOREDIALOG_H
#define KSCOREDIALOG_H

#include <KDialog>
#include <libkdegames_export.h>

#include <QtCore/QMap>
#include <QtCore/QFlags>
/**
 * @short A simple high score implementation
 * 
 * This class can be used both for displaying the current high scores
 * and also for adding new highscores. It is the recommended way of 
 * implementing a simple highscore table.
 * 
 * To display the current highscores it is simply a case of creating
 * a KScoreDialog object and calling exec. This example code will
 * display the Name and Score of the top 10 players:
 * \code
 * KScoreDialog ksdialog(KScoreDialog::Name | KScoreDialog::Score, this);
 * ksdialog.exec();
 * \endcode
 * To add a new highscore, e.g. at the end of a game you simply create an
 * object with the @ref Fields you want to write (i.e. KScoreDialog::Name | 
 * KScoreDialog::Score), call addScore and then (optionally) display
 * the dialog.
 * This code will allow you to add a highscore with a Name and Score
 * field. If it's the first time a player has a score on the table, they
 * will be prompted for their name but subsequent times they will have
 * their name filled in automatically.
 * \code
 * KScoreDialog ksdialog(KScoreDialog::Name | KScoreDialog::Score, this);
 * ksdialog.addScore(playersScore);
 * ksdialog.exec();
 * \endcode
 * If you want to prompt the player for their name every time do something
 * like
 * \code
 * ksdialog.addScore(playersScore, KScoreDialog::FieldInfo(), KScoreDialog::AskName);
 * \endcode
 * Or if you want to fill the name in from the code you can pass a default
 * name by doing
 * \code
 * ksdialog.addScore(playerScore, KScoreDialog::FieldInfo(), KScoreDialog::AskName, playerName);
 * \endcode
 */
class KDEGAMES_EXPORT KScoreDialog : public KDialog 
{
    Q_OBJECT

    public:
        ///Highscore fields
        enum Fields { 
            Name = 1 << 0,
            Level = 1 << 1,
            
            Custom1 = 1 << 10,
            Custom2 = 1 << 11,
            Custom3 = 1 << 12,
            
            Date = 1 << 27,
            Time = 1 << 28,
            Score = 1 << 29
        };

        ///Flags for setting preferences for adding scores
        enum AddScoreFlag {
            AskName = 0x1, /**< Promt the player for their name */
            LessIsMore = 0x2 /**< A lower numerical score means higher placing on the table */
        };
        Q_DECLARE_FLAGS(AddScoreFlags, AddScoreFlag)

        typedef QMap<int, QString> FieldInfo;

        /**
        * @param fields Bitwise OR of the @ref Fields that should be listed.
        * @param parent passed to parent QWidget constructor.
        */
        KScoreDialog(int fields, QWidget *parent);

        ~KScoreDialog();

        /**
        * @param group to use for reading/writing highscores from/to.
        * If you get a group, it will be prefixed by 'KHighscore_' otherwise
        * the group will simply be 'KHighscore'
        */
        void setConfigGroup(const QString& group = QString());

        /**
        * @param comment to add when showing high-scores.
        * The comment is only used once.  
        */
        void setComment(const QString& comment);

        /**
        * Define an extra FieldInfo entry.
        * @param field Id of this field
        * @param header Header shown in the dialog for this field
        * @param key used to store this field with.
        */
        void addField(int field, const QString& header, const QString& key); 

        /**
        * Adds a new score to the list.
        *
        * @param newScore the score of this game.
        * @param newInfo additional info about the score.
        * @param flags set whether the user should be prompted for their name and how the scores should be sorted
        * @param name the name of the player
        *
        * @returns The highscore position if the score was good enough to 
        * make it into the list (1 being topscore) or 0 otherwise.
        */
        int addScore(int newScore, const FieldInfo& newInfo = FieldInfo(), const AddScoreFlags& flags=AskName, const QString& name=QString());

        /**
        * @returns the current best score.
        */
        int highScore();

        virtual void show();
        virtual void exec();

        private Q_SLOTS:
            void slotGotReturn();
            void slotGotName();

        private:
            void loadScores();
            void saveScores();

            void aboutToShow();
            void setupDialog();
            void keyPressEvent( QKeyEvent *ev);

        private:
            class KScoreDialogPrivate;
            KScoreDialogPrivate* const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KScoreDialog::AddScoreFlags)

#endif // !KSCOREDIALOG_H
