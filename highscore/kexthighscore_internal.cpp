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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include "kexthighscore_internal.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <qfile.h>
#include <qlayout.h>
#include <qdom.h>

#include <kglobal.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kmdcodec.h>
#include <kdebug.h>

#include "kexthighscore.h"
#include "kexthighscore_gui.h"
#include "kemailsettings.h"


namespace KExtHighscore
{

//-----------------------------------------------------------------------------
const char ItemContainer::ANONYMOUS[] = "_";
const char ItemContainer::ANONYMOUS_LABEL[] = I18N_NOOP("anonymous");

ItemContainer::ItemContainer()
    : _item(0)
{}

ItemContainer::~ItemContainer()
{
    delete _item;
}

void ItemContainer::setItem(Item *item)
{
    delete _item;
    _item = item;
}

QString ItemContainer::entryName() const
{
    if ( _subGroup.isEmpty() ) return _name;
    return _name + "_" + _subGroup;
}

QVariant ItemContainer::read(uint i) const
{
    Q_ASSERT(_item);

    QVariant v = _item->defaultValue();
    if ( isStored() ) {
        internal->hsConfig().setHighscoreGroup(_group);
        v = internal->hsConfig().readPropertyEntry(i+1, entryName(), v);
    }
    return _item->read(i, v);
}

QString ItemContainer::pretty(uint i) const
{
    Q_ASSERT(_item);
    return _item->pretty(i, read(i));
}

void ItemContainer::write(uint i, const QVariant &value) const
{
    Q_ASSERT( isStored() );
    Q_ASSERT( internal->hsConfig().isLocked() );
    internal->hsConfig().setHighscoreGroup(_group);
    internal->hsConfig().writeEntry(i+1, entryName(), value);
}

uint ItemContainer::increment(uint i) const
{
    uint v = read(i).toUInt() + 1;
    write(i, v);
    return v;
}

//-----------------------------------------------------------------------------
ItemArray::ItemArray()
    : _group(""), _subGroup("") // no null groups
{}

ItemArray::~ItemArray()
{
    for (uint i=0; i<size(); i++) delete at(i);
}

int ItemArray::findIndex(const QString &name) const
{
    for (uint i=0; i<size(); i++)
        if ( at(i)->name()==name ) return i;
    return -1;
}

const ItemContainer *ItemArray::item(const QString &name) const
{
    int i = findIndex(name);
    if ( i==-1 ) kdError(11002) << k_funcinfo << "no item named \"" << name
                                << "\"" << endl;
    return at(i);
}

ItemContainer *ItemArray::item(const QString &name)
{
    int i = findIndex(name);
    if ( i==-1 ) kdError(11002) << k_funcinfo << "no item named \"" << name
                                << "\"" << endl;
    return at(i);
}

void ItemArray::setItem(const QString &name, Item *item)
{
    int i = findIndex(name);
    if ( i==-1 ) kdError(11002) << k_funcinfo << "no item named \"" << name
                                << "\"" << endl;
    bool stored = at(i)->isStored();
    bool canHaveSubGroup = at(i)->canHaveSubGroup();
    _setItem(i, name, item, stored, canHaveSubGroup);
}

void ItemArray::addItem(const QString &name, Item *item,
                        bool stored, bool canHaveSubGroup)
{
    if ( findIndex(name)!=-1 )
        kdError(11002) << "item already exists \"" << name << "\"" << endl;
    uint i = size();
    resize(i+1);
    at(i) = new ItemContainer;
    _setItem(i, name, item, stored, canHaveSubGroup);
}

void ItemArray::_setItem(uint i, const QString &name, Item *item,
                         bool stored, bool canHaveSubGroup)
{
    at(i)->setItem(item);
    at(i)->setName(name);
    at(i)->setGroup(stored ? _group : QString::null);
    at(i)->setSubGroup(canHaveSubGroup ? _subGroup : QString::null);
}

void ItemArray::setGroup(const QString &group)
{
    Q_ASSERT( !group.isNull() );
    _group = group;
    for (uint i=0; i<size(); i++)
        if ( at(i)->isStored() ) at(i)->setGroup(group);
}

void ItemArray::setSubGroup(const QString &subGroup)
{
    Q_ASSERT( !subGroup.isNull() );
    _subGroup = subGroup;
    for (uint i=0; i<size(); i++)
        if ( at(i)->canHaveSubGroup() ) at(i)->setSubGroup(subGroup);
}

void ItemArray::read(uint k, Score &data) const
{
    for (uint i=0; i<size(); i++) {
        if ( !at(i)->isStored() ) continue;
        data.setData(at(i)->name(), at(i)->read(k));
    }
}

void ItemArray::write(uint k, const Score &data, uint nb) const
{
    for (uint i=0; i<size(); i++) {
        if ( !at(i)->isStored() ) continue;
        for (uint j=nb-1; j>k; j--)  at(i)->write(j, at(i)->read(j-1));
        at(i)->write(k, data.data(at(i)->name()));
    }
}

void ItemArray::exportToText(QTextStream &s) const
{
    for (uint k=0; k<nbEntries()+1; k++) {
        for (uint i=0; i<size(); i++) {
            const Item *item = at(i)->item();
            if ( item->isVisible() ) {
                if ( i!=0 ) s << '\t';
                if ( k==0 ) s << item->label();
                else s << at(i)->pretty(k-1);
            }
        }
        s << endl;
    }
}

//-----------------------------------------------------------------------------
class ScoreNameItem : public NameItem
{
 public:
    ScoreNameItem(const ScoreInfos &score, const PlayerInfos &infos)
        : _score(score), _infos(infos) {}

