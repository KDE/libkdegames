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

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTimer>
#include <QGridLayout>
#include <QKeyEvent>
#include <QList>

#include <kconfig.h>
#include <klocale.h>
#include <kseparator.h>
#include <kglobal.h>

#include "kscoredialog.h"

class KScoreDialog::KScoreDialogPrivate
{
public:
   QList<FieldInfo*> scores;
   QWidget *page;
   QGridLayout *layout;
   QLineEdit *edit;
   QList<QStackedWidget*> stack;
   QList<QLabel*> labels;
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


KScoreDialog::KScoreDialog(int fields, QWidget *parent)
        : KDialog(parent)
{
    setCaption( i18n("High Scores") );
    setModal( true );
   d = new KScoreDialogPrivate();
   d->edit = 0;
   d->fields = fields;
   d->newName = -1;
   d->latest = -1;
   d->loaded = false;
   d->nrCols = 0;
   d->configGroup = "High Score";

   d->header[Name] = i18n("Name");
   d->key[Name] = "Name";

   d->header[Date] = i18n("Date");
   d->key[Date] = "Date";

   d->header[Level] = i18n("Level");
   d->key[Level] = "Level";

   d->header[Score] = i18n("Score");
   d->key[Score] = "Score";
   d->page = new QWidget( this );
   setMainWidget(d->page);

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

   d->layout = new QGridLayout(d->page);
   d->layout->setMargin(marginHint()+20);
   d->layout->setSpacing(spacingHint());
   d->layout->addItem(new QSpacerItem(0, 15), 4, 0);

   d->commentLabel = new QLabel(d->page);
   d->commentLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
   d->layout->addWidget(d->commentLabel, 1, 0, 1, d->nrCols);

   QFont bold = font();
   bold.setBold(true);

   QLabel *label;
   d->layout->addItem(new QSpacerItem(50, 0), 0, 0);
   label = new QLabel(i18n("Rank"), d->page);
   d->layout->addWidget(label, 3, 0);
   label->setFont(bold);

   for(int field = 1; field < d->fields; field = field * 2)
   {
      if (d->fields & field)
      {
         d->layout->addItem( new QSpacerItem( 50, 0 ), 0, d->col[field] );
         label = new QLabel(d->header[field], d->page);
         d->layout->addWidget(label, 3, d->col[field], field <= Name ? Qt::AlignLeft : Qt::AlignRight);
         label->setFont(bold);
      }
   }

   KSeparator *sep = new KSeparator(Qt::Horizontal, d->page);
   d->layout->addWidget(sep, 4, 0, 1, d->nrCols);

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
         label = new QLabel(stack);
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
      d->layout->addItem( new QSpacerItem( 0, -15 ), 0, 0 );
      d->layout->addItem( new QSpacerItem( 0, -15 ), 2, 0 );
   }
   else
   {
      d->commentLabel->setMinimumSize(d->commentLabel->sizeHint());
      d->commentLabel->show();
      d->layout->addItem( new QSpacerItem( 0, -10 ), 0, 0 );
      d->layout->addItem( new QSpacerItem( 0, 10 ), 2, 0 );
   }
   d->comment.clear();

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
   qDeleteAll( d->scores );
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
   FieldInfo *score;
   for(int i=0; i<d->scores.size(); i++)
   {
      score = d->scores.at(i);
      bool ok;
      int num_score = (*score)[Score].toLong(&ok);
      if (lessIsMore && !ok)
         num_score = 1 << 30;
      if (((newScore > num_score) && !lessIsMore) ||
          ((newScore < num_score) && lessIsMore))
      {
        score = new FieldInfo(newInfo);
        (*score)[Score].setNum(newScore);
        d->scores.insert(i, score);
        d->scores.removeAt(10);
        d->latest = i+1;
        if (askName)
          d->newName = i+1;
        else
          saveScores();
        if (i == 0)
          d->comment = i18n("Excellent!\nYou have a new high score!");
        else
          d->comment = i18n("Well done!\nYou made it to the high score list!");
        return i+1;
      }
   }
   return 0;
}

void KScoreDialog::show()
{
   aboutToShow();
   KDialog::show();
}

void KScoreDialog::exec()
{
   aboutToShow();
   KDialog::exec();
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
   KDialog::keyPressEvent(ev);
}


#include "kscoredialog.moc"
