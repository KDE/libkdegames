/*
    This file is part of the KDE games library
    Copyright (C) 2002 Nicolas Hadacek (hadacek@kde.org)

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

#include "kexthighscore_tab.h"
#include "kexthighscore_tab.moc"

#include <QLayout>
#include <QLabel>
#include <QPixmap>
#include <QVector>
#include <QGroupBox>
#include <q3header.h>

#include <kdialog.h>
#include <k3listview.h>
#include <kdebug.h>
#include <kglobal.h>

#include "kexthighscore.h"
#include "kexthighscore_internal.h"


namespace KExtHighscore
{

//-----------------------------------------------------------------------------
PlayersCombo::PlayersCombo(QWidget *parent)
    : QComboBox(parent)
{
    const PlayerInfos &p = internal->playerInfos();
    for (uint i = 0; i<p.nbEntries(); i++)
        addItem(p.prettyName(i));
    addItem(QString("<") + i18n("all") + '>');
    connect(this, SIGNAL(activated(int)), SLOT(activatedSlot(int)));
}

void PlayersCombo::activatedSlot(int i)
{
    const PlayerInfos &p = internal->playerInfos();
    if ( i==(int)p.nbEntries() ) emit allSelected();
    else if ( i==(int)p.nbEntries()+1 ) emit noneSelected();
    else emit playerSelected(i);
}

void PlayersCombo::load()
{
    const PlayerInfos &p = internal->playerInfos();
    for (uint i = 0; i<p.nbEntries(); i++)
        setItemText(i, p.prettyName(i));
}

//-----------------------------------------------------------------------------
AdditionalTab::AdditionalTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setMargin( KDialog::marginHint() );
    top->setSpacing( KDialog::spacingHint() );

    QHBoxLayout *hbox = new QHBoxLayout;
    top->addLayout(hbox);
    QLabel *label = new QLabel(i18n("Select player:"), this);
    hbox->addWidget(label);
    _combo = new PlayersCombo(this);
    connect(_combo, SIGNAL(playerSelected(uint)),
            SLOT(playerSelected(uint)));
    connect(_combo, SIGNAL(allSelected()), SLOT(allSelected()));
    hbox->addWidget(_combo);
    hbox->addStretch(1);
}

void AdditionalTab::init()
{
    uint id = internal->playerInfos().id();
    _combo->setCurrentIndex(id);
    playerSelected(id);
}

void AdditionalTab::allSelected()
{
    display(internal->playerInfos().nbEntries());
}

QString AdditionalTab::percent(uint n, uint total, bool withBraces)
{
    if ( n==0 || total==0 ) return QString::null;
    QString s =  QString("%1%").arg(100.0 * n / total, 0, 'f', 1);
    return (withBraces ? QString("(") + s + ")" : s);
}

void AdditionalTab::load()
{
    _combo->load();
}


//-----------------------------------------------------------------------------
const char *StatisticsTab::COUNT_LABELS[Nb_Counts] = {
    I18N_NOOP("Total:"), I18N_NOOP("Won:"), I18N_NOOP("Lost:"),
    I18N_NOOP("Draw:")
};
const char *StatisticsTab::TREND_LABELS[Nb_Trends] = {
    I18N_NOOP("Current:"), I18N_NOOP("Max won:"), I18N_NOOP("Max lost:")
};

StatisticsTab::StatisticsTab(QWidget *parent)
    : AdditionalTab(parent)
{
    setObjectName("statistics_tab");
    // construct GUI
    QVBoxLayout *top = static_cast<QVBoxLayout *>(layout());

    QHBoxLayout *hbox = new QHBoxLayout;
    QVBoxLayout *vbox = new QVBoxLayout;
    hbox->addLayout(vbox);
    top->addLayout(hbox);

    QGroupBox *group = new QGroupBox(i18n("Game Counts"), this);
    vbox->addWidget(group);
    QGridLayout *gridLay = new QGridLayout(group);
    gridLay->setSpacing(KDialog::spacingHint());
    for (uint k=0; k<Nb_Counts; k++) {
        if ( Count(k)==Draw && !internal->showDrawGames ) continue;
        gridLay->addWidget(new QLabel(i18n(COUNT_LABELS[k]), group), k, 0);
        _nbs[k] = new QLabel(group);
        gridLay->addWidget(_nbs[k], k, 1);
        _percents[k] = new QLabel(group);
        gridLay->addWidget(_percents[k], k, 2);
    }

    group = new QGroupBox(i18n("Trends"), this);
    vbox->addWidget(group);
    gridLay = new QGridLayout(group);
    gridLay->setSpacing(KDialog::spacingHint());
    for (uint k=0; k<Nb_Trends; k++) {
        gridLay->addWidget(new QLabel(i18n(TREND_LABELS[k]), group), k, 0);
        _trends[k] = new QLabel(group);
        gridLay->addWidget(_trends[k], k, 1);
    }

    hbox->addStretch(1);
    top->addStretch(1);
}

void StatisticsTab::load()
{
    AdditionalTab::load();
    const PlayerInfos &pi = internal->playerInfos();
    uint nb = pi.nbEntries();
    _data.resize(nb+1);
    for (int i=0; i<_data.size()-1; i++) {
        _data[i].count[Total] = pi.item("nb games")->read(i).toUInt();
        _data[i].count[Lost] = pi.item("nb lost games")->read(i).toUInt()
                       + pi.item("nb black marks")->read(i).toUInt(); // legacy
        _data[i].count[Draw] = pi.item("nb draw games")->read(i).toUInt();
        _data[i].count[Won] = _data[i].count[Total] - _data[i].count[Lost]
                              - _data[i].count[Draw];
        _data[i].trend[CurrentTrend] =
            pi.item("current trend")->read(i).toInt();
        _data[i].trend[WonTrend] = pi.item("max won trend")->read(i).toUInt();
        _data[i].trend[LostTrend] =
            -(int)pi.item("max lost trend")->read(i).toUInt();
    }

    for (int k=0; k<Nb_Counts; k++) _data[nb].count[k] = 0;
    for (int k=0; k<Nb_Trends; k++) _data[nb].trend[k] = 0;
    for (int i=0; i<_data.size()-1; i++) {
        for (uint k=0; k<Nb_Counts; k++)
            _data[nb].count[k] += _data[i].count[k];
        for (uint k=0; k<Nb_Trends; k++)
            _data[nb].trend[k] += _data[i].trend[k];
    }
    for (uint k=0; k<Nb_Trends; k++)
        _data[nb].trend[k] /= (_data.size()-1);

    init();
}

QString StatisticsTab::percent(const Data &d, Count count) const
{
    if ( count==Total ) return QString::null;
    return AdditionalTab::percent(d.count[count], d.count[Total], true);
}

void StatisticsTab::display(uint i)
{
    const Data &d = _data[i];
    for (uint k=0; k<Nb_Counts; k++) {
        if ( Count(k) && !internal->showDrawGames ) continue;
        _nbs[k]->setText(QString::number(d.count[k]));
        _percents[k]->setText(percent(d, Count(k)));
    }
    for (uint k=0; k<Nb_Trends; k++) {
        QString s;
        if ( d.trend[k]>0 ) s = '+';
        int prec = (i==internal->playerInfos().nbEntries() ? 1 : 0);
        _trends[k]->setText(s + QString::number(d.trend[k], 'f', prec));
    }
}

//-----------------------------------------------------------------------------
HistogramTab::HistogramTab(QWidget *parent)
    : AdditionalTab(parent)
{
    setObjectName("histogram_tab");
    // construct GUI
    QVBoxLayout *top = static_cast<QVBoxLayout *>(layout());

    _list = new K3ListView(this);
    _list->setSelectionMode(Q3ListView::NoSelection);
    _list->setItemMargin(3);
    _list->setAllColumnsShowFocus(true);
    _list->setSorting(-1);
    _list->header()->setClickEnabled(false);
    _list->header()->setMovingEnabled(false);
    top->addWidget(_list);

    _list->addColumn(i18n("From"));
    _list->addColumn(i18n("To"));
    _list->addColumn(i18n("Count"));
    _list->addColumn(i18n("Percent"));
    for (int i=0; i<4; i++) _list->setColumnAlignment(i, Qt::AlignRight);
    _list->addColumn(QString::null);

    const Item *sitem = internal->scoreInfos().item("score")->item();
    const PlayerInfos &pi = internal->playerInfos();
    const QVector<uint> &sh = pi.histogram();
    for (int k=1; k<( int )pi.histoSize(); k++) {
        QString s1 = sitem->pretty(0, sh[k-1]);
        QString s2;
        if ( k==sh.size() ) s2 = "...";
        else if ( sh[k]!=sh[k-1]+1 ) s2 = sitem->pretty(0, sh[k]);
        (void)new K3ListViewItem(_list, s1, s2);
    }
}

void HistogramTab::load()
{
    AdditionalTab::load();
    const PlayerInfos &pi = internal->playerInfos();
    uint n = pi.nbEntries();
    uint s = pi.histoSize() - 1;
    _counts.resize((n+1) * s);
    _data.fill(0, n+1);
    for (uint k=0; k<s; k++) {
        _counts[n*s + k] = 0;
        for (uint i=0; i<n; i++) {
            uint nb = pi.item(pi.histoName(k+1))->read(i).toUInt();
            _counts[i*s + k] = nb;
            _counts[n*s + k] += nb;
            _data[i] += nb;
            _data[n] += nb;
        }
    }

    init();
}

void HistogramTab::display(uint i)
{
    const PlayerInfos &pi = internal->playerInfos();
    Q3ListViewItem *item = _list->firstChild();
    uint s = pi.histoSize() - 1;
    for (int k=s-1; k>=0; k--) {
        uint nb = _counts[i*s + k];
        item->setText(2, QString::number(nb));
        item->setText(3, percent(nb, _data[i]));
        uint width = (_data[i]==0 ? 0 : qRound(150.0 * nb / _data[i]));
        QPixmap pixmap(width, 10);
        pixmap.fill(Qt::blue);
        item->setPixmap(4, pixmap);
        item = item->nextSibling();
    }
}

} // namespace
