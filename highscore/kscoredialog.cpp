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
#include <QStackedWidget>
#include <qtimer.h>
#include <qevent.h>
#include <q3ptrvector.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QKeyEvent>
#include <Q3PtrList>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kseparator.h>
#include <kglobal.h>

#include "kscoredialog.h"

class KScoreDialog::KScoreDialogPrivate
{ 
public:  
   Q3PtrList<FieldInfo> scores;
   QWidget *page;
   QGridLayout *layout;
   QLineEdit *edit;
   Q3PtrVector<QStackedWidget> stack;
   Q3PtrVector<QLabel> labels;
   QLabel *commentLabel;
   QString comment;
   int fields;
   int newName;
   int latest;
   int nrCols;
   bool loaded;
   QString configGroup;
   
   QMap<int, int> col;
   QMap<int, QString> header;
   QMap<int, QString> key;
   QString player;
};


KScoreDialog::KScoreDialog(int fields, QWidget *parent, const char *oname)
        : KDialogBase(parent, oname, true, i18n("High Scores"), Ok, Ok, true)
{
   d = new KScoreDialogPrivate();
   d->edit = 0;
   d->fields = fields;
   d->newName = -1;
   d->latest = -1;
   d->loaded = false;
   d->nrCols = 0;
   d->configGroup = "High Score";
   
   d->scores.setAutoDelete(true);
   d->header[Name] = i18n("Name");
   d->key[Name] = "Name";
   
   d->header[Date] = i18n("Date");
   d->key[Date] = "Date";
   
   d->header[Level] = i18n("Level");
   d->key[Level] = "Level";
   
   d->header[Score] = i18n("Score");
   d->key[Score] = "Score";
   d->page = makeMainWidget();
   
   connect(this, SIGNAL(okClicked()), SLOT(slotGotName()));
}

KScoreDialog::~KScoreDialog()
{
   delete d;
}

void KScoreDialog::setConfigGroup(const QString &group)
{
   d->configGroup = group;
   d->loaded = false;
}

void KScoreDialog::setComment(const QString &comment)
{
   d->comment = comment;
}

void KScoreDialog::addField(int field, const QString &header, const QString &key)
{
   d->fields |= field;
   d->header[field] = header;
   d->key[field] = key;
}

void KScoreDialog::setupDialog()
{
   d->nrCols = 1;
   
   for(int field = 1; field < d->fields; field = field * 2)
   {
      if (d->fields & field)
         d->col[field] = d->nrCols++;
   }
      
   d->layout = new QGridLayout(d->page, 15, d->nrCols, marginHint() + 20, spacingHint());
   d->layout->addRowSpacing(4, 15);

   d->commentLabel = new QLabel(d->page);
   d->commentLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
   d->layout->addMultiCellWidget(d->commentLabel, 1, 1, 0, d->nrCols-1);

   QFont bold = font();
   bold.setBold(true);

   QLabel *label;
   d->layout->addColSpacing(0, 50);
   label = new QLabel(i18n("Rank"), d->page);
   d->layout->addWidget(label, 3, 0);
   label->setFont(bold);

   for(int field = 1; field < d->fields; field = field * 2)
   {
      if (d->fields & field)
      {
         d->layout->addColSpacing(d->col[field], 50);

         label = new QLabel(d->header[field], d->page);
         d->layout->addWidget(label, 3, d->col[field], field <= Name ? Qt::AlignLeft : Qt::AlignRight);
         label->setFont(bold);
      }
   }

   KSeparator *sep = new KSeparator(Qt::Horizontal, d->page);
   d->layout->addMultiCellWidget(sep, 4, 4, 0, d->nrCols-1);

   d->labels.resize(d->nrCols * 10);
   d->stack.resize(10);

   QString num;
   for (int i = 1; i <= 10; ++i) {
      QLabel *label;
      num.setNum(i);
      label = new QLabel(i18n("#%1", num), d->page);
      d->labels.insert((i-1)*d->nrCols + 0, label);
      d->layout->addWidget(label, i+4, 0);
      if (d->fields & Name)
      {
         QStackedWidget *stack = new QStackedWidget(d->page);
         d->stack.insert(i-1, stack);
         d->layout->addWidget(stack, i+4, d->col[Name]);
         label = new QLabel(d->page);
         d->labels.insert((i-1)*d->nrCols + d->col[Name], label);
         stack->addWidget(label);
         stack->setCurrentWidget(label);
      }
      for(int field = Name * 2; field < d->fields; field = field * 2)
      {
         if (d->fields & field)
         {
           label = new QLabel(d->page);
           d->labels.insert((i-1)*d->nrCols + d->col[field], label);
           d->layout->addWidget(label, i+4, d->col[field], Qt::AlignRight);
         }
      }
   }
}

