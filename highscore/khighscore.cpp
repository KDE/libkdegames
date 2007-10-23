/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2003 Nicolas Hadacek <hadacek@kde.org>

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

#include "khighscore.h"
#include <config-highscore.h>

#include <KConfig>
#include <KGlobal>
#include <KStandardGuiItem>
#include <KLocale>
#include <KMessageBox>
#include <KDebug>
//#include <K3StaticDeleter>
#include <KLockFile>
#include <KConfigGroup>

#include <QtCore/QFile>

#include <unistd.h> // sleep

#define GROUP "KHighscore"

class KHighscore::KHighscorePrivate
{
public:
    KHighscorePrivate() {}

    QString  group;
    bool     global;
};

class KHighscoreLockedConfig
{
public:
    ~KHighscoreLockedConfig();
    KLockFile *lock;
    KConfig *config;
};

KHighscoreLockedConfig::~KHighscoreLockedConfig()
{
    delete lock;
    delete config;
}

K_GLOBAL_STATIC(KHighscoreLockedConfig, lockedConfig)

KHighscore::KHighscore(bool forceLocal, QObject* parent)
    : QObject(parent), d(new KHighscorePrivate)
{
    init(forceLocal);
}

void KHighscore::init(bool forceLocal)
{
#ifdef HIGHSCORE_DIRECTORY
    d->global = !forceLocal;
    if ( d->global && lockedConfig->lock==0 )    //If we're doing global highscores but not KFileLock has been set up yet
        kFatal(11002) << "KHighscore::init should be called before!!";
#else
    d->global = false;
    Q_UNUSED(forceLocal);
#endif
    readCurrentConfig();
}

bool KHighscore::isLocked() const
{
    return (d->global ? lockedConfig->lock->isLocked() : true);
}

void KHighscore::readCurrentConfig()
{
    if ( d->global ) lockedConfig->config->reparseConfiguration();
}

void KHighscore::init(const char *appname)
{
#ifdef HIGHSCORE_DIRECTORY
    const QString filename =  QString::fromLocal8Bit("%1/%2.scores")
                              .arg(HIGHSCORE_DIRECTORY).arg(appname);
    //int fd = fopen(filename.toLocal8Bit(), O_RDWR);
    /*QFile file(filename);
    if ( !file.open(QIODevice::ReadWrite) ) kFatal(11002) << "cannot open global highscore file \""
                               << filename << "\"";*/
    /*if (!(QFile::permissions(filename) & QFile::WriteOwner)) kFatal(11002) << "cannot write to global highscore file \""
                << filename << "\"";*/
    kDebug() << "Global highscore file \"" << filename << "\"";
    lockedConfig->lock = new KLockFile(filename);
    lockedConfig->config = new KConfig(filename, KConfig::OpenFlags(KConfig::CascadeConfig | KConfig::SimpleConfig)); // read-only   (matt-?)

    // drop the effective gid
    #warning not portable yet. Unix only. Actually it does not even work there yet.
    int gid = getgid();
    setregid(gid, gid);
#else
    Q_UNUSED(appname);
#endif
}

bool KHighscore::lockForWriting(QWidget *widget)
{
    if ( isLocked() ) return true;

    bool first = true;
    for (;;) {
        kDebug(11002) << "try locking";
        // lock the highscore file (it should exist)
        int result = lockedConfig->lock->lock();
        bool ok = ( result==0 );
        kDebug(11002) << "locking system-wide highscore file res="
                       <<  result << " (ok=" << ok << ")";
        if (ok) {
            readCurrentConfig();
            return true;
        }

        if ( !first ) {
            KGuiItem item = KStandardGuiItem::cont();
            item.setText(i18n("Retry"));
            int res = KMessageBox::warningContinueCancel(widget, i18n("Cannot access the highscore file. Another user is probably currently writing to it."), QString(), item, KStandardGuiItem::cancel(), "ask_lock_global_highscore_file");
            if ( res==KMessageBox::Cancel ) break;
        } else sleep(1);
        first = false;
    }
    return false;
}

