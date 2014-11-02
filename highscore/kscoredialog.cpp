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
#include "../kgdifficulty.h"

#include <KConfig>
#include <KUser>
#include <KSeparator>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KLineEdit>

#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QGridLayout>
#include <QtGui/QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QStackedWidget>
#include <QTabWidget>
#include <QApplication>
#include <QStyle>
#include <QPushButton>
#include <QLoggingCategory>

#define DEFAULT_GROUP_NAME I18N_NOOP("High Scores")

typedef QList<KScoreDialog::FieldInfo> GroupScores; ///<The list of scores in a group

class KScoreDialog::KScoreDialogPrivate
{
    public:
        //QList<FieldInfo*> scores;
        QMap<QByteArray, GroupScores> scores; ///<Maps config group name to GroupScores
        QList<QByteArray> hiddenGroups; /// Groups that should not be shown in the dialog
        QMap<int, QByteArray> configGroupWeights; /// Weights of the groups, defines ordering
        QTabWidget *tabWidget;
        //QWidget *page;
        //QGridLayout *layout;
        KLineEdit *edit;    ///<The line edit for entering player name
        QMap<QByteArray, QList<QStackedWidget*> > stack;
        QMap<QByteArray, QList<QLabel*> > labels; ///<For each group, the labels along each row in turn starting from "#1"
        QLabel *commentLabel;
        QString comment;
        int fields;
        int hiddenFields;
        QPair<QByteArray, int> newName; //index of the new name to add (groupKey, position)
        QPair<QByteArray, int> latest; //index of the latest addition (groupKey, position)
        int nrCols;
        bool loaded;
        QByteArray configGroup;
        KHighscore* highscoreObject;
        QMap<QByteArray, QString> translatedGroupNames; ///<List of the translated group names.
        QMap<QByteArray, QWidget*> tabs;

