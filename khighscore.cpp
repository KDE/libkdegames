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
};

KHighscore::KHighscore()
{
 d = new KHighscorePrivate;

}

KHighscore::~KHighscore()
{
// not necessary, as KConfig destructor should handle this
 config()->sync();
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
 if (entry < 1) {
	return;
 }
 // save the group in case that we are on kapp->config()
 QString group = config()->group();
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 config()->setGroup(GROUP);
 config()->writeEntry(confKey, value);
 config()->setGroup(group);
}


QString KHighscore::readEntry(int entry, const QString& key) const
{
 if (entry < 1) {
	return QString::null;
 }
 // save the group in case that we are on kapp->config()
 QString group = config()->group();
 QString confKey = QString("%1_%2").arg(entry).arg(key);
 config()->setGroup(GROUP);
 QString value = config()->readEntry(confKey);
 config()->setGroup(group);
 return value;
}


QStringList KHighscore::readList(const QString& key, int lastEntry) const
{
 QStringList list;
 for (int i = 1; i <= lastEntry; i++) {
	QString e = readEntry(i, key);
	if (e != QString::null) {
		list.append(e);
	} else {
		// abort reading
		i = lastEntry + 1;
	}
 }
 return list;
}

void KHighscore::writeList(const QString& key, const QStringList& list)
{
 for (int i = 0; i < list.count(); i++) {
	writeEntry(i + 1, key, list[i]);
 }
}

