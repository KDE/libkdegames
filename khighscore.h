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
#ifndef __KHIGHSCORE_H__
#define __KHIGHSCORE_H__

#include <qstring.h>
#include <qobject.h>

class KConfig;

class KHighscorePrivate;
/**
 * This is the KDE class for saving and reading highscore tables. It offers the
 * possibility for system-wide highscore tables (configure with e.g.
 * --enable-highscore=/var/games) and a theoretically unlimited number of of
 * entries. 
 *
 * You can specify different "keys" for an entry - just like the @ref KConfig
 * keys. But it will be prefixed with the number of the entry. For example you
 * will probably use something like this to save the name of the player on the
 * top of the list (ie the winner):
 * <pre>
 * highscore->writeEntry(1, "name", myPlayer->name());
 * </pre>
 * Note that it doesn't really matter if you use "0" or "1" as the first entry
 * of the list as long as your program always uses the same for the first entry.
 * I recommend to use "1", as several convenience methods use this.
 * 
 * Also note that yout MUST NOT mark the key for translation! I.e. don't use
 * i18n() for the keys! Here is the code to read the above written entry:
 * <pre>
 * QString firstName = highscore->readEntry(0, "name");
 * </pre>
 * Easy, what?
 * @short Class for managing highscore tables
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/ 
class KHighscore : public QObject
{
	Q_OBJECT
public:
	KHighscore(QObject* parent = 0);
	~KHighscore();

	/**
	 * @param entry The number of the entry / the placing of the player
	 * @param key A key for this entry. E.g. "name" for the name of the
	 * player. Nearly the same as the usual keys in @ref KConfig - but they
	 * are prefixed with the entry number
	 * @param value The value of this entry
	 **/
	void writeEntry(int entry, const QString& key, const QString& value);

	/**
	 * This is an overloaded member function, provided for convinience.
	 * It differs from the above function only in what argument(s) it accepts.
	 **/
	void writeEntry(int entry, const QString& key, int value);

	/**
	 * Reads an entry from the highscore table.
	 * @param entry The number of the entry / the placing to be read
	 * @param key The key of the entry. E.g. "name" for the name of the
	 * player. Nearly the same as the usual keys in @ref KConfig - but they
	 * are prefixed with the entry number
	 * @param pDefault This will be used as default value if the key+pair
	 * entry can't be found.
	 * @return The value of this entry+key pair or pDefault if the entry+key
	 * pair doesn't exist
	 **/
	QString readEntry(int entry, const QString& key, const QString& pDefault = QString::null) const;

	/**
	 * Read a numeric value.
	 * @param entry The number of the entry / the placing to be read
	 * @param key The key of the entry. E.g. "name" for the name of the
	 * player. Nearly the same as the usual keys in @ref KConfig - but they
	 * are prefixed with the entry number
	 * @param pDefault This will be used as default value if the key+pair
	 * entry can't be found.
	 * @return The value of this entry+key pair or pDefault if the entry+key
	 * pair doesn't exist
	 **/
	int readNumEntry(int entry, const QString& key, int pDefault = -1) const;

	/**
	 * @return True if the highscore table conatins the entry/key pair,
	 * otherwise false
	 **/
	bool hasEntry(int entry, const QString& key) const;

	/**
	 * Reads a list of entries from the highscore table starting at 1 until
	 * lastEntry. If an entry between those numbers doesn't exist the
	 * function aborts reading even if after the missing entry is an
	 * existing one.
	 * @return A list of the entries of this key. You could also calls
	 * @readEntry(i, key) where i is from 1 to 20. Note that this function
	 * depends on "1" as the first entry!
	 * @param key The key of the entry. E.g. "name" for the name of the
	 * player. Nearly the same as the usual keys in @ref KConfig - but they
	 * are prefixed with the entry number
	 * @param lastEntry the last entry which will be includes into the list.
	 * 1 will include a list with maximal 1 entry - 20 a list with maximal
	 * 20 entries.
	 **/
	QStringList readList(const QString& key, int lastEntry = 20) const;

	/**
	 * Writes a list of entries to the highscore table.
	 * 
	 * The first entry is prefixed with "1". Using this method is a short
	 * way of calling writeEntry(i, key, list[i]) from i = 1 to
	 * list.count()
	 * @param key A key for the entry. E.g. "name" for the name of the
	 * player. Nearly the same as the usual keys in @ref KConfig - but they
	 * are prefixed with the entry number
	 * @param list The list of values
	 **/
	void writeList(const QString& key, const QStringList& list);

	/**
	 * @return Whether a highscore table exists. You can use this
	 * function to indicate whether KHighscore created a highscore table
	 * before and - if not - read your old (non-KHighscore) table instead.
	 * This way you can safely read an old table and save it using
	 * KHighscore without losing any data
	 **/
	bool hasTable() const;

	void sync();

protected:
	/**
	 * @return A pointer to the @ref KConfig object to be used. This is
	 * either kapp->config() (default) or a @ref KSimpleConfig object (if
	 * you configured with --enable-highscore)
	 **/
	KConfig* config() const;

private:
	KHighscorePrivate* d;
};

#endif
