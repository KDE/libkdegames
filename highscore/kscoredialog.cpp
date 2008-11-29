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
#include <KUser>
#include <KLocale>
#include <KSeparator>
#include <KGlobal>
#include <KConfigGroup>
#include <KTabWidget>
#include <KDebug>

#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtGui/QGridLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QStackedWidget>

#define DEFAULT_GROUP_NAME I18N_NOOP("High Scores")

typedef QList<KScoreDialog::FieldInfo> GroupScores; ///<The list of scores in a group

class KScoreDialog::KScoreDialogPrivate
{
    public:
        //QList<FieldInfo*> scores;
        QMap<QByteArray, GroupScores> scores; ///<Maps config group name to GroupScores
        KTabWidget *tabWidget;
        //QWidget *page;
        //QGridLayout *layout;
        QLineEdit *edit;    ///<The line edit for entering player name
        QMap<QByteArray, QList<QStackedWidget*> > stack;
        QMap<QByteArray, QList<QLabel*> > labels; ///<For each group, the labels along each row in turn starting from "#1"
        QLabel *commentLabel;
        QString comment;
        int fields;
        int hiddenFields;
        QPair<QByteArray, int> newName; //index of the newname to add (groupKey, position)
        QPair<QByteArray, int> latest; //index of the latest addition (groupKey, position)
        int nrCols;
        int numberOfPages;
        bool loaded;
        QByteArray configGroup;
        KHighscore* highscoreObject;
        QMap<QByteArray, QString> translatedGroupNames; ///<List of the translated group names.

        QMap<int, int> col;
        QMap<int, QString> header; ///<Header for fields. Maps field index to a string
        QMap<int, QString> key; ///<Keys for fields. Maps field index to a string
        QString player;

        //Q-Pointer
        KScoreDialogPrivate(KScoreDialog* parent):q(parent){}
        KScoreDialog* const q;

        //Functions
        void loadScores();
        void saveScores();

        void setupDialog();
        void setupGroup(const QByteArray& groupName);
        void aboutToShow();

        QString findTranslatedGroupName(const QByteArray& name);
};


KScoreDialog::KScoreDialog(int fields, QWidget *parent)
    : KDialog(parent), d(new KScoreDialogPrivate(this))
{
    setCaption( i18n(DEFAULT_GROUP_NAME) );
    setModal( true );
    d->highscoreObject = new KHighscore();
    d->edit = 0;
    fields |= Score; //Make 'Score' field automatic (it can be hidden if necessary)
    d->fields = fields;
    d->hiddenFields = 0;
    d->newName = QPair<QByteArray,int>(QByteArray(),-1);
    d->latest = QPair<QByteArray,int>("Null",-1);
    d->loaded = false;
    d->nrCols = 0;
    d->numberOfPages=0;
    d->configGroup=QByteArray();

    //Set up the default table headers
    d->header[Name] = i18n("Name");
    d->key[Name] = "Name";
    d->header[Date] = i18n("Date");
    d->key[Date] = "Date";
    d->header[Level] = i18n("Level");
    d->key[Level] = "Level";
    d->header[Score] = i18n("Score");
    d->key[Score] = "Score";
    d->header[Time] = i18n("Time");
    d->key[Time] = "Time";

    //d->page = new QWidget(this);

    d->tabWidget = new KTabWidget(this);
    d->tabWidget->setTabPosition(QTabWidget::West);

    setMainWidget(d->tabWidget);
    if(d->newName.second == -1)
      setButtons(Close);
    else
    {
       setButtons(Ok|Cancel);
       connect(this, SIGNAL(okClicked()), SLOT(slotGotName()));
    }
}

KScoreDialog::~KScoreDialog()
{
    delete d->highscoreObject;

    delete d;
}

void KScoreDialog::setConfigGroup(const QString &group)  //DEPRECATED!
{
    d->configGroup = group.toUtf8();
    d->loaded = false;
}

void KScoreDialog::setConfigGroup(const QPair<QByteArray, QString>& group)
{
    d->configGroup = group.first; //untranslated string
    addLocalizedConfigGroupName(group); //add the translation to the list
    d->loaded = false;
}

void KScoreDialog::addLocalizedConfigGroupName(const QPair<QByteArray, QString>& group)
{
    bool alreadyPresent = false;
    foreach(const QByteArray& groupKey, d->translatedGroupNames.keys())
    {
        if(groupKey == group.first)
        {
            alreadyPresent = true;
            break;
        }
    }
    if(!alreadyPresent)
    {
        d->translatedGroupNames.insert(group.first, group.second);
        kDebug() << "adding" << group.first << "->" << group.second;
    }
}

