/****************************************************************
Copyright (c) 1998 Sandro Sigala <ssigala@globalnet.it>.
Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
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

#include <qmap.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QKeyEvent>

#include <kdialogbase.h>
#include <kdemacros.h>
class QGridLayout;
class QLineEdit;
class Q3WidgetStack;

/**
 * A simple high score dialog.
 */
class KDE_EXPORT KScoreDialog : public KDialogBase {
   Q_OBJECT
   
public:
   enum Fields { Name = 1 << 0, 
                 Level = 1 << 1, 
                 
                 Custom1 = 1 << 10,
                 Custom2 = 1 << 11,
                 Custom3 = 1 << 12,   	
                 
                 Date = 1 << 27, 
                 Time = 1 << 28, 
                 Score = 1 << 29 };
        
   typedef QMap<int, QString> FieldInfo;

   /**
    * @param fields Which fields should be listed.
    * @param parent passed to parent QWidget constructor
    * @param name passed to parent QWidget constructor
    */
   KScoreDialog(int fields, QWidget *parent=0, const char *name=0);

   ~KScoreDialog();

   /**
    * @param group to use for reading/writing highscores from/to. By default
    * the class will use "High Score"
    */
   void setConfigGroup(const QString &group);

   /**
    * @param comment to add when showing high-scores.
    * The comment is only used once.  
    */
   void setComment(const QString &comment);

   /**
    * Define an extra FieldInfo entry.
    * @param field Id of this field
    * @param header Header shown in the dialog for this field
    * @param key used to store this field with.
    */
   void addField(int field, const QString &header, const QString &key); 

   /**
    * Adds a new score to the list.
    *
    * @param newScore the score of this game.
    * @param newInfo additional info about the score.
    * @param askName Whether to prompt for the players name.
    * @param lessIsMore If true, the lowest score is the best score.
    *
    * @returns The highscore position if the score was good enough to 
    * make it into the list (1 being topscore) or 0 otherwise.
    */   
   int addScore(int newScore, const FieldInfo &newInfo, bool askName, bool lessIsMore);
   int addScore(int newScore, const FieldInfo &newInfo, bool askName=true);

   /**
    * Returns the current best score.
    */
   int highScore();

   virtual void show();

private slots:
   void slotGotReturn();
   void slotGotName();

private:
   /* read scores */
   void loadScores();   
   void saveScores();
   
   void aboutToShow();
   void setupDialog();
   void keyPressEvent( QKeyEvent *ev);

private:           
   class KScoreDialogPrivate;
   KScoreDialogPrivate *d;
};

#endif // !KSCOREDIALOG_H
