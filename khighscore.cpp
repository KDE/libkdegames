/* **************************************************************************
                           KHighscore Class
                           -------------------
    begin                : 23 April 2001
    copyright            : (C) 2001 by Andreas Beckermann
    email                : b_mann@gmx.de
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Additional license: Any of the above copyright holders can add an     *
 *   enhanced license which complies with the license of the KDE core      *
 *   libraries so that this file resp. this library is compatible with     *
 *   the KDE core libraries.                                               *
 *   The user of this program shall have the choice which license to use   *
 *                                                                         *
 ***************************************************************************/
/*
    $Id$
*/

#include <kapp.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
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
		//AB: is fromLatin1 correct? Better fromLocal8Bit? don't know...
		//AB: is instanceName() correct? MUST be the same for all
		//processes of the game!
		QString file=QString::fromLatin1("%1/%2").arg(HIGHSCORE_DIRECTORY).arg(KGlobal::instance()->instanceName());
		d->mConfig = new KSimpleConfig(file);
	}
	return d->mConfig;
 #else
	return kapp->config();
 #endif
}

void KHighscore::writeEntry(int entry, const QString& key, const QString& value)
{
 // save the group in case that we are on kapp->config()
 QString origGroup = config()->group();
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 config()->setGroup(group());
 config()->writeEntry(confKey, value);
 config()->setGroup(origGroup);
}

void KHighscore::writeEntry(int entry, const QString& key, int value)
{ writeEntry(entry, key, QString::number(value)); }

QString KHighscore::readEntry(int entry, const QString& key, const QString& pDefault) const
{
 // save the group in case that we are on kapp->config()
 QString origGroup = config()->group();
 config()->setGroup(group());
 QString value;
 if (hasEntry(entry, key)) {
	QString confKey = QString("%1_%2").arg(entry).arg(key);
	value = config()->readEntry(confKey);
 } else {
	value = pDefault;
 }
 config()->setGroup(origGroup);
 return value;
}

int KHighscore::readNumEntry(int entry, const QString& key, int pDefault) const
{
 QString v = readEntry(entry, key, QString::number(pDefault));
 bool ok;
 int value = v.toInt(&ok);
 return ok ? value : pDefault;
}

bool KHighscore::hasEntry(int entry, const QString& key) const
{
 QString origGroup = config()->group();
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 config()->setGroup(group());
 bool value = config()->hasKey(confKey);
 config()->setGroup(origGroup);
 return value;
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
 if (highscoreGroup() == QString::null) {
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
