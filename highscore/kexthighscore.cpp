/*
    This file is part of the KDE games library
    Copyright (C) 2001-2004 Nicolas Hadacek (hadacek@kde.org)

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

#include "kexthighscore.h"

#include <qlayout.h>

#include <kdebug.h>

#include "kexthighscore_internal.h"
#include "kexthighscore_gui.h"


namespace KExtHighscore
{

//-----------------------------------------------------------------------------
ManagerPrivate *internal = 0;

uint gameType()
{
    internal->checkFirst();
    return internal->gameType();
}

void setGameType(uint type)
{
    internal->setGameType(type);
}

bool configure(QWidget *parent)
{
    internal->checkFirst();
    ConfigDialog *cd = new ConfigDialog(parent);
    cd->exec();
    bool saved = cd->hasBeenSaved();
    delete cd;
    return saved;
}

void show(QWidget *parent, int rank)
{
    HighscoresDialog *hd = new HighscoresDialog(rank, parent);
    hd->exec();
    delete hd;
}

void submitScore(const Score &score, QWidget *widget)
{
    int rank = internal->submitScore(score, widget,
                                     internal->showMode!=Manager::NeverShow);

    switch (internal->showMode) {
    case Manager::AlwaysShow:
        show(widget, -1);
        break;
    case Manager::ShowForHigherScore:
        if ( rank!=-1) show(widget, rank);
        break;
    case Manager::ShowForHighestScore:
        if ( rank==0 ) show(widget, rank);
        break;
    case Manager::NeverShow:
        break;
    }
}

void show(QWidget *widget)
{
    internal->checkFirst();
    show(widget, -1);
}

Score lastScore()
{
    internal->checkFirst();
    internal->hsConfig().readCurrentConfig();
    uint nb = internal->scoreInfos().maxNbEntries();
    return internal->readScore(nb-1);
}

Score firstScore()
{
    internal->checkFirst();
    internal->hsConfig().readCurrentConfig();
    return internal->readScore(0);
}


//-----------------------------------------------------------------------------
Manager::Manager(uint nbGameTypes, uint maxNbEntries)
{
    Q_ASSERT(nbGameTypes);
    Q_ASSERT(maxNbEntries);
    if (internal)
        kdFatal(11002) << "A highscore object already exists" << endl;
    internal = new ManagerPrivate(nbGameTypes, *this);
    internal->init(maxNbEntries);
}

Manager::~Manager()
{
    delete internal;
    internal = 0;
}

void Manager::setTrackLostGames(bool track)
{
    internal->trackLostGames = track;
}

void Manager::setTrackDrawGames(bool track)
{
    internal->trackDrawGames = track;
}

void Manager::setShowStatistics(bool show)
{
    internal->showStatistics = show;
}

void Manager::showStatistics(bool show)
{
    internal->showStatistics = show;
}

void Manager::setShowDrawGamesStatistic(bool show)
{
    internal->showDrawGames = show;
}

void Manager::setWWHighscores(const KURL &url, const QString &version)
{
    Q_ASSERT( url.isValid() );
    internal->serverURL = url;
    const char *HS_WW_URL = "ww hs url";
    ConfigGroup cg;
    if ( cg.config()->hasKey(HS_WW_URL) )
        internal->serverURL = cg.config()->readEntry(HS_WW_URL);
    else cg.config()->writeEntry(HS_WW_URL, url.url());
    internal->version = version;
}

void Manager::setScoreHistogram(const QMemArray<uint> &scores,
                                ScoreTypeBound type)
{
    Q_ASSERT( scores.size()>=2 );
    for (uint i=0; i<scores.size()-1; i++)
        Q_ASSERT( scores[i]<scores[i+1] );
    internal->playerInfos().createHistoItems(scores, type==ScoreBound);
}

void Manager::setShowMode(ShowMode mode)
{
    internal->showMode = mode;
}

void Manager::setScoreType(ScoreType type)
{
    switch (type) {
    case Normal:
        return;
    case MinuteTime: {
        Item *item = createItem(ScoreDefault);
        item->setPrettyFormat(Item::MinuteTime);
        setScoreItem(0, item);

        item = createItem(MeanScoreDefault);
        item->setPrettyFormat(Item::MinuteTime);
        setPlayerItem(MeanScore, item);

        item = createItem(BestScoreDefault);
        item->setPrettyFormat(Item::MinuteTime);
        setPlayerItem(BestScore, item);
        return;
    }
    }
}

void Manager::submitLegacyScore(const Score &score) const
{
    internal->submitLocal(score);
}

bool Manager::isStrictlyLess(const Score &s1, const Score &s2) const
{
    return s1.score()<s2.score();
}

Item *Manager::createItem(ItemType type)
{
    Item *item = 0;
    switch (type) {
    case ScoreDefault:
        item = new Item((uint)0, i18n("Score"), Qt::AlignRight);
        break;
    case MeanScoreDefault:
        item = new Item((double)0, i18n("Mean Score"), Qt::AlignRight);
        item->setPrettyFormat(Item::OneDecimal);
        item->setPrettySpecial(Item::DefaultNotDefined);
        break;
    case BestScoreDefault:
        item = new Item((uint)0, i18n("Best Score"), Qt::AlignRight);
        item->setPrettySpecial(Item::DefaultNotDefined);
        break;
    case ElapsedTime:
        item = new Item((uint)0, i18n("Elapsed Time"), Qt::AlignRight);
        item->setPrettyFormat(Item::MinuteTime);
        item->setPrettySpecial(Item::ZeroNotDefined);
        break;
    }
    return item;
}

void Manager::setScoreItem(uint worstScore, Item *item)
{
    item->setDefaultValue(worstScore);
    internal->scoreInfos().setItem("score", item);
    internal->playerInfos().item("mean score")
        ->item()->setDefaultValue(double(worstScore));
    internal->playerInfos().item("best score")
        ->item()->setDefaultValue(worstScore);
}

void Manager::addScoreItem(const QString &name, Item *item)
{
    internal->scoreInfos().addItem(name, item, true);
}

void Manager::setPlayerItem(PlayerItemType type, Item *item)
{
    const Item *scoreItem = internal->scoreInfos().item("score")->item();
    uint def = scoreItem->defaultValue().toUInt();
    QString name;
    switch (type) {
    case MeanScore:
        name = "mean score";
        item->setDefaultValue(double(def));
        break;
    case BestScore:
        name = "best score";
        item->setDefaultValue(def);
        break;
    }
    internal->playerInfos().setItem(name, item);
}

QString Manager::gameTypeLabel(uint gameType, LabelType type) const
{
    if ( gameType!=0 )
        kdFatal(11002) << "You need to reimplement KExtHighscore::Manager for "
                       << "multiple game types" << endl;
    switch (type) {
    case Icon:
    case Standard:
    case I18N:     break;
    case WW:       return "normal";
    }
    return QString::null;
}

void Manager::addToQueryURL(KURL &url, const QString &item,
                               const QString &content)
{
    Q_ASSERT( !item.isEmpty() && url.queryItem(item).isNull() );

    QString query = url.query();
    if ( !query.isEmpty() ) query += '&';
	query += item + '=' + KURL::encode_string(content);
	url.setQuery(query);
}

} // namescape
