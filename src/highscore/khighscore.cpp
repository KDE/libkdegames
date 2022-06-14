/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2003 Nicolas Hadacek <hadacek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "khighscore.h"

// own
#include <config-highscore.h>
// KF
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardGuiItem>
// Qt
#include <QFile>
#include <QGlobalStatic>
#include <QLockFile>

#ifndef WIN32
#include <unistd.h> // sleep
#else
#include <qt_windows.h>
#endif

#define GROUP "KHighscore"

Q_LOGGING_CATEGORY(GAMES_HIGHSCORE, "org.kde.games.highscore", QtWarningMsg)

class KHighscorePrivate
{
public:
    KHighscorePrivate()
    {
    }

    QString group;
    bool global;
};

class KHighscoreLockedConfig
{
public:
    ~KHighscoreLockedConfig();
    QLockFile *lock;
    KConfig *config;
};

KHighscoreLockedConfig::~KHighscoreLockedConfig()
{
    delete lock;
    delete config;
}

Q_GLOBAL_STATIC(KHighscoreLockedConfig, lockedConfig)

KHighscore::KHighscore(bool forceLocal, QObject *parent)
    : QObject(parent)
    , d(new KHighscorePrivate)
{
    init(forceLocal);
}

void KHighscore::init(bool forceLocal)
{
#ifdef HIGHSCORE_DIRECTORY
    d->global = !forceLocal;
    if (d->global && lockedConfig->lock == 0) // If we're doing global highscores but not KFileLock has been set up yet
    {
        qCWarning(GAMES_HIGHSCORE) << "KHighscore::init should be called before!!";
        abort();
    }
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
    if (d->global)
        lockedConfig->config->reparseConfiguration();
}

void KHighscore::init(const char *appname)
{
#ifdef HIGHSCORE_DIRECTORY
    const QString filename = QString::fromLocal8Bit("%1/%2.scores").arg(HIGHSCORE_DIRECTORY).arg(appname);

    // int fd = fopen(filename.toLocal8Bit(), O_RDWR);
    /*QFile file(filename);
    if ( !file.open(QIODevice::ReadWrite) )
    {
      qCWarning(GAMES_HIGHSCORE) << "cannot open global highscore file \""
                               << filename << "\"";
      abort();
    }*/

    /*if (!(QFile::permissions(filename) & QFile::WriteOwner))
      {
    qCWarning(GAMES_HIGHSCORE) << "cannot write to global highscore file \""
                << filename << "\"";
    abort();
      }*/

    qCDebug(GAMES_HIGHSCORE) << "Global highscore file \"" << filename << "\"";
    lockedConfig->lock = new QLockFile(filename);
    lockedConfig->config = new KConfig(filename, KConfig::NoGlobals); // read-only   (matt-?)

    // drop the effective gid
#ifdef __GNUC__
#warning not portable yet. Unix only. Actually it does not even work there yet.
#endif
    int gid = getgid();
    setregid(gid, gid);
#else
    Q_UNUSED(appname);
#endif
}

bool KHighscore::lockForWriting(QWidget *widget)
{
    if (isLocked())
        return true;

    bool first = true;
    for (;;) {
        qCDebug(GAMES_HIGHSCORE) << "try locking";
        // lock the highscore file (it should exist)
        int result = lockedConfig->lock->lock();
        bool ok = (result == 0);
        qCDebug(GAMES_HIGHSCORE) << "locking system-wide highscore file res=" << result << " (ok=" << ok << ")";
        if (ok) {
            readCurrentConfig();
            return true;
        }

        if (!first) {
            KGuiItem item = KStandardGuiItem::cont();
            item.setText(i18n("Retry"));
            int res = KMessageBox::warningContinueCancel(widget,
                                                         i18n("Cannot access the highscore file. Another user is probably currently writing to it."),
                                                         QString(),
                                                         item,
                                                         KStandardGuiItem::cancel(),
                                                         QStringLiteral("ask_lock_global_highscore_file"));
            if (res == KMessageBox::Cancel)
                break;
        } else {
#ifdef WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
        }
        first = false;
    }
    return false;
}

