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
#include <qptrlist.h>

#include <kdialogbase.h>

class QGridLayout;
class QLineEdit;
class QWidgetStack;

/**
 * A simple high score dialog.
 */
class KScoreDialog : public KDialogBase {
   Q_OBJECT
   
public:
   enum Fields { None = 0, Name = 1, Score = 2, Level = 4, Time = 8 };
        
   typedef QMap<Fields, QString> FieldInfo;

   /**
    * @param latest Ranking of the latest entry. [1 - 10]
    * Use 0 for none.
    * @param fields Which fields should be listed.
    */
   KScoreDialog(int fields, QWidget *parent=0, const char *name=0);

   ~KScoreDialog();

   /**
    * Adds a new score to the list.
    *
    * Returns true when the score was good enough to make it into
    * the list.
    */   
   bool addScore(int newScore, const FieldInfo &newInfo, bool askName=true);

   virtual void show();

private slots:
   void slotGotReturn();
   void slotGotName();

private:
   /* read scores */
   void loadScores();   
   void saveScores();
   
   void aboutToShow();

private:           
   class KScoreDialogPrivate;
   KScoreDialogPrivate *d;
};

#endif // !KSCOREDIALOG_H