    QString pretty(uint i, const QVariant &v) const {
        uint id = _score.item("id")->read(i).toUInt();
        if ( id==0 ) return NameItem::pretty(i, v);
        return _infos.prettyName(id-1);
    }

 private:
    const ScoreInfos  &_score;
    const PlayerInfos &_infos;
};

//-----------------------------------------------------------------------------
ScoreInfos::ScoreInfos(uint maxNbEntries, const PlayerInfos &infos)
    : _maxNbEntries(maxNbEntries)
{
    addItem("id", new Item((uint)0));
    addItem("rank", new RankItem, false);
    addItem("name", new ScoreNameItem(*this, infos));
    addItem("score", Manager::createItem(Manager::ScoreDefault));
    addItem("date", new DateItem);
}

uint ScoreInfos::nbEntries() const
{
    uint i = 0;
    for (; i<_maxNbEntries; i++)
        if ( item("score")->read(i)==item("score")->item()->defaultValue() )
            break;
    return i;
}

//-----------------------------------------------------------------------------
const char *HS_ID              = "player id";
const char *HS_REGISTERED_NAME = "registered name";
const char *HS_KEY             = "player key";
const char *HS_WW_ENABLED      = "ww hs enabled";

PlayerInfos::PlayerInfos()
{
    setGroup("players");

    // standard items
    addItem("name", new NameItem);
    Item *it = new Item((uint)0, i18n("Games Count"),Qt::AlignRight);
    addItem("nb games", it, true, true);
    it = Manager::createItem(Manager::MeanScoreDefault);
    addItem("mean score", it, true, true);
    it = Manager::createItem(Manager::BestScoreDefault);
    addItem("best score", it, true, true);
    addItem("date", new DateItem, true, true);
    it = new Item(QString::null, i18n("Comment"), Qt::AlignLeft);
    addItem("comment", it);

    // statistics items
    addItem("nb black marks", new Item((uint)0), true, true); // legacy
    addItem("nb lost games", new Item((uint)0), true, true);
    addItem("nb draw games", new Item((uint)0), true, true);
    addItem("current trend", new Item((int)0), true, true);
    addItem("max lost trend", new Item((uint)0), true, true);
    addItem("max won trend", new Item((uint)0), true, true);

#ifdef HIGHSCORE_DIRECTORY
    struct passwd *pwd = getpwuid(getuid());
    QString username = pwd->pw_name;
    internal->hsConfig().setHighscoreGroup("users");
    for (uint i=0; ;i++) {
        if ( !internal->hsConfig().hasEntry(i+1, "username") ) {
            _newPlayer = true;
            _id = i;
            break;
        }
        if ( internal->hsConfig().readEntry(i+1, "username")==username ) {
            _newPlayer = false;
            _id = i;
            return;
        }
    }
#else
    QString username;
#endif
    internal->hsConfig().lockForWriting();
	KEMailSettings emailConfig;
	emailConfig.setProfile(emailConfig.defaultProfileName());
	QString name = emailConfig.getSetting(KEMailSettings::RealName);
	if ( name.isEmpty() || isNameUsed(name) ) name = username;
	if ( isNameUsed(name) ) name= QString(ItemContainer::ANONYMOUS);
#ifdef HIGHSCORE_DIRECTORY
    internal->hsConfig().writeEntry(_id+1, "username", username);
    item("name")->write(_id, name);
#endif

    ConfigGroup cg;
    _oldLocalPlayer = cg.config()->hasKey(HS_ID);
    _oldLocalId = cg.config()->readUnsignedNumEntry(HS_ID);
#ifdef HIGHSCORE_DIRECTORY
    if (_oldLocalPlayer) { // player already exists in local config file
        // copy player data
        QString prefix = QString("%1_").arg(_oldLocalId+1);
        QMap<QString, QString> entries =
            cg.config()->entryMap("KHighscore_players");
        QMap<QString, QString>::const_iterator it;
        for (it=entries.begin(); it!=entries.end(); ++it) {
            QString key = it.key();
            if ( key.find(prefix)==0 ) {
                QString name = key.right(key.length()-prefix.length());
                if ( name!="name" || !isNameUsed(it.data()) )
                    internal->hsConfig().writeEntry(_id+1, name, it.data());
            }
        }
    }
#else
    _newPlayer = !_oldLocalPlayer;
    if (_oldLocalPlayer) _id = _oldLocalId;
    else {
        _id = nbEntries();
        cg.config()->writeEntry(HS_ID, _id);
        item("name")->write(_id, name);
    }
#endif
    internal->hsConfig().writeAndUnlock();
}

void PlayerInfos::createHistoItems(const QMemArray<uint> &scores, bool bound)
{
    Q_ASSERT( _histogram.size()==0 );
    _bound = bound;
    _histogram = scores;
    for (uint i=1; i<histoSize(); i++)
        addItem(histoName(i), new Item((uint)0), true, true);
}

bool PlayerInfos::isAnonymous() const
{
    return ( name()==ItemContainer::ANONYMOUS );
}

uint PlayerInfos::nbEntries() const
{
    internal->hsConfig().setHighscoreGroup("players");
    QStringList list = internal->hsConfig().readList("name", -1);
    return list.count();
}

QString PlayerInfos::key() const
{
    ConfigGroup cg;
    return cg.config()->readEntry(HS_KEY, QString::null);
}

bool PlayerInfos::isWWEnabled() const
{
    ConfigGroup cg;
    return cg.config()->readBoolEntry(HS_WW_ENABLED, false);
}

QString PlayerInfos::histoName(uint i) const
{
    const QMemArray<uint> &sh = _histogram;
    Q_ASSERT( i<sh.size() || (_bound || i==sh.size()) );
    if ( i==sh.size() )
        return QString("nb scores greater than %1").arg(sh[sh.size()-1]);
    return QString("nb scores less than %1").arg(sh[i]);
}

uint PlayerInfos::histoSize() const
{
     return _histogram.size() + (_bound ? 0 : 1);
}

void PlayerInfos::submitScore(const Score &score) const
{
    // update counts
    uint nbGames = item("nb games")->increment(_id);
    switch (score.type()) {
    case Lost:
        item("nb lost games")->increment(_id);
        break;
    case Won: break;
    case Draw:
        item("nb draw games")->increment(_id);
        break;
    };

    // update mean
    if ( score.type()==Won ) {
        uint nbWonGames = nbGames - item("nb lost games")->read(_id).toUInt()
                        - item("nb draw games")->read(_id).toUInt()
                        - item("nb black marks")->read(_id).toUInt(); // legacy
        double mean = (nbWonGames==1 ? 0.0
                       : item("mean score")->read(_id).toDouble());
        mean += (double(score.score()) - mean) / nbWonGames;
        item("mean score")->write(_id, mean);
    }

    // update best score
    Score best = score; // copy optionnal fields (there are not taken into account here)
    best.setScore( item("best score")->read(_id).toUInt() );
    if ( best<score ) {
        item("best score")->write(_id, score.score());
        item("date")->write(_id, score.data("date").toDateTime());
    }

    // update trends
    int current = item("current trend")->read(_id).toInt();
    switch (score.type()) {
    case Won: {
        if ( current<0 ) current = 0;
        current++;
        uint won = item("max won trend")->read(_id).toUInt();
        if ( (uint)current>won ) item("max won trend")->write(_id, current);
        break;
    }
    case Lost: {
        if ( current>0 ) current = 0;
        current--;
        uint lost = item("max lost trend")->read(_id).toUInt();
        uint clost = -current;
        if ( clost>lost ) item("max lost trend")->write(_id, clost);
        break;
    }
    case Draw:
        current = 0;
        break;
    }
    item("current trend")->write(_id, current);

    // update histogram
    if ( score.type()==Won ) {
        const QMemArray<uint> &sh = _histogram;
        for (uint i=1; i<histoSize(); i++)
            if ( i==sh.size() || score.score()<sh[i] ) {
                item(histoName(i))->increment(_id);
                break;
            }
    }
}

bool PlayerInfos::isNameUsed(const QString &newName) const
{
    if ( newName==name() ) return false; // own name...
    for (uint i=0; i<nbEntries(); i++)
        if ( newName.lower()==item("name")->read(i).toString().lower() ) return true;
    if ( newName==i18n(ItemContainer::ANONYMOUS_LABEL) ) return true;
    return false;
}

void PlayerInfos::modifyName(const QString &newName) const
{
    item("name")->write(_id, newName);
}

void PlayerInfos::modifySettings(const QString &newName,
                                 const QString &comment, bool WWEnabled,
                                 const QString &newKey) const
{
    modifyName(newName);
    item("comment")->write(_id, comment);
    ConfigGroup cg;
    cg.config()->writeEntry(HS_WW_ENABLED, WWEnabled);
    if ( !newKey.isEmpty() ) cg.config()->writeEntry(HS_KEY, newKey);
    if (WWEnabled) cg.config()->writeEntry(HS_REGISTERED_NAME, newName);
}

QString PlayerInfos::registeredName() const
{
    ConfigGroup cg;
    return cg.config()->readEntry(HS_REGISTERED_NAME, QString::null);
}

void PlayerInfos::removeKey()
{
    ConfigGroup cg;

    // save old key/nickname
    uint i = 0;
    QString str = "%1 old #%2";
    QString sk;
    do {
        i++;
        sk = str.arg(HS_KEY).arg(i);
    } while ( !cg.config()->readEntry(sk, QString::null).isEmpty() );
    cg.config()->writeEntry(sk, key());
    cg.config()->writeEntry(str.arg(HS_REGISTERED_NAME).arg(i),
                            registeredName());

    // clear current key/nickname
    cg.config()->deleteEntry(HS_KEY);
    cg.config()->deleteEntry(HS_REGISTERED_NAME);
    cg.config()->writeEntry(HS_WW_ENABLED, false);
}

//-----------------------------------------------------------------------------
ManagerPrivate::ManagerPrivate(uint nbGameTypes, Manager &m)
    : manager(m), showStatistics(false), showDrawGames(false),
      trackLostGames(false), trackDrawGames(false),
      showMode(Manager::ShowForHigherScore),
      _first(true), _nbGameTypes(nbGameTypes), _gameType(0)
{}

void ManagerPrivate::init(uint maxNbEntries)
{
    _hsConfig = new KHighscore(false, 0);
    _playerInfos = new PlayerInfos;
    _scoreInfos = new ScoreInfos(maxNbEntries, *_playerInfos);
}

ManagerPrivate::~ManagerPrivate()
{
    delete _scoreInfos;
    delete _playerInfos;
    delete _hsConfig;
}

KURL ManagerPrivate::queryURL(QueryType type, const QString &newName) const
{
    KURL url = serverURL;
    QString nameItem = "nickname";
    QString name = _playerInfos->registeredName();
    bool withVersion = true;
    bool key = false;
    bool level = false;

	switch (type) {
        case Submit:
            url.addPath("submit.php");
            level = true;
            key = true;
            break;
        case Register:
            url.addPath("register.php");
            name = newName;
            break;
        case Change:
            url.addPath("change.php");
            key = true;
            if ( newName!=name )
                Manager::addToQueryURL(url, "new_nickname", newName);
            break;
        case Players:
            url.addPath("players.php");
            nameItem = "highlight";
            withVersion = false;
            break;
        case Scores:
            url.addPath("highscores.php");
            withVersion = false;
            if ( _nbGameTypes>1 ) level = true;
            break;
	}

    if (withVersion) Manager::addToQueryURL(url, "version", version);
    if ( !name.isEmpty() ) Manager::addToQueryURL(url, nameItem, name);
    if (key) Manager::addToQueryURL(url, "key", _playerInfos->key());
    if (level) {
        QString label = manager.gameTypeLabel(_gameType, Manager::WW);
        if ( !label.isEmpty() ) Manager::addToQueryURL(url, "level", label);
    }

    return url;
}

// strings that needs to be translated (coming from the highscores server)
const char *DUMMY_STRINGS[] = {
    I18N_NOOP("Undefined error."),
    I18N_NOOP("Missing argument(s)."),
    I18N_NOOP("Invalid argument(s)."),

    I18N_NOOP("Unable to connect to MySQL server."),
    I18N_NOOP("Unable to select database."),
    I18N_NOOP("Error on database query."),
    I18N_NOOP("Error on database insert."),

    I18N_NOOP("Nickname already registered."),
    I18N_NOOP("Nickname not registered."),
    I18N_NOOP("Invalid key."),
    I18N_NOOP("Invalid submit key."),

    I18N_NOOP("Invalid level."),
    I18N_NOOP("Invalid score.")
};

const char *UNABLE_TO_CONTACT =
    I18N_NOOP("Unable to contact world-wide highscore server");

bool ManagerPrivate::doQuery(const KURL &url, QWidget *parent,
                                QDomNamedNodeMap *map)
{
    KIO::http_update_cache(url, true, 0); // remove cache !

    QString tmpFile;
    if ( !KIO::NetAccess::download(url, tmpFile, parent) ) {
        QString details = i18n("Server URL: %1").arg(url.host());
        KMessageBox::detailedSorry(parent, i18n(UNABLE_TO_CONTACT), details);
        return false;
    }

	QFile file(tmpFile);
	if ( !file.open(IO_ReadOnly) ) {
        KIO::NetAccess::removeTempFile(tmpFile);
        QString details = i18n("Unable to open temporary file.");
        KMessageBox::detailedSorry(parent, i18n(UNABLE_TO_CONTACT), details);
        return false;
    }

	QTextStream t(&file);
	QString content = t.read().stripWhiteSpace();
	file.close();
    KIO::NetAccess::removeTempFile(tmpFile);

	QDomDocument doc;
    if ( doc.setContent(content) ) {
        QDomElement root = doc.documentElement();
        QDomElement element = root.firstChild().toElement();
        if ( element.tagName()=="success" ) {
            if (map) *map = element.attributes();
            return true;
        }
        if ( element.tagName()=="error" ) {
            QDomAttr attr = element.attributes().namedItem("label").toAttr();
            if ( !attr.isNull() ) {
                QString msg = i18n(attr.value().latin1());
                QString caption = i18n("Message from world-wide highscores "
                                       "server");
                KMessageBox::sorry(parent, msg, caption);
                return false;
            }
        }
    }
    QString msg = i18n("Invalid answer from world-wide highscores server.");
    QString details = i18n("Raw message: %1").arg(content);
    KMessageBox::detailedSorry(parent, msg, details);
    return false;
}

bool ManagerPrivate::getFromQuery(const QDomNamedNodeMap &map,
                                  const QString &name, QString &value,
                                  QWidget *parent)
{
    QDomAttr attr = map.namedItem(name).toAttr();
    if ( attr.isNull() ) {
	    KMessageBox::sorry(parent,
               i18n("Invalid answer from world-wide "
                    "highscores server (missing item: %1).").arg(name));
		return false;
    }
    value = attr.value();
    return true;
}

Score ManagerPrivate::readScore(uint i) const
{
    Score score(Won);
    _scoreInfos->read(i, score);
    return score;
}

int ManagerPrivate::rank(const Score &score) const
{
    uint nb = _scoreInfos->nbEntries();
    uint i = 0;
	for (; i<nb; i++)
        if ( readScore(i)<score ) break;
	return (i<_scoreInfos->maxNbEntries() ? (int)i : -1);
}

bool ManagerPrivate::modifySettings(const QString &newName,
                                    const QString &comment, bool WWEnabled,
                                    QWidget *widget)
{
    QString newKey;
    bool newPlayer = false;

    if (WWEnabled) {
        newPlayer = _playerInfos->key().isEmpty()
                    || _playerInfos->registeredName().isEmpty();
        KURL url = queryURL((newPlayer ? Register : Change), newName);
        Manager::addToQueryURL(url, "comment", comment);

        QDomNamedNodeMap map;
        bool ok = doQuery(url, widget, &map);
        if ( !ok || (newPlayer && !getFromQuery(map, "key", newKey, widget)) )
            return false;
    }

    bool ok = _hsConfig->lockForWriting(widget); // no GUI when locking
    if (ok) {
        // check again name in case the config file has been changed...
        // if it has, it is unfortunate because the WWW name is already
        // committed but should be very rare and not really problematic
        ok = ( !_playerInfos->isNameUsed(newName) );
        if (ok)
            _playerInfos->modifySettings(newName, comment, WWEnabled, newKey);
        _hsConfig->writeAndUnlock();
    }
    return ok;
}

void ManagerPrivate::convertToGlobal()
{
    // read old highscores
    KHighscore *tmp = _hsConfig;
    _hsConfig = new KHighscore(true, 0);
    QValueVector<Score> scores(_scoreInfos->nbEntries());
    for (uint i=0; i<scores.count(); i++)
        scores[i] = readScore(i);

    // commit them
    delete _hsConfig;
    _hsConfig = tmp;
    _hsConfig->lockForWriting();
    for (uint i=0; i<scores.count(); i++)
        if ( scores[i].data("id").toUInt()==_playerInfos->oldLocalId()+1 )
            submitLocal(scores[i]);
    _hsConfig->writeAndUnlock();
}

void ManagerPrivate::setGameType(uint type)
{
    if (_first) {
        _first = false;
        if ( _playerInfos->isNewPlayer() ) {
            // convert legacy highscores
            for (uint i=0; i<_nbGameTypes; i++) {
                setGameType(i);
                manager.convertLegacy(i);
            }

#ifdef HIGHSCORE_DIRECTORY
            if ( _playerInfos->isOldLocalPlayer() ) {
                // convert local to global highscores
                for (uint i=0; i<_nbGameTypes; i++) {
                    setGameType(i);
                    convertToGlobal();
                }
            }
#endif
        }
    }

    Q_ASSERT( type<_nbGameTypes );
    _gameType = kMin(type, _nbGameTypes-1);
    QString str = "scores";
    QString lab = manager.gameTypeLabel(_gameType, Manager::Standard);
    if ( !lab.isEmpty() ) {
        _playerInfos->setSubGroup(lab);
        str += "_" + lab;
    }
    _scoreInfos->setGroup(str);
}

void ManagerPrivate::checkFirst()
{
    if (_first) setGameType(0);
}

int ManagerPrivate::submitScore(const Score &ascore,
                                QWidget *widget, bool askIfAnonymous)
{
    checkFirst();

    Score score = ascore;
    score.setData("id", _playerInfos->id() + 1);
    score.setData("date", QDateTime::currentDateTime());

    // ask new name if anonymous and winner
    const char *dontAskAgainName = "highscore_ask_name_dialog";
    QString newName;
    KMessageBox::ButtonCode dummy;
    if ( score.type()==Won && askIfAnonymous && _playerInfos->isAnonymous()
     && KMessageBox::shouldBeShownYesNo(dontAskAgainName, dummy) ) {
         AskNameDialog d(widget);
         if ( d.exec()==QDialog::Accepted ) newName = d.name();
         if ( d.dontAskAgain() )
             KMessageBox::saveDontShowAgainYesNo(dontAskAgainName,
                                                 KMessageBox::No);
    }

    int rank = -1;
    if ( _hsConfig->lockForWriting(widget) ) { // no GUI when locking
        // check again new name in case the config file has been changed...
        if ( !newName.isEmpty() && !_playerInfos->isNameUsed(newName) )
             _playerInfos->modifyName(newName);

        // commit locally
        _playerInfos->submitScore(score);
        if ( score.type()==Won ) rank = submitLocal(score);
        _hsConfig->writeAndUnlock();
    }

    if ( _playerInfos->isWWEnabled() )
        submitWorldWide(score, widget);

    return rank;
}

int ManagerPrivate::submitLocal(const Score &score)
{
    int r = rank(score);
    if ( r!=-1 ) {
        uint nb = _scoreInfos->nbEntries();
        if ( nb<_scoreInfos->maxNbEntries() ) nb++;
        _scoreInfos->write(r, score, nb);
    }
    return r;
}

bool ManagerPrivate::submitWorldWide(const Score &score,
                                     QWidget *widget) const
{
    if ( score.type()==Lost && !trackLostGames ) return true;
    if ( score.type()==Draw && !trackDrawGames ) return true;

    KURL url = queryURL(Submit);
    manager.additionalQueryItems(url, score);
    int s = (score.type()==Won ? score.score() : (int)score.type());
    QString str =  QString::number(s);
    Manager::addToQueryURL(url, "score", str);
    KMD5 context(QString(_playerInfos->registeredName() + str).latin1());
    Manager::addToQueryURL(url, "check", context.hexDigest());

    return doQuery(url, widget);
}

void ManagerPrivate::exportHighscores(QTextStream &s)
{
    uint tmp = _gameType;

    for (uint i=0; i<_nbGameTypes; i++) {
        setGameType(i);
        if ( _nbGameTypes>1 ) {
            if ( i!=0 ) s << endl;
            s << "--------------------------------" << endl;
            s << "Game type: "
              << manager.gameTypeLabel(_gameType, Manager::I18N)
              << endl;
            s << endl;
        }
        s << "Players list:" << endl;
        _playerInfos->exportToText(s);
        s << endl;
        s << "Highscores list:" << endl;
        _scoreInfos->exportToText(s);
    }

    setGameType(tmp);
}

} // namespace
