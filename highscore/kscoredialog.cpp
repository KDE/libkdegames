/****************************************************************
Copyright (c) 1998 Sandro Sigala <ssigala@globalnet.it>.
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

#include "kscoredialog.h"
#include "khighscore.h"

#include <KConfig>
#include <KLocale>
#include <KSeparator>
#include <KGlobal>
#include <KConfigGroup>
#include <KTabWidget>

#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtGui/QGridLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QStackedWidget>

typedef QList<KScoreDialog::FieldInfo*> GroupScores;

class KScoreDialog::KScoreDialogPrivate
{
    public:
        //QList<FieldInfo*> scores;
        QMap<QString, GroupScores> scores;
        KTabWidget *tabWidget;
        //QWidget *page;
        //QGridLayout *layout;
        QLineEdit *edit;    ///<The line edit for entering player name
        QMap<QString, QList<QStackedWidget*> > stack;
        QMap<QString, QList<QLabel*> > labels;
        QLabel *commentLabel;
        QString comment;
        int fields;
        QPair<QString, int> newName; //index of the newname to add
        QPair<QString, int> latest; //index of the latest addition
        int nrCols;
        int numberOfPages;
        bool loaded;
        QString configGroup;
        KHighscore* highscoreObject;
        
        QMap<int, int> col;
        QMap<int, QString> header; ///<Header for fields. Maps field index to a string
        QMap<int, QString> key; ///<Keys for fields. Maps field index to a string
        QString player;
};


KScoreDialog::KScoreDialog(int fields, QWidget *parent)
    : KDialog(parent), d(new KScoreDialogPrivate)
{
    setCaption( i18n("High Scores") );
    setModal( true );
    d->highscoreObject = new KHighscore();
    d->edit = 0;
    d->fields = fields;
    d->newName = QPair<QString,int>(QString(),-1);
    d->latest = QPair<QString,int>(QString(),-1);
    d->loaded = false;
    d->nrCols = 0;
    d->numberOfPages=0;
    d->configGroup=QString();
    
    //Set up the default table headers
    d->header[Name] = i18n("Name");
    d->key[Name] = "Name";
    d->header[Date] = i18n("Date");
    d->key[Date] = "Date";
    d->header[Level] = i18n("Level");
    d->key[Level] = "Level";
    d->header[Score] = i18n("Score");
    d->key[Score] = "Score";
    
    //d->page = new QWidget(this);
    
    d->tabWidget = new KTabWidget(this);
    d->tabWidget->setTabPosition(QTabWidget::West);
    
    setMainWidget(d->tabWidget);
    
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
    
    d->tabWidget->clear();
    foreach(QString groupName, d->scores.keys())
    {
        if(groupName.isEmpty()) //If the group doesn't have a name, use a default.
            d->tabWidget->addTab(new QWidget(this), "High Scores");
        else
            d->tabWidget->addTab(new QWidget(this), groupName);
        d->tabWidget->setCurrentIndex(d->tabWidget->count()-1);
        
        QGridLayout* layout;
        layout = new QGridLayout( d->tabWidget->widget( d->tabWidget->currentIndex() ) );
        //layout->setObjectName("ScoreTab-"+groupName);
        layout->setMargin(marginHint()+20);
        layout->setSpacing(spacingHint());
        layout->addItem(new QSpacerItem(0, 15), 4, 0);
        
        d->commentLabel = new QLabel(d->tabWidget);
        d->commentLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        
        QFont bold = font();
        bold.setBold(true);
        
        QLabel *label;
        layout->addItem(new QSpacerItem(50, 0), 0, 0);
        label = new QLabel(i18n("Rank"), d->tabWidget->widget(d->tabWidget->currentIndex()));
        layout->addWidget(label, 3, 0);
        label->setFont(bold);
        
        for(int field = 1; field < d->fields; field = field * 2)
        {
            if (d->fields & field)
            {
                layout->addItem( new QSpacerItem( 50, 0 ), 0, d->col[field] );
                label = new QLabel(d->header[field], d->tabWidget->widget(d->tabWidget->currentIndex()));
                layout->addWidget(label, 3, d->col[field], field <= Name ? Qt::AlignLeft : Qt::AlignRight);
                label->setFont(bold);
            }
        }
        
        KSeparator *sep = new KSeparator(Qt::Horizontal, d->tabWidget->widget(d->tabWidget->currentIndex()));
        layout->addWidget(sep, 4, 0, 1, d->nrCols);
        
        QString num;
        for (int i = 1; i <= 10; ++i) 
        {
            QLabel *label;
            num.setNum(i);
            label = new QLabel(i18n("#%1", num), d->tabWidget->widget(d->tabWidget->currentIndex()));
            d->labels[groupName].insert((i-1)*d->nrCols + 0, label);
            layout->addWidget(label, i+4, 0);
            if (d->fields & Name) //If we have a Name field
            {
                QStackedWidget *stack = new QStackedWidget(d->tabWidget->widget(d->tabWidget->currentIndex()));
                d->stack[groupName].insert(i-1, stack);
                layout->addWidget(stack, i+4, d->col[Name]);
                label = new QLabel(stack);
                d->labels[groupName].insert((i-1)*d->nrCols + d->col[Name], label);
                stack->addWidget(label);
                stack->setCurrentWidget(label);
            }
            for(int field = Name * 2; field < d->fields; field = field * 2)
            {
                if (d->fields & field)
                {
                    label = new QLabel(d->tabWidget->widget(d->tabWidget->currentIndex()));
                    d->labels[groupName].insert((i-1)*d->nrCols + d->col[field], label);
                    layout->addWidget(label, i+4, d->col[field], Qt::AlignRight);
                }
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
    
    int tabIndex=0;
    int newScoreTabIndex=0;
    foreach(QString groupName, d->scores.keys())
    {
        //Only display the comment on the page with the new score
        if((d->latest.first == d->tabWidget->tabText(tabIndex)) || ( d->latest.first=="" && d->tabWidget->tabText(tabIndex) == "High Scores" ))
        {
            newScoreTabIndex=tabIndex;
            d->commentLabel->setText(d->comment);
            if (d->comment.isEmpty())
            {
                d->commentLabel->setMinimumSize(QSize(1,1));
                d->commentLabel->hide();
                QGridLayout* layout = qobject_cast<QGridLayout*>(d->tabWidget->widget(newScoreTabIndex)->layout());
                layout->addItem( new QSpacerItem( 0, -15 ), 0, 0 );
                layout->addItem( new QSpacerItem( 0, -15 ), 2, 0 );
            }
            else
            {
                QGridLayout* layout = qobject_cast<QGridLayout*>(d->tabWidget->widget(newScoreTabIndex)->layout());
                layout->addWidget(d->commentLabel, 1, 0, 1, d->nrCols);
                d->commentLabel->setMinimumSize(d->commentLabel->sizeHint());
                d->commentLabel->show();
                layout->addItem( new QSpacerItem( 0, -10 ), 0, 0 );
                layout->addItem( new QSpacerItem( 0, 10 ), 2, 0 );
            }
            d->comment.clear(); //TODO move out
        }
        
        QFont normal = font();
        QFont bold = normal;
        bold.setBold(true);
        
        QString num;
        for (int i = 1; i <= 10; ++i)
        {
            QLabel *label;
            num.setNum(i);
            
            //kDebug() << "groupName: " << groupName << " id: " << i-1 << endl;
            
            FieldInfo *score = d->scores[groupName].at(i-1);
            label = d->labels[groupName].at((i-1)*d->nrCols + 0);
            if ( (i == d->latest.second) && (groupName == d->latest.first) )
                label->setFont(bold);
            else
                label->setFont(normal);
    
            if (d->fields & Name)
            {
                if ( (d->newName.second == i) && (groupName == d->newName.first) )
                {
                    QStackedWidget *stack = d->stack[groupName].at(i-1);
                    d->edit = new QLineEdit(d->player, stack);
                    d->edit->setMinimumWidth(40);
                    stack->addWidget(d->edit);
                    stack->setCurrentWidget(d->edit);
                    d->edit->setFocus();
                    connect(d->edit, SIGNAL(returnPressed()), this, SLOT(slotGotReturn()));
                }
                else
                {
                    label = d->labels[groupName].at((i-1)*d->nrCols + d->col[Name]);
                    if ( (i == d->latest.second) && (groupName == d->latest.first) )
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
                    label = d->labels[groupName].at((i-1)*d->nrCols + d->col[field]);
                    if ( (i == d->latest.second) && (groupName == d->latest.first) )
                        label->setFont(bold);
                    else
                        label->setFont(normal);
                    label->setText((*score)[field]);
                }
            }
        }
        tabIndex++;
    }
    d->latest = QPair<QString,int>(QString(),-1);
    setFixedSize(minimumSizeHint());
    d->tabWidget->setCurrentIndex(newScoreTabIndex);
}

void KScoreDialog::loadScores()
{
    QString key;
    QString value;
    
    d->scores.clear();
    
    QStringList groupList = d->highscoreObject->groupList(); //List of the group names
    d->numberOfPages = groupList.size();
    
    QString tempCurrentGroup = d->configGroup; //temp to store the user-set group name
    
    if (groupList.count(d->configGroup) == 0) //If the current group doesn't have any entries, add it to the list to process
    {
        kDebug() << "The current high score group \"" << d->configGroup << "\" isn't in the list, adding it" << endl;
        groupList << d->configGroup;
    }
    
    foreach(QString groupName, groupList)
    {
        d->highscoreObject->setHighscoreGroup(groupName);
        d->player = d->highscoreObject->readEntry(0, "LastPlayer");  //FIXME
        
        for (int i = 1; i <= 10; ++i)
        {
            FieldInfo *score = new FieldInfo();
            for(int field = 1; field < d->fields; field = field * 2)
            {
                if (d->fields & field)
                {
                    (*score)[field] = d->highscoreObject->readEntry(i, d->key[field], QString("-"));
                }
            }
            d->scores[groupName].append(score);
        }
    }
    d->highscoreObject->setHighscoreGroup(tempCurrentGroup); //reset to the user-set group name
    d->loaded = true;
}

void KScoreDialog::saveScores()
{
    QString key, value;
    d->highscoreObject->setHighscoreGroup(d->configGroup.toUtf8());
    
    d->highscoreObject->writeEntry(0,"LastPlayer", d->player);
    
    for (int i = 1; i <= 10; ++i)
    {
        FieldInfo *score = d->scores[d->configGroup].at(i-1);
        for(int field = 1; field < d->fields; field = field * 2)
        {
            if (d->fields & field)
            {
                d->highscoreObject->writeEntry(i, d->key[field], (*score)[field]);
            }
        }
    }
    d->highscoreObject->writeAndUnlock();
}

int KScoreDialog::addScore(int newScore, const FieldInfo &newInfo, const AddScoreFlags& flags)
{
    bool askName=false, lessIsMore=false;
    if((flags & KScoreDialog::AskName) == KScoreDialog::AskName)
        askName = true;
    if((flags & KScoreDialog::LessIsMore) == KScoreDialog::LessIsMore)
        lessIsMore = true;
    
    if (!d->loaded)
        loadScores();
    FieldInfo *score;
    for(int i=0; i<d->scores[d->configGroup].size(); i++)
    {
        score = d->scores[d->configGroup].at(i);
        bool ok;
        int num_score = (*score)[Score].toLong(&ok);
        if (lessIsMore && !ok)
            num_score = 1 << 30;
        if (((newScore > num_score) && !lessIsMore) ||
	    ((newScore < num_score) && lessIsMore))
        {
            score = new FieldInfo(newInfo);
            (*score)[Score].setNum(newScore);
            d->latest = QPair<QString,int>(d->configGroup,i+1);
            d->scores[d->configGroup].insert(i, score);
            d->scores[d->configGroup].removeAt(10);
            
            if((*score)[Name].isEmpty()) //If we don't have a name, prompt the player.
                askName = true;
            
            if (askName)
            {
                d->player=(*score)[Name];
                d->newName = QPair<QString,int>(d->configGroup,i+1);
            }
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
    if (d->newName.second == -1) return;

    d->player = d->edit->text();
    
    (*d->scores[d->newName.first].at(d->newName.second-1))[Name] = d->player;
    saveScores();
    
    QFont bold = font();
    bold.setBold(true);
    
    QLabel *label = d->labels[d->newName.first].at((d->newName.second-1)*d->nrCols + d->col[Name]);
    label->setFont(bold);
    label->setText(d->player);
    d->stack[d->newName.first].at((d->newName.second-1))->setCurrentWidget(label);
    delete d->edit;
    d->edit = 0;
    d->newName = QPair<QString,int>(QString(),-1);
}

int KScoreDialog::highScore()
{
    if (!d->loaded)
        loadScores();
    
    return (*d->scores[d->configGroup].first())[Score].toInt();
}

void KScoreDialog::keyPressEvent(QKeyEvent *ev)
{
    if ((d->newName.second != -1) && (ev->key() == Qt::Key_Return))
    {
        ev->ignore();
        return;
    }
    KDialog::keyPressEvent(ev);
}

#include "kscoredialog.moc"