void KScoreDialog::addLocalizedConfigGroupNames(const QMap<QByteArray, QString>& groups)
{
    foreach(const QByteArray& groupKey, groups.keys())
    {
        addLocalizedConfigGroupName(qMakePair(groupKey, groups.value(groupKey)));
    }
}

QString KScoreDialog::KScoreDialogPrivate::findTranslatedGroupName(const QByteArray& name)
{
    foreach(const QByteArray& groupKey, translatedGroupNames.keys())
    {
        if(name == groupKey) //if this string is the same as the untranslated version in the list
        {
            return translatedGroupNames.value(groupKey); //return the translated string for the current locale
        }
    }
    //If it wasn't found then just try i18n( to see if it happens to be in the database
    return i18n(name); //FIXME?
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

void KScoreDialog::hideField(int field)
{
    d->hiddenFields |= field;
}

/*
Create the widgets and layouts etc. for the dialog
*/

void KScoreDialog::KScoreDialogPrivate::setupDialog()
{
    nrCols = 1;
    for(int field = 1; field < fields; field = field * 2)
    {
        if ( (fields & field) && !(hiddenFields & field ) )
            col[field] = nrCols++;
    }

    tabWidget->clear();
    foreach(const QByteArray &groupName, scores.keys())
        setupGroup(groupName);
}

void KScoreDialog::KScoreDialogPrivate::setupGroup(const QByteArray& groupKey)
{
    if(groupKey.isEmpty()) //If the group doesn't have a name, use a default.
            tabWidget->addTab(new QWidget(q), i18n(DEFAULT_GROUP_NAME));
        else
            tabWidget->addTab(new QWidget(q), findTranslatedGroupName(groupKey));
        tabWidget->setCurrentIndex(tabWidget->count()-1);

        QGridLayout* layout = new QGridLayout( tabWidget->widget( tabWidget->currentIndex() ) );
        //layout->setObjectName("ScoreTab-"+groupName);
        layout->setMargin(marginHint()+20);
        layout->setSpacing(spacingHint());
        layout->addItem(new QSpacerItem(0, 15), 4, 0);

        commentLabel = new QLabel(tabWidget);
        commentLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

        QFont bold = q->font();
        bold.setBold(true);

        QLabel *label;
        layout->addItem(new QSpacerItem(50, 0), 0, 0);
        label = new QLabel(i18n("Rank"), tabWidget->widget(tabWidget->currentIndex()));
        layout->addWidget(label, 3, 0);
        label->setFont(bold);

        for(int field = 1; field < fields; field = field * 2)
        {
            if ( (fields & field) && !(hiddenFields & field ) ) //If it's used and not hidden
            {
                layout->addItem( new QSpacerItem( 50, 0 ), 0, col[field] );
                label = new QLabel(header[field], tabWidget->widget(tabWidget->currentIndex()));
                layout->addWidget(label, 3, col[field], field <= Name ? Qt::AlignLeft : Qt::AlignRight);
                label->setFont(bold);
            }
        }

        KSeparator *sep = new KSeparator(Qt::Horizontal, tabWidget->widget(tabWidget->currentIndex()));
        layout->addWidget(sep, 4, 0, 1, nrCols);

        QString num;
        for (int i = 1; i <= 10; ++i)
        {
            QLabel *label;
            num.setNum(i);
            label = new QLabel(i18n("#%1", num), tabWidget->widget(tabWidget->currentIndex()));
            labels[groupKey].insert((i-1)*nrCols + 0, label); //Fill up column zero
            layout->addWidget(label, i+4, 0);
            if (fields & Name) //If we have a Name field
            {
                QStackedWidget *localStack = new QStackedWidget(tabWidget->widget(tabWidget->currentIndex()));
                stack[groupKey].insert(i-1, localStack);
                layout->addWidget(localStack, i+4, col[Name]);
                label = new QLabel(localStack);
                labels[groupKey].insert((i-1)*nrCols + col[Name], label);
                localStack->addWidget(label);
                localStack->setCurrentWidget(label);
            }
            for(int field = Name * 2; field < fields; field = field * 2)
            {
                if ( (fields & field) && !(hiddenFields & field ) ) //Maybe disable for Name?
                {
                    label = new QLabel(tabWidget->widget(tabWidget->currentIndex()));
                    labels[groupKey].insert((i-1)*nrCols + col[field], label);
                    layout->addWidget(label, i+4, col[field], Qt::AlignRight);
                }
            }
        }
}

/*
Fill the dialog with the correct data
*/

void KScoreDialog::KScoreDialogPrivate::aboutToShow()
{
    if (!loaded)
        loadScores();

    if (!nrCols)
        setupDialog();

    int tabIndex=0; //Index of the current tab
    int newScoreTabIndex=0; //The index of the tab of the group with the new score
    foreach(const QByteArray &groupKey, scores.keys())
    {
        //Only display the comment on the page with the new score (or) this one if there's only one tab
        if((latest.first == tabWidget->tabText(tabIndex)) || ( latest.first.isEmpty() && tabWidget->tabText(tabIndex) == i18n(DEFAULT_GROUP_NAME) ))
        {
            newScoreTabIndex=tabIndex;
            commentLabel->setText(comment);
            if (comment.isEmpty())
            {
                commentLabel->setMinimumSize(QSize(1,1));
                commentLabel->hide();
                QGridLayout* layout = qobject_cast<QGridLayout*>(tabWidget->widget(newScoreTabIndex)->layout());
                layout->addItem( new QSpacerItem( 0, -15 ), 0, 0 );
                layout->addItem( new QSpacerItem( 0, -15 ), 2, 0 );
            }
            else
            {
                QGridLayout* layout = qobject_cast<QGridLayout*>(tabWidget->widget(newScoreTabIndex)->layout());
                layout->addWidget(commentLabel, 1, 0, 1, nrCols);
                commentLabel->setMinimumSize(commentLabel->sizeHint());
                commentLabel->show();
                layout->addItem( new QSpacerItem( 0, -10 ), 0, 0 );
                layout->addItem( new QSpacerItem( 0, 10 ), 2, 0 );
            }
            comment.clear();
        }

        QFont normal = q->font();
        QFont bold = normal;
        bold.setBold(true);

        QString num;
        for (int i = 1; i <= 10; ++i)
        {
            QLabel *label;
            num.setNum(i);

            //kDebug() << "groupName:" << groupName << "id:" << i-1;

            FieldInfo score = scores[groupKey].at(i-1);
            label = labels[groupKey].at((i-1)*nrCols + 0); //crash! FIXME
            if ( (i == latest.second) && (groupKey == latest.first) )
                label->setFont(bold);
            else
                label->setFont(normal);

            if (fields & Name)
            {
                if ( (newName.second == i) && (groupKey == newName.first) )
                {
                    QStackedWidget *localStack = stack[groupKey].at(i-1);
                    edit = new QLineEdit(player, localStack);
                    edit->setMinimumWidth(40);
                    localStack->addWidget(edit);
                    localStack->setCurrentWidget(edit);
                    edit->setFocus();
                    connect(edit, SIGNAL(returnPressed()), q, SLOT(slotGotReturn()));
                }
                else
                {
                    label = labels[groupKey].at((i-1)*nrCols + col[Name]);
                    if ( (i == latest.second) && (groupKey == latest.first) )
                        label->setFont(bold);
                    else
                        label->setFont(normal);
                    label->setText(score[Name]);
                }

            }
            for(int field = Name * 2; field < fields; field = field * 2)
            {
                if ( (fields & field) && !(hiddenFields & field ) )
                {
                    label = labels[groupKey].at((i-1)*nrCols + col[field]);
                    if ( (i == latest.second) && (groupKey == latest.first) )
                        label->setFont(bold);
                    else
                        label->setFont(normal);
                    label->setText(score[field]);
                }
            }
        }
        tabIndex++;
    }
    latest = QPair<QByteArray,int>(QByteArray(),-1);
    q->setFixedSize(q->minimumSizeHint()); //NOTE Remove this line to make dialog resizable
    tabWidget->setCurrentIndex(newScoreTabIndex);
}

void KScoreDialog::KScoreDialogPrivate::loadScores()
{
    scores.clear();

    QStringList groupList = highscoreObject->groupList(); //List of the group names actually in the config file
    numberOfPages = groupList.size(); //how many groups are in the config fileo

    QList<QByteArray> groupKeyList; //This will be a list of all the groups in the config fie
    for(int i = 0; i < numberOfPages; i++)
    {
        groupKeyList << groupList.at(i).toUtf8(); //Convert all the QStrings to QByteArrays
    }

    QByteArray tempCurrentGroup = configGroup; //temp to store the user-set group name

    if (groupKeyList.count(configGroup) == 0) //If the current group doesn't have any entries, add it to the list to process
    {
        kDebug(11002) << "The current high score group " << configGroup << " isn't in the list, adding it";
        groupKeyList << configGroup;
        setupGroup(configGroup);
    }

    foreach(const QByteArray &groupKey, groupKeyList)
    {
        highscoreObject->setHighscoreGroup(groupKey);
        player = highscoreObject->readEntry(0, "LastPlayer");  //FIXME

        for (int i = 1; i <= 10; ++i)
        {
            FieldInfo score;
            for(int field = 1; field < fields; field = field * 2)
            {
                if (fields & field)
                {
                    score[field] = highscoreObject->readEntry(i, key[field], QString("-"));
                }
            }
            scores[groupKey].append(score);
        }
    }
    highscoreObject->setHighscoreGroup(tempCurrentGroup); //reset to the user-set group name
    foreach(const QByteArray &groupKey, scores.keys())
    {
        if( (scores[groupKey][0].value(Score)=="-") && (scores.size() > 1) && (latest.first != groupKey) )
        {
            kDebug(11002) << "Removing group " << groupKey << " since it's unused.";
            scores.remove(groupKey);
        }
    }
    loaded = true;
}

void KScoreDialog::KScoreDialogPrivate::saveScores()
{
    highscoreObject->setHighscoreGroup(configGroup);

    highscoreObject->writeEntry(0,"LastPlayer", player);

    for (int i = 1; i <= 10; ++i)
    {
        FieldInfo score = scores[configGroup].at(i-1);
        for(int field = 1; field < fields; field = field * 2)
        {
            if (fields & field)
            {
                highscoreObject->writeEntry(i, key[field], score[field]);
            }
        }
    }
    highscoreObject->writeAndUnlock();
}

int KScoreDialog::addScore(const FieldInfo& newInfo, const AddScoreFlags& flags)
{
    kDebug() << "adding new score";

    bool askName=false, lessIsMore=false;
    if(flags.testFlag(KScoreDialog::AskName))
        askName = true;
    if(flags.testFlag(KScoreDialog::LessIsMore))
        lessIsMore = true;

    d->latest.first = d->configGroup; //Temporarily set this so loadScores() knows not to delete this group
    if (!d->loaded)
        d->loadScores();
    d->latest.first = "Null"; //and reset it.

    for(int i=0; i<d->scores[d->configGroup].size(); i++)
    {
        kDebug() << "in loop 1";
        FieldInfo score = d->scores[d->configGroup].at(i); //First look at the score in the config file
        bool ok; //will be false if there isn't any score yet in position i
        int num_score = score[Score].toLong(&ok); //test if the stored score is a number

        score = FieldInfo(newInfo); //now look at the submitted score
        int newScore = score[Score].toInt();

        kDebug() << "num_score =" << num_score << " - newScore =" << newScore;

        if (((newScore > num_score) && !lessIsMore) ||
              ((newScore < num_score) && lessIsMore) || !ok)
        {
            kDebug() << "in if() 1";
            d->latest = QPair<QByteArray,int>(d->configGroup,i+1);
            d->scores[d->configGroup].insert(i, score);
            d->scores[d->configGroup].removeAt(10);

            if(score[Name].isEmpty()) //If we don't have a name, prompt the player.
            {
                if(!d->player.isEmpty()) //d->player should be filled out by d->loadScores()
                {
                    score[Name] = d->player;
                }
                else
                {
                    KUser user;
                    score[Name] = user.property(KUser::FullName).toString();
                    if (score[Name].isEmpty())
                    {
                        score[Name] = user.loginName();
                    }
                }
                askName = true;
            }

            if (askName)
            {
                d->player=score[Name];
                d->newName = QPair<QByteArray,int>(d->configGroup,i+1);

                setButtons(Ok|Cancel);
                connect(this, SIGNAL(okClicked()), SLOT(slotGotName()));
            }
            else
                d->saveScores();

            if (i == 0)
                d->comment = i18n("Excellent!\nYou have a new high score!");
            else
                d->comment = i18n("Well done!\nYou made it to the high score list!");
            return i+1;
        }
    }
    d->latest = qMakePair(d->configGroup, 0);
    return 0;
}

int KScoreDialog::addScore(int newScore, const AddScoreFlags& flags)
{
    FieldInfo scoreInfo;
    scoreInfo[Score]=QString::number(newScore);
    return addScore(scoreInfo, AskName | flags);
}

void KScoreDialog::show()
{
    d->aboutToShow();
    KDialog::show();
}

void KScoreDialog::exec()
{
    d->aboutToShow();
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

    d->scores[d->newName.first][d->newName.second-1][Name] = d->player;
    d->saveScores();

    QFont bold = font();
    bold.setBold(true);

    QLabel *label = d->labels[d->newName.first].at((d->newName.second-1)*d->nrCols + d->col[Name]);
    label->setFont(bold);
    label->setText(d->player);
    d->stack[d->newName.first].at((d->newName.second-1))->setCurrentWidget(label);
    delete d->edit;
    d->edit = 0;
    d->newName = QPair<QByteArray,int>(QByteArray(),-1);
}

int KScoreDialog::highScore()
{
    if (!d->loaded)
        d->loadScores();

    if (!d->scores[d->configGroup].isEmpty())
        return d->scores[d->configGroup].first()[Score].toInt();
    else
        return 0;
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
