/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2003 Nicolas Hadacek <hadacek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "khighscore.h"

// own
#include <config-highscore.h>
#include <kdegames_highscore_logging.h>
// KF
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardGuiItem>
// Qt
#include <QFile>
#include <QLockFile>

#ifndef WIN32
#include <unistd.h> // sleep
#else
#include <qt_windows.h>
#endif

#define GROUP "KHighscore"

class KHighscorePrivate
{
public:
    KHighscorePrivate() = default;

public:
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
    , d_ptr(new KHighscorePrivate)
{
    init(forceLocal);
}

void KHighscore::init(bool forceLocal)
{
    Q_D(KHighscore);

#ifdef HIGHSCORE_DIRECTORY
    d->global = !forceLocal;
    if (d->global && lockedConfig->lock == 0) // If we're doing global highscores but not KFileLock has been set up yet
    {
        qCWarning(KDEGAMES_HIGHSCORE_LOG) << "KHighscore::init should be called before!!";
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
    Q_D(const KHighscore);

    return (d->global ? lockedConfig->lock->isLocked() : true);
}

void KHighscore::readCurrentConfig()
{
    Q_D(KHighscore);

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
      qCWarning(KDEGAMES_HIGHSCORE_LOG) << "cannot open global highscore file \""
                               << filename << "\"";
      abort();
    }*/

    /*if (!(QFile::permissions(filename) & QFile::WriteOwner))
      {
    qCWarning(KDEGAMES_HIGHSCORE_LOG) << "cannot write to global highscore file \""
                << filename << "\"";
    abort();
      }*/

    qCDebug(KDEGAMES_HIGHSCORE_LOG) << "Global highscore file \"" << filename << "\"";
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
        qCDebug(KDEGAMES_HIGHSCORE_LOG) << "try locking";
        // lock the highscore file (it should exist)
        int result = lockedConfig->lock->lock();
        bool ok = (result == 0);
        qCDebug(KDEGAMES_HIGHSCORE_LOG) << "locking system-wide highscore file res=" << result << " (ok=" << ok << ")";
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
    Q_D(KHighscore);

    if (!d->global) {
        KSharedConfig::openConfig()->sync();
        return;
    }
    if (!isLocked())
        return;

    qCDebug(KDEGAMES_HIGHSCORE_LOG) << "unlocking";
    lockedConfig->config->sync(); // write config
    lockedConfig->lock->unlock();
}

KHighscore::~KHighscore()
{
    writeAndUnlock();
}

KConfig *KHighscore::config() const
{
    Q_D(const KHighscore);

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
    if (lastEntry > 0) {
        list.reserve(lastEntry);
    }
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
    Q_D(KHighscore);

    d->group = group;
}

QString KHighscore::highscoreGroup() const
{
    Q_D(const KHighscore);

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
    Q_D(const KHighscore);

    if (highscoreGroup().isEmpty())
        return (d->global ? QString() : QStringLiteral(GROUP));
    return (d->global ? highscoreGroup() : QStringLiteral("%1_%2").arg(QStringLiteral(GROUP), highscoreGroup()));
}

bool KHighscore::hasTable() const
{
    return config()->hasGroup(group());
}

#include "moc_khighscore.cpp"