void KScoreDialog::aboutToShow()
{
   if (!d->loaded)
      loadScores();
      
   if (!d->nrCols)
      setupDialog();

   d->commentLabel->setText(d->comment);
   if (d->comment.isEmpty())
   {
      d->commentLabel->setMinimumSize(QSize(1,1));
      d->commentLabel->hide();    
      d->layout->addRowSpacing(0, -15);
      d->layout->addRowSpacing(2, -15);
   } 
   else
   {
      d->commentLabel->setMinimumSize(d->commentLabel->sizeHint());
      d->commentLabel->show();
      d->layout->addRowSpacing(0, -10);
      d->layout->addRowSpacing(2, 10);
   }
   d->comment = QString::null;

   QFont normal = font();
   QFont bold = normal;
   bold.setBold(true);

   QString num;
   for (int i = 1; i <= 10; ++i) {
      QLabel *label;
      num.setNum(i);
      FieldInfo *score = d->scores.at(i-1);
      label = d->labels[(i-1)*d->nrCols + 0];
      if (i == d->latest) 
         label->setFont(bold);
      else
         label->setFont(normal);

      if (d->fields & Name)
      {
         if (d->newName == i)
         {
           QStackedWidget *stack = d->stack[i-1];
           d->edit = new QLineEdit(d->player, stack);
           d->edit->setMinimumWidth(40);
           stack->addWidget(d->edit);
           stack->setCurrentWidget(d->edit);
           d->edit->setFocus();
           connect(d->edit, SIGNAL(returnPressed()), 
                 this, SLOT(slotGotReturn()));
         }
         else
         {
           label = d->labels[(i-1)*d->nrCols + d->col[Name]];
           if (i == d->latest) 
             label->setFont(bold);
           else
             label->setFont(normal);
           label->setText((*score)[Name]);
         }
         
      }
      for(int field = Name * 2; field < d->fields; field = field * 2)
      {
         if (d->fields & field)
         {
           label = d->labels[(i-1)*d->nrCols + d->col[field]];
           if (i == d->latest) 
             label->setFont(bold);
           else
             label->setFont(normal);
           label->setText((*score)[field]);
         }
      }
   }
   d->latest = -1;
   setFixedSize(minimumSizeHint());
}

void KScoreDialog::loadScores()
{
   QString key, value;
   d->loaded = true;
   d->scores.clear();
   KConfigGroup config(KGlobal::config(), d->configGroup.toUtf8());

   d->player = config.readEntry("LastPlayer");

   QString num;
   for (int i = 1; i <= 10; ++i) {
      num.setNum(i);
      FieldInfo *score = new FieldInfo();
      for(int field = 1; field < d->fields; field = field * 2)
      {
         if (d->fields & field)
         {
            key = "Pos" + num + d->key[field];
            (*score)[field] = config.readEntry(key, QString("-"));
         }
      }
      d->scores.append(score);
   }
}

void KScoreDialog::saveScores()
{
   QString key, value;
   KConfigGroup config(KGlobal::config(), d->configGroup.toUtf8());

   config.writeEntry("LastPlayer", d->player);

   QString num;
   for (int i = 1; i <= 10; ++i) {
      num.setNum(i);
      FieldInfo *score = d->scores.at(i-1);
      for(int field = 1; field < d->fields; field = field * 2)
      {
         if (d->fields & field)
         {
            key = "Pos" + num + d->key[field];
            config.writeEntry(key, (*score)[field]);
         }
      }
   }
   KGlobal::config()->sync();
}

int KScoreDialog::addScore(int newScore, const FieldInfo &newInfo, bool askName)
{
   return addScore(newScore, newInfo, askName, false);
}

int KScoreDialog::addScore(int newScore, const FieldInfo &newInfo, bool askName, bool lessIsMore)
{
   if (!d->loaded)
      loadScores();
   FieldInfo *score = d->scores.first();
   int i = 1;
   for(; score; score = d->scores.next(), i++)
   {
      bool ok;
      int num_score = (*score)[Score].toLong(&ok);
      if (lessIsMore && !ok)
         num_score = 1 << 30;
      if (((newScore > num_score) && !lessIsMore) ||
          ((newScore < num_score) && lessIsMore))
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
        if (i == 1)
          d->comment = i18n("Excellent!\nYou have a new high score!");
        else
          d->comment = i18n("Well done!\nYou made it to the high score list!");
        return i;
      }
   }
   return 0;
}

void KScoreDialog::show()
{
   aboutToShow();
   KDialogBase::show();
}

void KScoreDialog::exec()
{
   aboutToShow();
   KDialogBase::exec();
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
   
   QLabel *label = d->labels[(d->newName-1)*d->nrCols + d->col[Name]];
   label->setFont(bold);
   label->setText(d->player);
   d->stack[(d->newName-1)]->setCurrentWidget(label);
   delete d->edit;
   d->edit = 0;
   d->newName = -1;
}

int KScoreDialog::highScore()
{
   if (!d->loaded)
      loadScores();

   return (*d->scores.first())[Score].toInt();
}

void KScoreDialog::keyPressEvent( QKeyEvent *ev)
{
   if ((d->newName != -1) && (ev->key() == Qt::Key_Return))
   {
       ev->ignore();
       return;
   }
   KDialogBase::keyPressEvent(ev);
}


#include "kscoredialog.moc"