        QMap<int, int> col;
        QMap<int, QString> header; ///<Header for fields. Maps field index to a string
        QMap<int, QString> key; ///<Keys for fields. Maps field index to a string
        QString player;
        int lastHighPosition; /// remember the position to delete if the user wants to forget

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
    : QDialog(parent), d(new KScoreDialogPrivate(this))
{
    QLoggingCategory::setFilterRules(QLatin1Literal("games.highscore.debug = true"));
    
    setWindowTitle( i18n(DEFAULT_GROUP_NAME) );
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
    d->configGroup=QByteArray();

    //Set up the default table headers
    d->header[Name] = i18n("Name");
    d->key[Name] = QLatin1String( "Name" );
    d->header[Date] = i18n("Date");
    d->key[Date] = QLatin1String( "Date" );
    d->header[Level] = i18n("Level");
    d->key[Level] = QLatin1String( "Level" );
    d->header[Score] = i18n("Score");
    d->key[Score] = QLatin1String( "Score" );
    d->header[Time] = i18n("Time");
    d->key[Time] = QLatin1String( "Time" );

    //d->page = new QWidget(this);

    d->tabWidget = new QTabWidget(this);
    d->tabWidget->setTabPosition(QTabWidget::West);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(d->tabWidget);
    
    buttonBox = new QDialogButtonBox(this);
    
    if(d->newName.second == -1)
    {
      buttonBox->setStandardButtons(QDialogButtonBox::Close);
      connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    }
    else
    {
       buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
       connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
       connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
       connect(buttonBox, SIGNAL(accepted()), SLOT(slotGotName()));
    }
    
    mainLayout->addWidget(buttonBox);
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
    if (!d->translatedGroupNames.contains(group.first))
    {
        d->translatedGroupNames.insert(group.first, group.second);
        qCDebug(GAMES_HIGHSCORE) << "adding" << group.first << "->" << group.second;
    }
}

void KScoreDialog::addLocalizedConfigGroupNames(const QMap<QByteArray, QString>& groups)
{
    QMap<QByteArray, QString>::const_iterator it = groups.begin();
    for (; it != groups.end(); ++it)
    {
        addLocalizedConfigGroupName(qMakePair(it.key(), it.value()));
    }
}

void KScoreDialog::initFromDifficulty(const KgDifficulty* diff, bool doSetConfigGroup)
{
    QMap<QByteArray, QString> localizedLevelStrings;
    QMap<int, QByteArray> levelWeights;
    foreach (const KgDifficultyLevel* level, diff->levels())
    {
        localizedLevelStrings.insert(level->key(), level->title());
        levelWeights.insert(level->hardness(), level->key());
    }
    addLocalizedConfigGroupNames(localizedLevelStrings);
    setConfigGroupWeights(levelWeights);
    if (doSetConfigGroup)
    {
        const KgDifficultyLevel* curLvl = diff->currentLevel();
        setConfigGroup(qMakePair(curLvl->key(), curLvl->title()));
    }
}

void KScoreDialog::setHiddenConfigGroups(const QList<QByteArray>& hiddenGroups)
{
    d->hiddenGroups = hiddenGroups;
}

void KScoreDialog::setConfigGroupWeights(const QMap<int, QByteArray>& weights)
{
    d->configGroupWeights = weights;
}

QString KScoreDialog::KScoreDialogPrivate::findTranslatedGroupName(const QByteArray& name)
{
    const QString lookupResult = translatedGroupNames.value(name);
    //If it wasn't found then just try i18n( to see if it happens to be in the database
    return lookupResult.isEmpty() ? i18n(name) : lookupResult; //FIXME?
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
    QList<QByteArray> keysToConfigure = scores.keys();
    foreach(const QByteArray &groupName, configGroupWeights)
    {
        int index = keysToConfigure.indexOf(groupName);
        if (index != -1)
        {
            setupGroup(groupName);
            keysToConfigure.removeAt(index);
        }
    }
    foreach(const QByteArray &groupName, keysToConfigure)
    {
        setupGroup(groupName);
    }
}

void KScoreDialog::KScoreDialogPrivate::setupGroup(const QByteArray& groupKey)
{
    if (hiddenGroups.contains(groupKey))
        return;
    QWidget* widget = new QWidget(q);
    tabs[groupKey] = widget;

    QString tabName = groupKey.isEmpty() ? i18n(DEFAULT_GROUP_NAME) : findTranslatedGroupName(groupKey);
    tabWidget->addTab(widget, tabName);

    QGridLayout* layout = new QGridLayout(widget);
    //layout->setObjectName( QLatin1String("ScoreTab-" )+groupName);
    //layout->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin)+20);
    //layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    layout->addItem(new QSpacerItem(0, 15), 4, 0);

    commentLabel = new QLabel(tabWidget);
    commentLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

    QFont bold = q->font();
    bold.setBold(true);

    QLabel *label;
    layout->addItem(new QSpacerItem(50, 0), 0, 0);
    label = new QLabel(i18n("Rank"), widget);
    layout->addWidget(label, 3, 0);
    label->setFont(bold);

    for(int field = 1; field < fields; field = field * 2)
    {
        if ( (fields & field) && !(hiddenFields & field ) ) //If it's used and not hidden
        {
            layout->addItem( new QSpacerItem( 50, 0 ), 0, col[field] );
            label = new QLabel(header[field], widget);
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
        label = new QLabel(i18nc("Enumeration (#1, #2 ...) of the highscore entries", "#%1", num), widget);
        labels[groupKey].insert((i-1)*nrCols + 0, label); //Fill up column zero
        layout->addWidget(label, i+4, 0);
        if (fields & Name) //If we have a Name field
        {
            QStackedWidget *localStack = new QStackedWidget(widget);
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
                label = new QLabel(widget);
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

    QMap<QByteArray, GroupScores>::const_iterator it = scores.constBegin();
    for (; it != scores.constEnd(); ++it)
    {
        const QByteArray &groupKey = it.key();
        if (hiddenGroups.contains(groupKey))
            continue;
        qCDebug(GAMES_HIGHSCORE) << latest.first << tabWidget->tabText(tabIndex);

        //Only display the comment on the page with the new score (or) this one if there's only one tab
        if(latest.first == groupKey || ( latest.first.isEmpty() && groupKey == DEFAULT_GROUP_NAME ) )
        {
            QWidget* widget = tabs.value(groupKey);
            QGridLayout* layout = qobject_cast<QGridLayout*>(widget->layout());

            commentLabel->setText(comment);
            if (comment.isEmpty())
            {
                commentLabel->setMinimumSize(QSize(1,1));
                commentLabel->hide();
                layout->addItem( new QSpacerItem( 0, -15 ), 0, 0 );
                layout->addItem( new QSpacerItem( 0, -15 ), 2, 0 );
            }
            else
            {
                layout->addWidget(commentLabel, 1, 0, 1, nrCols);
                commentLabel->setMinimumSize(commentLabel->sizeHint());
                commentLabel->show();
                layout->addItem( new QSpacerItem( 0, -10 ), 0, 0 );
                layout->addItem( new QSpacerItem( 0, 10 ), 2, 0 );
            }
            comment.clear();

            tabWidget->setCurrentWidget(widget);
        }

        QFont normal = q->font();
        QFont bold = normal;
        bold.setBold(true);

        QString num;
        for (int i = 1; i <= 10; ++i)
        {
            QLabel *label;
            num.setNum(i);

            //qCDebug(GAMES_HIGHSCORE) << "groupName:" << groupName << "id:" << i-1;

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
                    edit = new KLineEdit(player, localStack);
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
}

void KScoreDialog::KScoreDialogPrivate::loadScores()
{
    scores.clear();

    QList<QByteArray> groupKeyList; //This will be a list of all the groups in the config fie
    foreach( const QString & groupString, highscoreObject->groupList())
    {
        groupKeyList << groupString.toUtf8(); //Convert all the QStrings to QByteArrays
    }

    QByteArray tempCurrentGroup = configGroup; //temp to store the user-set group name

    if (!groupKeyList.contains( configGroup) ) //If the current group doesn't have any entries, add it to the list to process
    {
        qCDebug(GAMES_HIGHSCORE) << "The current high score group " << configGroup << " isn't in the list, adding it";
        groupKeyList << configGroup;
        setupGroup(configGroup);
    }

    foreach(const QByteArray &groupKey, groupKeyList)
    {
        highscoreObject->setHighscoreGroup(QLatin1String( groupKey ));
        player = highscoreObject->readEntry(0, QLatin1String( "LastPlayer" ));  //FIXME

        for (int i = 1; i <= 10; ++i)
        {
            FieldInfo score;
            for(int field = 1; field < fields; field = field * 2)
            {
                if (fields & field)
                {
                    score[field] = highscoreObject->readEntry(i, key[field], QLatin1String("-"));
                }
            }
            scores[groupKey].append(score);
        }
    }
    highscoreObject->setHighscoreGroup(QLatin1String( tempCurrentGroup )); //reset to the user-set group name
    foreach(const QByteArray &groupKey, scores.keys())
    {
        if( (scores[groupKey][0].value(Score)==QLatin1String( "-" )) && (scores.size() > 1) && (latest.first != groupKey) )
        {
            qCDebug(GAMES_HIGHSCORE) << "Removing group " << groupKey << " since it's unused.";
            scores.remove(groupKey);
        }
    }
    loaded = true;
}

void KScoreDialog::KScoreDialogPrivate::saveScores()
{
    highscoreObject->setHighscoreGroup(QLatin1String( configGroup ));

    highscoreObject->writeEntry(0,QLatin1String( "LastPlayer" ), player);

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
    qCDebug(GAMES_HIGHSCORE) << "adding new score";

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
        FieldInfo score = d->scores[d->configGroup].at(i); //First look at the score in the config file
        bool ok; //will be false if there isn't any score yet in position i
        int num_score = score[Score].toLong(&ok); //test if the stored score is a number

        score = FieldInfo(newInfo); //now look at the submitted score
        int newScore = score[Score].toInt();

        qCDebug(GAMES_HIGHSCORE) << "num_score =" << num_score << " - newScore =" << newScore;

        if (((newScore > num_score) && !lessIsMore) ||
              ((newScore < num_score) && lessIsMore) || !ok)
        {
            d->latest = QPair<QByteArray,int>(d->configGroup,i+1);
            d->scores[d->configGroup].insert(i, score);
            // Save the position to delete in case of Forget
            d->lastHighPosition = i;

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

		buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		
		buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("&Remember"));
		buttonBox->button(QDialogButtonBox::Cancel)->setText(i18n("&Forget"));
		buttonBox->button(QDialogButtonBox::Ok)->setToolTip(i18n("Remember this high score"));
		buttonBox->button(QDialogButtonBox::Cancel)->setToolTip(i18n("Forget this high score"));
                
                connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
		connect(buttonBox, SIGNAL(accepted()), SLOT(slotGotName()));
		connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
		connect(buttonBox, SIGNAL(rejected()), SLOT(slotForgetScore()));
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
    QDialog::show();
}

int KScoreDialog::exec()
{
    d->aboutToShow();
    return QDialog::exec();
}

void KScoreDialog::slotGotReturn()
{
    QTimer::singleShot(0, this, SLOT(slotGotName()));
    // TODO: Is it better to hide the window, as if any button where pressed?
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
    d->stack[d->newName.first].at((d->newName.second-1))->removeWidget(d->edit);
    delete d->edit;
    d->edit = 0;
    d->newName = QPair<QByteArray,int>(QByteArray(),-1);
    d->scores[d->configGroup].removeAt(10);
    d->comment.clear();  // hide the congratulations
    d->commentLabel->hide();
    
    delete buttonBox;
    buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void KScoreDialog::slotForgetScore()
{
    if (d->newName.second == -1) return;
    // remove the editor from the stack
    d->stack[d->newName.first].at((d->newName.second-1))->removeWidget(d->edit);
    // delete the editor
    delete d->edit;
    d->edit = 0;
    // avoid to recreate the KTextEdit widget
    d->newName = QPair<QByteArray,int>(QByteArray(),-1);
    // delete the highscore to forget
    d->scores[d->configGroup].removeAt(d->lastHighPosition);
    d->comment.clear();
    d->commentLabel->hide();
    
    delete buttonBox;
    buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
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
    QDialog::keyPressEvent(ev);
}