void KHighscore::writeAndUnlock()
{
    if (!d->global) {
        KSharedConfig::openConfig()->sync();
        return;
    }
    if (!isLocked())
        return;

    qCDebug(GAMES_HIGHSCORE) << "unlocking";
    lockedConfig->config->sync(); // write config
    lockedConfig->lock->unlock();
}

KHighscore::~KHighscore()
{
    writeAndUnlock();
}

KConfig *KHighscore::config() const
{
    return (d->global ? lockedConfig->config : static_cast<KConfig *>(KSharedConfig::openConfig().data()));
}

void KHighscore::writeEntry(int entry, const QString &key, const QVariant &value)
{
    Q_ASSERT(isLocked());
    KConfigGroup cg(config(), group());
    QString confKey = QStringLiteral("%1_%2").arg(entry).arg(key);
    cg.writeEntry(confKey, value);
}

void KHighscore::writeEntry(int entry, const QString &key, int value)
{
    Q_ASSERT(isLocked());
    KConfigGroup cg(config(), group());
    QString confKey = QStringLiteral("%1_%2").arg(entry).arg(key);
    cg.writeEntry(confKey, value);
}

void KHighscore::writeEntry(int entry, const QString &key, const QString &value)
{
    Q_ASSERT(isLocked());
    KConfigGroup cg(config(), group());
    QString confKey = QStringLiteral("%1_%2").arg(entry).arg(key);
    cg.writeEntry(confKey, value);
}

QVariant KHighscore::readPropertyEntry(int entry, const QString &key, const QVariant &pDefault) const
{
    KConfigGroup cg(config(), group());
    QString confKey = QStringLiteral("%1_%2").arg(entry).arg(key);
    return cg.readEntry(confKey, pDefault);
}

QString KHighscore::readEntry(int entry, const QString &key, const QString &pDefault) const
{
    KConfigGroup cg(config(), group());
    QString confKey = QStringLiteral("%1_%2").arg(entry).arg(key);
    return cg.readEntry(confKey, pDefault);
}

int KHighscore::readNumEntry(int entry, const QString &key, int pDefault) const
{
    KConfigGroup cg(config(), group());
    QString confKey = QStringLiteral("%1_%2").arg(entry).arg(key);
    return cg.readEntry(confKey, pDefault);
}

bool KHighscore::hasEntry(int entry, const QString &key) const
{
    KConfigGroup cg(config(), group());
    QString confKey = QStringLiteral("%1_%2").arg(entry).arg(key);
    return cg.hasKey(confKey);
}

QStringList KHighscore::readList(const QString &key, int lastEntry) const
{
    QStringList list;
    for (int i = 1; hasEntry(i, key) && ((lastEntry > 0) ? (i <= lastEntry) : true); i++) {
        list.append(readEntry(i, key));
    }
    return list;
}

void KHighscore::writeList(const QString &key, const QStringList &list)
{
    for (int i = 1; i <= list.count(); i++) {
        writeEntry(i, key, list[i - 1]);
    }
}

void KHighscore::setHighscoreGroup(const QString &group)
{
    d->group = group;
}

QString KHighscore::highscoreGroup() const
{
    return d->group;
}

QStringList KHighscore::groupList() const
{
    const QStringList groupList = config()->groupList();
    QStringList highscoreGroupList;
    for (QString group : groupList) {
        if (group.contains(QLatin1String("KHighscore"))) // If it's one of _our_ groups (KHighscore or KHighscore_xxx)
        {
            if (group == QLatin1String("KHighscore"))
                group.remove(QStringLiteral("KHighscore")); // Set to blank
            else
                group.remove(QStringLiteral("KHighscore_")); // Remove the KHighscore_ prefix
            highscoreGroupList << group;
        }
    }

    return highscoreGroupList;
}

QString KHighscore::group() const
{
    if (highscoreGroup().isEmpty())
        return (d->global ? QString() : QStringLiteral(GROUP));
    return (d->global ? highscoreGroup() : QStringLiteral("%1_%2").arg(QStringLiteral(GROUP), highscoreGroup()));
}

bool KHighscore::hasTable() const
{
    return config()->hasGroup(group());
}
