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
/*
    $Id$
*/

#include <config.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

#include <kapplication.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kstdguiitem.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include "khighscore.h"
#include "kconfigrawbackend.h"
#include "kfilelock.h"

#define GROUP "KHighscore"

class KHighscorePrivate
{
public:
    KHighscorePrivate() {}

    QString  group;
    bool     global;
};

KFileLock *KHighscore::_lock = 0;
KRawConfig *KHighscore::_config = 0;
static KStaticDeleter<KFileLock> lockSD;
static KStaticDeleter<KRawConfig> configSD;


KHighscore::KHighscore(QObject* parent)
    : QObject(parent)
{
    init(true);
}

KHighscore::KHighscore(bool forceLocal, QObject* parent)
    : QObject(parent)
{
    init(forceLocal);
}

void KHighscore::init(bool forceLocal)
{
    d = new KHighscorePrivate;
#ifdef HIGHSCORE_DIRECTORY
    d->global = !forceLocal;
    if ( d->global && _lock==0 )
        kdFatal(11002) << "KHighscore::init should be called before!!" << endl;
#else
    d->global = false;
    Q_UNUSED(forceLocal);
#endif
    readCurrentConfig();
}

bool KHighscore::isLocked() const
{
    return (d->global ? _lock->isLocked() : true);
}

void KHighscore::readCurrentConfig()
{
    if ( d->global ) _config->reparseConfiguration();
}

void KHighscore::init(const char *appname)
{
#ifdef HIGHSCORE_DIRECTORY
    const QString filename =  QString::fromLocal8Bit("%1/%2.scores")
                              .arg(HIGHSCORE_DIRECTORY).arg(appname);
    int fd = open(filename.local8Bit(), O_RDWR);
    if ( fd<0 ) kdFatal(11002) << "cannot open global highscore file \""
                               << filename << "\"" << endl;
    lockSD.setObject(_lock, new KFileLock(fd));
    configSD.setObject(_config, new KRawConfig(fd, true)); // read-only

    // drop the effective gid
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
        kdDebug(11002) << "try locking" << endl;
        // lock the highscore file (it should exist)
        int result = _lock->lock();
        bool ok = ( result==0 );
        kdDebug(11002) << "locking system-wide highscore file res="
                       <<  result << " (ok=" << ok << ")" << endl;
        if (ok) {
            readCurrentConfig();
            _config->setReadOnly(false);
            return true;
        }

        if ( !first ) {
            KGuiItem item = KStdGuiItem::cont();
            item.setText(i18n("Retry"));
            int res = KMessageBox::warningContinueCancel(widget, i18n("Cannot access the highscore file. Another user is probably currently writing to it."), QString::null, item, "ask_lock_global_highscore_file");
            if ( res==KMessageBox::Cancel ) break;
        } else sleep(1);
        first = false;
    }
    return false;
}

void KHighscore::writeAndUnlock()
{
    if ( !d->global ) {
        kapp->config()->sync();
        return;
    }
    if ( !isLocked() ) return;

    kdDebug(11002) << "unlocking" << endl;
    _config->sync(); // write config
    _lock->unlock();
    _config->setReadOnly(true);
}

KHighscore::~KHighscore()
{
    writeAndUnlock();
    delete d;
}

KConfig* KHighscore::config() const
{
    return (d->global ? _config : kapp->config());
}

void KHighscore::writeEntry(int entry, const QString& key, const QVariant& value)
{
 Q_ASSERT( isLocked() );
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 cg.config()->writeEntry(confKey, value);
}

void KHighscore::writeEntry(int entry, const QString& key, int value)
{
 Q_ASSERT( isLocked() );
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 cg.config()->writeEntry(confKey, value);
}

void KHighscore::writeEntry(int entry, const QString& key, const QString &value)
{
 Q_ASSERT (isLocked() );
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 cg.config()->writeEntry(confKey, value);
}

QVariant KHighscore::readPropertyEntry(int entry, const QString& key, const QVariant& pDefault) const
{
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 return cg.config()->readPropertyEntry(confKey, pDefault);
}

QString KHighscore::readEntry(int entry, const QString& key, const QString& pDefault) const
{
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 return cg.config()->readEntry(confKey, pDefault);
}

int KHighscore::readNumEntry(int entry, const QString& key, int pDefault) const
{
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 return cg.config()->readNumEntry(confKey, pDefault);
}

bool KHighscore::hasEntry(int entry, const QString& key) const
{
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 return cg.config()->hasKey(confKey);
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
 for (int unsigned i = 1; i <= list.count(); i++) {
	writeEntry(i, key, list[i - 1]);
 }
}

void KHighscore::setHighscoreGroup(const QString& group)
{
 d->group = group;
}

const QString& KHighscore::highscoreGroup() const
{
 return d->group;
}

QString KHighscore::group() const
{
    if ( highscoreGroup().isNull() )
        return (d->global ? QString::null : GROUP);
    return (d->global ? highscoreGroup()
            : QString("%1_%2").arg(GROUP).arg(highscoreGroup()));
}

bool KHighscore::hasTable() const
{ return config()->hasGroup(group()); }

void KHighscore::sync()
{
    writeAndUnlock();
}

#include "khighscore.moc"