void KHighscore::writeAndUnlock()
{
    if ( !d->global ) {
        KGlobal::config()->sync();
        return;
    }
    if ( !isLocked() ) return;

    kDebug(11002) << "unlocking";
    lockedConfig->config->sync(); // write config
    lockedConfig->lock->unlock();
}

KHighscore::~KHighscore()
{
    writeAndUnlock();
    delete d;
}

KConfig* KHighscore::config() const
{
    return (d->global ? lockedConfig->config : static_cast<KConfig*>(KGlobal::config().data()));
}

void KHighscore::writeEntry(int entry, const QString& key, const QVariant& value)
{
    Q_ASSERT( isLocked() );
    KConfigGroup cg(config(), group());
    QString confKey = QString("%1_%2").arg(entry).arg(key);
    cg.writeEntry(confKey, value);
}

void KHighscore::writeEntry(int entry, const QString& key, int value)
{
    Q_ASSERT( isLocked() );
    KConfigGroup cg(config(), group());
    QString confKey = QString("%1_%2").arg(entry).arg(key);
    cg.writeEntry(confKey, value);
}

void KHighscore::writeEntry(int entry, const QString& key, const QString &value)
{
    Q_ASSERT (isLocked() );
    KConfigGroup cg(config(), group());
    QString confKey = QString("%1_%2").arg(entry).arg(key);
    cg.writeEntry(confKey, value);
}

QVariant KHighscore::readPropertyEntry(int entry, const QString& key, const QVariant& pDefault) const
{
    KConfigGroup cg(config(), group());
    QString confKey = QString("%1_%2").arg(entry).arg(key);
    return cg.readEntry(confKey, pDefault);
}

QString KHighscore::readEntry(int entry, const QString& key, const QString& pDefault) const
{
    KConfigGroup cg(config(), group());
    QString confKey = QString("%1_%2").arg(entry).arg(key);
    return cg.readEntry(confKey, pDefault);
}

int KHighscore::readNumEntry(int entry, const QString& key, int pDefault) const
{
    KConfigGroup cg(config(), group());
    QString confKey = QString("%1_%2").arg(entry).arg(key);
    return cg.readEntry(confKey, pDefault);
}

bool KHighscore::hasEntry(int entry, const QString& key) const
{
    KConfigGroup cg(config(), group());
    QString confKey = QString("%1_%2").arg(entry).arg(key);
    return cg.hasKey(confKey);
}

QStringList KHighscore::readList(const QString& key, int lastEntry) const
{
    QStringList list;
    for (int i = 1; hasEntry(i, key) && ((lastEntry > 0) ? (i <= lastEntry) : true); i++) {
        list.append(readEntry(i, key));
    }
    return list;
}

void KHighscore::writeList(const QString& key, const QStringList& list)
{
    for (int i = 1; i <= list.count(); i++) {
            writeEntry(i, key, list[i - 1]);
    }
}

void KHighscore::setHighscoreGroup(const QString& group)
{
    d->group = group;
}

QString KHighscore::highscoreGroup() const
{
    return d->group;
}

QStringList KHighscore::groupList() const
{
    QStringList groupList = config()->groupList();
    QStringList highscoreGroupList;
    foreach (QString group, groupList)
    {
        if(group.contains("KHighscore")) //If it's one of _our_ groups (KHighscore or KHighscore_xxx)
        {
            if(group == "KHighscore")
                group.replace("KHighscore", QString()); //Set to blank
            else
                group.replace("KHighscore_", QString()); //Remove the KHighscore_ prefix
            highscoreGroupList << group;
        }
    }

    return highscoreGroupList;
}

QString KHighscore::group() const
{
    if (highscoreGroup().isEmpty())
        return (d->global ? QString() : QString::fromLatin1(GROUP));
    return (d->global ?
            highscoreGroup() :
            QString("%1_%2").arg(GROUP).arg(highscoreGroup()));
}

bool KHighscore::hasTable() const
{
    return config()->hasGroup(group());
}

#include "khighscore.moc"
