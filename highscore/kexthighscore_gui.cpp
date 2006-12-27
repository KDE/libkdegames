/*
    This file is part of the KDE games library
    Copyright (C) 2001-2003 Nicolas Hadacek (hadacek@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kexthighscore_gui.h"
#include "kexthighscore_gui.moc"

#include <QLayout>
#include <qtextstream.h>
#include <q3header.h>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kurllabel.h>
#include <krun.h>
#include <kfiledialog.h>
#include <kvbox.h>
#include <ktemporaryfile.h>
#include <kio/netaccess.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kvbox.h>

#include "kexthighscore_internal.h"
#include "kexthighscore.h"
#include "kexthighscore_tab.h"


namespace KExtHighscore
{

//-----------------------------------------------------------------------------
ShowItem::ShowItem(Q3ListView *list, bool highlight)
    : K3ListViewItem(list), _highlight(highlight)
{}

void ShowItem::paintCell(QPainter *p, const QColorGroup &cg,
                         int column, int width, int align)
{
    QColorGroup cgrp(cg);
    if (_highlight) cgrp.setColor(QPalette::Text, Qt::red);
    K3ListViewItem::paintCell(p, cgrp, column, width, align);
}

//-----------------------------------------------------------------------------
ScoresList::ScoresList(QWidget *parent)
    : K3ListView(parent)
{
    setSelectionMode(Q3ListView::NoSelection);
    setItemMargin(3);
    setAllColumnsShowFocus(true);
    setSorting(-1);
    header()->setClickEnabled(false);
    header()->setMovingEnabled(false);
}

void ScoresList::addHeader(const ItemArray &items)
{
    addLineItem(items, 0, 0);
}

Q3ListViewItem *ScoresList::addLine(const ItemArray &items,
                                   uint index, bool highlight)
{
    Q3ListViewItem *item = new ShowItem(this, highlight);
    addLineItem(items, index, item);
    return item;
}

void ScoresList::addLineItem(const ItemArray &items,
                             uint index, Q3ListViewItem *line)
{
    uint k = 0;
    for (int i=0; i<items.size(); i++) {
        const ItemContainer &container = *items[i];
        if ( !container.item()->isVisible() ) continue;
        if (line) line->setText(k, itemText(container, index));
        else {
            addColumn( container.item()->label() );
            setColumnAlignment(k, container.item()->alignment());
        }
        k++;
    }
}

//-----------------------------------------------------------------------------
HighscoresList::HighscoresList(QWidget *parent)
    : ScoresList(parent)
{}

QString HighscoresList::itemText(const ItemContainer &item, uint row) const
{
    return item.pretty(row);
}

void HighscoresList::load(const ItemArray &items, int highlight)
{
    clear();
    Q3ListViewItem *line = 0;
    for (int j=items.nbEntries()-1; j>=0; j--) {
        Q3ListViewItem *item = addLine(items, j, j==highlight);
        if ( j==highlight ) line = item;
    }
    if (line) ensureItemVisible(line);
}

//-----------------------------------------------------------------------------
HighscoresWidget::HighscoresWidget(QWidget *parent)
    : QWidget(parent),
      _scoresUrl(0), _playersUrl(0), _statsTab(0), _histoTab(0)
{
    setObjectName("show_highscores_widget");
    const ScoreInfos &s = internal->scoreInfos();
    const PlayerInfos &p = internal->playerInfos();

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(KDialog::spacingHint());

    _tw = new QTabWidget(this);
    connect(_tw, SIGNAL(currentChanged(QWidget *)), SLOT(tabChanged()));
    vbox->addWidget(_tw);

    // scores tab
    _scoresList = new HighscoresList(0);
    _scoresList->addHeader(s);
    _tw->addTab(_scoresList, i18n("Best &Scores"));

    // players tab
    _playersList = new HighscoresList(0);
    _playersList->addHeader(p);
    _tw->addTab(_playersList, i18n("&Players"));

    // statistics tab
    if ( internal->showStatistics ) {
        _statsTab = new StatisticsTab(0);
        _tw->addTab(_statsTab, i18n("Statistics"));
    }

    // histogram tab
    if ( p.histogram().size()!=0 ) {
        _histoTab = new HistogramTab(0);
        _tw->addTab(_histoTab, i18n("Histogram"));
    }

    // url labels
    if ( internal->isWWHSAvailable() ) {
        KUrl url = internal->queryUrl(ManagerPrivate::Scores);
        _scoresUrl = new KUrlLabel(url.url(),
                                   i18n("View world-wide highscores"), this);
        connect(_scoresUrl, SIGNAL(leftClickedUrl(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(_scoresUrl);

        url = internal->queryUrl(ManagerPrivate::Players);
        _playersUrl = new KUrlLabel(url.url(),
                                    i18n("View world-wide players"), this);
        connect(_playersUrl, SIGNAL(leftClickedUrl(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(_playersUrl);
    }
}

void HighscoresWidget::changeTab(int i)
{
    if ( i!=_tw->currentIndex() )
        _tw->setCurrentIndex(i);
}

void HighscoresWidget::showURL(const QString &url)
{
    (void)new KRun(KUrl(url), this);
}

void HighscoresWidget::load(int rank)
{
    _scoresList->load(internal->scoreInfos(), rank);
    _playersList->load(internal->playerInfos(), internal->playerInfos().id());
    if (_scoresUrl)
        _scoresUrl->setUrl(internal->queryUrl(ManagerPrivate::Scores).url());
    if (_playersUrl)
        _playersUrl->setUrl(internal->queryUrl(ManagerPrivate::Players).url());
    if (_statsTab) _statsTab->load();
    if (_histoTab) _histoTab->load();
}

//-----------------------------------------------------------------------------
HighscoresDialog::HighscoresDialog(int rank, QWidget *parent)
    : KPageDialog(parent), _rank(rank), _tab(0)
{
    setCaption( i18n("Highscores") );
    setButtons( Close|User1|User2 );
    setDefaultButton( Close );
    if ( internal->nbGameTypes()>1 )
        setFaceType( KPageDialog::Tree );
    else
        setFaceType( KPageDialog::Plain );
    setButtonGuiItem( User1, KGuiItem(i18n("Configure..."), "configure") );
    setButtonGuiItem( User2, KGuiItem(i18n("Export...")) );
    connect( this, SIGNAL(user1Clicked()), SLOT(slotUser1()) );
    connect( this, SIGNAL(user2Clicked()), SLOT(slotUser2()) );

    for (uint i=0; i<internal->nbGameTypes(); i++) {
        QString title = internal->manager.gameTypeLabel(i, Manager::I18N);
        QString icon = internal->manager.gameTypeLabel(i, Manager::Icon);
        HighscoresWidget *hsw = new HighscoresWidget(0);
        KPageWidgetItem *pageItem = new KPageWidgetItem( hsw, title);
        pageItem->setIcon( KIcon( BarIcon(icon, K3Icon::SizeLarge) ) );
        addPage( pageItem );
        _pages.append(pageItem);
        connect(hsw, SIGNAL(tabChanged(int)), SLOT(tabChanged(int)));
    }

    connect(this, SIGNAL( currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)),
            SLOT(highscorePageChanged(KPageWidgetItem *, KPageWidgetItem *)));
    setCurrentPage(_pages[internal->gameType()]);
}

void HighscoresDialog::highscorePageChanged(KPageWidgetItem* page, KPageWidgetItem* pageold)
{
    Q_UNUSED(pageold);
    int idx = _pages.indexOf( page );
    Q_ASSERT(idx != -1);

    internal->hsConfig().readCurrentConfig();
    uint type = internal->gameType();
    bool several = ( internal->nbGameTypes()>1 );
    if (several)
        internal->setGameType(idx);
    HighscoresWidget *hsw = static_cast<HighscoresWidget*>(page->widget());
    hsw->load(uint(idx)==type ? _rank : -1);
    if (several) setGameType(type);
    hsw->changeTab(_tab);
}

void HighscoresDialog::slotUser1()
{
    if ( KExtHighscore::configure(this) )
        highscorePageChanged(currentPage(), 0);//update data
}

void HighscoresDialog::slotUser2()
{
    KUrl url = KFileDialog::getSaveUrl(KUrl(), QString(), this);
    if ( url.isEmpty() ) return;
    if ( KIO::NetAccess::exists(url, true, this) ) {
        KGuiItem gi = KStandardGuiItem::save();
        gi.setText(i18n("Overwrite"));
        int res = KMessageBox::warningContinueCancel(this,
                                 i18n("The file already exists. Overwrite?"),
                                 i18n("Export"), gi);
        if ( res==KMessageBox::Cancel ) return;
    }
    KTemporaryFile tmp;
    tmp.open();
    QTextStream stream(&tmp);
    internal->exportHighscores(stream);
    stream.flush();
    KIO::NetAccess::upload(tmp.fileName(), url, this);
}

//-----------------------------------------------------------------------------
LastMultipleScoresList::LastMultipleScoresList(
                            const QVector<Score> &scores, QWidget *parent)
    : ScoresList(parent), _scores(scores)
{
    const ScoreInfos &s = internal->scoreInfos();
    addHeader(s);
    for (int i=0; i<scores.size(); i++) addLine(s, i, false);
}

void LastMultipleScoresList::addLineItem(const ItemArray &si,
                                         uint index, Q3ListViewItem *line)
{
    uint k = 1; // skip "id"
    for (int i=0; i<si.size()-2; i++) {
        if ( i==3 ) k = 5; // skip "date"
        const ItemContainer *container = si[k];
        k++;
        if (line) line->setText(i, itemText(*container, index));
        else {
            addColumn(  container->item()->label() );
            setColumnAlignment(i, container->item()->alignment());
        }
    }
}

QString LastMultipleScoresList::itemText(const ItemContainer &item,
                                         uint row) const
{
    QString name = item.name();
    if ( name=="rank" )
        return (_scores[row].type()==Won ? i18n("Winner") : QString::null);
    QVariant v = _scores[row].data(name);
    if ( name=="name" ) return v.toString();
    return item.item()->pretty(row, v);
}

//-----------------------------------------------------------------------------
TotalMultipleScoresList::TotalMultipleScoresList(
                            const QVector<Score> &scores, QWidget *parent)
    : ScoresList(parent), _scores(scores)
{
    const ScoreInfos &s = internal->scoreInfos();
    addHeader(s);
    for (int i=0; i<scores.size(); i++) addLine(s, i, false);
}

void TotalMultipleScoresList::addLineItem(const ItemArray &si,
                                          uint index, Q3ListViewItem *line)
{
    const PlayerInfos &pi = internal->playerInfos();
    uint k = 1; // skip "id"
    for (uint i=0; i<4; i++) { // skip additional fields
        const ItemContainer *container;
        if ( i==2 ) container = pi.item("nb games");
        else if ( i==3 ) container = pi.item("mean score");
        else {
            container = si[k];
            k++;
        }
        if (line) line->setText(i, itemText(*container, index));
        else {
            QString label =
                (i==2 ? i18n("Won Games") : container->item()->label());
            addColumn(label);
            setColumnAlignment(i, container->item()->alignment());
        }
    }
}

QString TotalMultipleScoresList::itemText(const ItemContainer &item,
                                          uint row) const
{
    QString name = item.name();
    if ( name=="rank" ) return QString::number(_scores.size()-row);
    if ( name=="nb games" )
        return QString::number( _scores[row].data("nb won games").toUInt() );
    QVariant v = _scores[row].data(name);
    if ( name=="name" ) return v.toString();
    return item.item()->pretty(row, v);
}


//-----------------------------------------------------------------------------
ConfigDialog::ConfigDialog(QWidget *parent)
    : KDialog(parent),
      _saved(false), _WWHEnabled(0)
{
    setCaption( i18n("Configure Highscores") );
    setButtons( Ok|Apply|Cancel );
    setDefaultButton( Cancel );
    setModal( true );
    QWidget *page = 0;
    QTabWidget *tab = 0;
    if ( internal->isWWHSAvailable() ) {
        tab = new QTabWidget(this);
        setMainWidget(tab);
        page = new QWidget;
        tab->addTab(page, i18n("Main"));
    } else {
        page = new QWidget(this);
        setMainWidget(page);
    }

    QGridLayout *pageTop =
        new QGridLayout(page);
    pageTop->setMargin(spacingHint());
    pageTop->setSpacing(spacingHint());

    QLabel *label = new QLabel(i18n("Nickname:"), page);
    pageTop->addWidget(label, 0, 0);
    _nickname = new QLineEdit(page);
    connect(_nickname, SIGNAL(textChanged(const QString &)),
            SLOT(modifiedSlot()));
    connect(_nickname, SIGNAL(textChanged(const QString &)),
            SLOT(nickNameChanged(const QString &)));

    _nickname->setMaxLength(16);
    pageTop->addWidget(_nickname, 0, 1);

    label = new QLabel(i18n("Comment:"), page);
    pageTop->addWidget(label, 1, 0);
    _comment = new QLineEdit(page);
    connect(_comment, SIGNAL(textChanged(const QString &)),
            SLOT(modifiedSlot()));
    _comment->setMaxLength(50);
    pageTop->addWidget(_comment, 1, 1);

    if (tab) {
        _WWHEnabled
            = new QCheckBox(i18n("World-wide highscores enabled"), page);
        connect(_WWHEnabled, SIGNAL(toggled(bool)),
                SLOT(modifiedSlot()));
        pageTop->addWidget(_WWHEnabled, 2, 0, 1, 2 );

        // advanced tab
        QWidget *page = new QWidget;
        tab->addTab(page, i18n("Advanced"));
        QVBoxLayout *pageTop = new QVBoxLayout(page);
        pageTop->setMargin(marginHint());
        pageTop->setSpacing(spacingHint());

        QGroupBox *group = new QGroupBox(page);
        group->setTitle( i18n("Registration Data") );
        pageTop->addWidget(group);
        QGridLayout *groupLayout = new QGridLayout(group);
        groupLayout->setSpacing(spacingHint());

        label = new QLabel(i18n("Nickname:"), group);
        groupLayout->addWidget(label, 0, 0);
        _registeredName = new KLineEdit(group);
        _registeredName->setReadOnly(true);
        groupLayout->addWidget(_registeredName, 0, 1);

        label = new QLabel(i18n("Key:"), group);
        groupLayout->addWidget(label, 1, 0);
        _key = new KLineEdit(group);
        _key->setReadOnly(true);
        groupLayout->addWidget(_key, 1, 1);

        KGuiItem gi = KStandardGuiItem::clear();
        gi.setText(i18n("Remove"));
        _removeButton = new KPushButton(gi, group);
        groupLayout->addWidget(_removeButton, 2, 0);
        connect(_removeButton, SIGNAL(clicked()), SLOT(removeSlot()));
    }

    load();
    enableButtonOk( !_nickname->text().isEmpty() );
    enableButtonApply(false);
}

void ConfigDialog::nickNameChanged(const QString &text)
{
    enableButtonOk( !text.isEmpty() );
}


void ConfigDialog::modifiedSlot()
{
    enableButtonApply(true && !_nickname->text().isEmpty() );
}

void ConfigDialog::accept()
{
    if ( save() ) {
        KDialog::accept();
        KGlobal::config()->sync(); // safer
    }
}

void ConfigDialog::removeSlot()
{
    KGuiItem gi = KStandardGuiItem::clear();
    gi.setText(i18n("Remove"));
    int res = KMessageBox::warningContinueCancel(this,
                               i18n("This will permanently remove your "
                               "registration key. You will not be able to use "
                               "the currently registered nickname anymore."),
                               QString::null, gi);
    if ( res==KMessageBox::Continue ) {
        internal->playerInfos().removeKey();
        _registeredName->clear();
        _key->clear();
        _removeButton->setEnabled(false);
        _WWHEnabled->setChecked(false);
        modifiedSlot();
    }
}

void ConfigDialog::load()
{
    internal->hsConfig().readCurrentConfig();
    const PlayerInfos &infos = internal->playerInfos();
    _nickname->setText(infos.isAnonymous() ? QString::null : infos.name());
    _comment->setText(infos.comment());
    if (_WWHEnabled) {
        _WWHEnabled->setChecked(infos.isWWEnabled());
        if ( !infos.key().isEmpty() ) {
            _registeredName->setText(infos.registeredName());
            _registeredName->home(false);
            _key->setText(infos.key());
            _key->home(false);
        }
        _removeButton->setEnabled(!infos.key().isEmpty());
    }
}

bool ConfigDialog::save()
{
    bool enabled = (_WWHEnabled ? _WWHEnabled->isChecked() : false);

    // do not bother the user with "nickname empty" if he has not
    // messed with nickname settings ...
    QString newName = _nickname->text();
    if ( newName.isEmpty() && !internal->playerInfos().isAnonymous()
         && !enabled ) return true;

    if ( newName.isEmpty() ) {
        KMessageBox::sorry(this, i18n("Please choose a non empty nickname."));
        return false;
    }
    if ( internal->playerInfos().isNameUsed(newName) ) {
        KMessageBox::sorry(this, i18n("Nickname already in use. Please "
                                      "choose another one"));
        return false;
    }

    int res =
        internal->modifySettings(newName, _comment->text(), enabled, this);
    if (res) {
        load(); // needed to update view when "apply" is clicked
        enableButtonApply(false);
    }
    _saved = true;
    return res;
}

//-----------------------------------------------------------------------------
AskNameDialog::AskNameDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Enter Your Nickname") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    internal->hsConfig().readCurrentConfig();
    QWidget *main = new QWidget( this );
    setMainWidget( main );
    QVBoxLayout *top = new QVBoxLayout( main );
    top->setMargin( marginHint() );
    top->setSpacing( spacingHint() );

    QLabel *label =
        new QLabel(i18n("Congratulations, you have won!"), main);
    top->addWidget(label);

    QHBoxLayout *hbox = new QHBoxLayout;
    top->addLayout(hbox);
    label = new QLabel(i18n("Enter your nickname:"), main);
    hbox->addWidget(label);
    _edit = new QLineEdit(main);
    _edit->setFocus();
    connect(_edit, SIGNAL(textChanged(const QString &)), SLOT(nameChanged()));
    hbox->addWidget(_edit);

    top->addSpacing(spacingHint());
    _checkbox = new QCheckBox(i18n("Do not ask again."),  main);
    top->addWidget(_checkbox);

    nameChanged();
}

void AskNameDialog::nameChanged()
{
    enableButtonOk( !name().isEmpty()
                    && !internal->playerInfos().isNameUsed(name()) );
}

} // namespace
