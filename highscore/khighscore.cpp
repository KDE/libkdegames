/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

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
/*
    $Id$
*/

#include <kapplication.h>
#include <ksimpleconfig.h>
#include <kglobal.h>

#include "khighscore.h"
#include "config.h" // HIGHSCORE_DIRECTORY is defined here (or not)

#define GROUP "KHighscore"

class KHighscorePrivate
{
public:
	KHighscorePrivate()
	{
		mConfig = 0;
	}

	KConfig* mConfig;
	QString group;
};

KHighscore::KHighscore(QObject* parent) : QObject(parent)
{
 d = new KHighscorePrivate;

}

KHighscore::~KHighscore()
{
// not necessary, as KConfig destructor should handle this
 sync();
 if (d->mConfig) {
	delete d->mConfig;
 }
 delete d;
}

KConfig* KHighscore::config() const
{
 #ifdef HIGHSCORE_DIRECTORY
	if (!d->mConfig) {
		//AB: is instanceName() correct? MUST be the same for all
		//processes of the game!
		QString file=QString::fromLocal8Bit("%1/%2").arg(HIGHSCORE_DIRECTORY).arg(KGlobal::instance()->instanceName());
		d->mConfig = new KSimpleConfig(file);
	}
	return d->mConfig;
 #else
	return kapp->config();
 #endif
}

void KHighscore::writeEntry(int entry, const QString& key, const QVariant& value)
{
 // save the group in case that we are on kapp->config()
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 cg.config()->writeEntry(confKey, value);
}

void KHighscore::writeEntry(int entry, const QString& key, int value)
{
 // save the group in case that we are on kapp->config()
 KConfigGroupSaver cg(config(), group());
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 cg.config()->writeEntry(confKey, value);
}

void KHighscore::writeEntry(int entry, const QString& key, const QString &value)
{
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
 if (highscoreGroup().isNull()) {
	return GROUP;
 } else {
	return QString("%1_%2").arg(GROUP).arg(highscoreGroup());
 }
}

bool KHighscore::hasTable() const
{ return config()->hasGroup(group()); }

void KHighscore::sync()
{ config()->sync(); }

#include "khighscore.moc"
