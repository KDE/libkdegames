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

#include "config.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qwidgetstack.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kseparator.h>

#include "kscoredialog.h"

class KScoreDialog::KScoreDialogPrivate
{ 
public:  
   QPtrList<FieldInfo> scores;
   QWidget *page;
   QGridLayout *layout;
   QLineEdit *edit;
   QWidgetStack *stack;
   int fields;
   int newName;
   int latest;
   
   QMap<Fields, int> col;
   QString player;
};


KScoreDialog::KScoreDialog(int fields, QWidget *parent, const char *oname)
        : KDialogBase(parent, oname, true, i18n("%1 High Scores").arg(kapp->caption()), Ok, Ok, true)
{
    d = new KScoreDialogPrivate();
    d->edit = 0;
    d->fields = fields;
    d->newName = -1;
    d->latest = -1;
    
    d->scores.setAutoDelete(true);

    d->page = makeMainWidget();
    int nrCols = 1;
    if (fields & Name)
       d->col[Name] = nrCols++;
       
    if (fields & Level)
       d->col[Level] = nrCols++;
       
    if (fields & Score)
       d->col[Score] = nrCols++;
       
    d->layout = new QGridLayout(d->page, 12, nrCols, marginHint() + 20, spacingHint());
    d->layout->addRowSpacing(1, 15);

    QFont bold = font();
    bold.setBold(true);

    QLabel *label;

    d->layout->addColSpacing(0, 50);
    label = new QLabel(i18n("Rank"), d->page);
    d->layout->addWidget(label, 0, 0);
    label->setFont(bold);

    if (fields & Name)
    {
       d->layout->addColSpacing(d->col[Name], 50);

       label = new QLabel(i18n("Name"), d->page);
       d->layout->addWidget(label, 0, d->col[Name]);
       label->setFont(bold);
    }

    if (fields & Level)
    {
       d->layout->addColSpacing(d->col[Level], 50);

       label = new QLabel(i18n("Level"), d->page);
       d->layout->addWidget(label, 0, d->col[Level], AlignRight);
       label->setFont(bold);
    }

    if (fields & Score)
    {
       d->layout->addColSpacing(d->col[Score], 50);

       label = new QLabel(i18n("Score"), d->page);
       d->layout->addWidget(label, 0, d->col[Score], AlignRight);
       label->setFont(bold);
    }
    
    KSeparator *sep = new KSeparator(Horizontal, d->page);
    d->layout->addMultiCellWidget(sep, 1, 1, 0, nrCols-1);

    loadScores();
}

KScoreDialog::~KScoreDialog()
{
    delete d;
}

void KScoreDialog::aboutToShow()
{
    QFont bold = font();
    bold.setBold(true);

    QString num;
    for (int i = 1; i <= 10; ++i) {
        QLabel *label;
        num.setNum(i);
        FieldInfo *score = d->scores.at(i-1);
	label = new QLabel("#"+num, d->page);
	if (i == d->latest) label->setFont(bold);
        d->layout->addWidget(label, i+1, 0);
        if (d->fields & Name)
        {
           if (d->newName == i)
           {
              d->stack = new QWidgetStack(d->page);
              d->edit = new QLineEdit(d->player, d->stack);
              d->edit->setMinimumWidth(40);
              d->stack->addWidget(d->edit);
              d->stack->raiseWidget(d->edit);
              d->layout->addWidget(d->stack, i+1, d->col[Name]);
              d->edit->setFocus();
              connect(d->edit, SIGNAL(returnPressed()), 
                      this, SLOT(slotGotReturn()));
              enableButtonOK(false);
           }
           else
           {
              label = new QLabel((*score)[Name], d->page);
              if (i == d->latest) label->setFont(bold);
              d->layout->addWidget(label, i+1, d->col[Name]);
           }
           
        }
        if (d->fields & Level)
        {
	    label = new QLabel((*score)[Level], d->page);
	    if (i == d->latest) label->setFont(bold);
            d->layout->addWidget(label, i+1, d->col[Level], AlignRight);
        }
        if (d->fields & Score)
        {
            label = new QLabel((*score)[Score], d->page);
            if (i == d->latest) label->setFont(bold);
            d->layout->addWidget(label, i+1, d->col[Score], AlignRight);
        }
    }
    incInitialSize(QSize(), true); // Fixed.
}

void KScoreDialog::loadScores()
{
    QString key, value;
    d->scores.clear();
    KConfigGroup config(kapp->config(), "High Score");

    d->player = config.readEntry("LastPlayer");

    QString num;
    for (int i = 1; i <= 10; ++i) {
        num.setNum(i);
        FieldInfo *score = new FieldInfo();
        if (d->fields & Level)
        {
           key = "Pos" + num + "Level";
           (*score)[Level] = config.readEntry(key, "0");
        }
        if (d->fields & Score)
        {
           key = "Pos" + num + "Score";
           (*score)[Score] = config.readEntry(key, "0");
        }
        if (d->fields & Name)
        {
           key = "Pos" + num + "Name";
           (*score)[Name] = config.readEntry(key, i18n("Noname"));
        }
        d->scores.append(score);
    }
}

void KScoreDialog::saveScores()
{
    QString key, value;
    KConfigGroup config(kapp->config(), "High Score");

    config.writeEntry("LastPlayer", d->player);

    QString num;
    for (int i = 1; i <= 10; ++i) {
        num.setNum(i);
        FieldInfo *score = d->scores.at(i-1);
        if (d->fields & Level)
        {
           key = "Pos" + num + "Level";
           config.writeEntry(key, (*score)[Level]);
        }
        if (d->fields & Score)
        {
           key = "Pos" + num + "Score";
           config.writeEntry(key, (*score)[Score]);
        }
        if (d->fields & Name)
        {
           key = "Pos" + num + "Name";
           config.writeEntry(key, (*score)[Name]);
        }
    }
    kapp->config()->sync();
}

bool KScoreDialog::addScore(int newScore, const FieldInfo &newInfo, bool askName)
{
    FieldInfo *score = d->scores.first();
    int i = 1;
    for(; score; score = d->scores.next(), i++)
    {
       int num_score = (*score)[Score].toLong();
       if (newScore > num_score)
       {
          score = new FieldInfo(newInfo);
          (*score)[Score].setNum(newScore);
          d->scores.insert(i-1, score);
          d->scores.remove(10);
          d->latest = i;
          if (askName)
             d->newName = i;
          else
             saveScores();
          return true;
       }
    }
    return false;
}

void KScoreDialog::show()
{
    aboutToShow();
    KDialogBase::show();
}

void KScoreDialog::slotGotReturn()
{
    QTimer::singleShot(0, this, SLOT(slotGotName()));
}

void KScoreDialog::slotGotName()
{
    if (d->newName == -1) return;
    
    d->player = d->edit->text();
    
    (*d->scores.at(d->newName-1))[Name] = d->player;
    saveScores();

    QFont bold = font();
    bold.setBold(true);
    
    QLabel *label = new QLabel(d->player, d->stack);
    label->setFont(bold);
    d->stack->addWidget(label);
    d->stack->raiseWidget(label);
    d->newName = -1;
    enableButtonOK(true);
}

#include "kscoredialog.moc"
