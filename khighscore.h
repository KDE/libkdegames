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
#ifndef __KHIGHSCORE_H__
#define __KHIGHSCORE_H__

#include <qstring.h>
#include <qobject.h>

class KConfig;

class KHighscorePrivate;
/**
 * This is the KDE class for saving and reading highscore tables. It offers the
 * possibility for system-wide highscore tables (configure with e.g.
 * --enable-highscore=/var/games) and a theoretically unlimited number of
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
 * You can also specify different groups using @ref setHighscoreGroup. Just like
 * the keys mentioned above the groups behave like groups in @ref KConfig but
 * are prefixed with "KHighscore_". The default group is just "KHighscore". You
 * might use this e.g. to create different highscore tables like
 * <pre>
 * table->setHighscoreGroup("Easy");
 * // write the highscores for level "easy" to the table
 * writeEasyHighscores(table);
 * 
 * table->setHighscore("Player_1");
 * // write player specific highscores to the table
 * writePlayerHighscores(table);
 * </pre>
 * As you can see above you can also use this to write the highscores of a
 * single player, so the "best times" of a player. To write highscores for a
 * specific player in a specific level you will have to use a more complex way:
 * <pre>
 * QString group = QString("%1_%2").arg(player).arg(level);
 * table->setGroup(group);
 * writeHighscore(table, player, level);
 * </pre>
 * 
 * Also note that yout MUST NOT mark the key or the group for translation! I.e. don't use
 * i18n() for the keys or groups! Here is the code to read the above written entry:
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
	 * existing one. The first entry of the list is the first placing, the
	 * last on is the last placing.
	 * @return A list of the entries of this key. You could also calls
	 * @readEntry(i, key) where i is from 1 to 20. Note that this function
	 * depends on "1" as the first entry!
	 * @param key The key of the entry. E.g. "name" for the name of the
	 * player. Nearly the same as the usual keys in @ref KConfig - but they
	 * are prefixed with the entry number
	 * @param lastEntry the last entry which will be includes into the list.
	 * 1 will include a list with maximal 1 entry - 20 a list with maximal
	 * 20 entries. If lastEntry is <= 0 then rading is only stopped when when an
	 * entry does not exist.
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

	/**
	 * Set the new highscore group. The group is being prefixed with
	 * "KHighscore_" in the table.
	 * @param groupname The new groupname. E.g. use "easy" for the easy 
	 * level of your game. If you use QString::null (the default) the
	 * default group is used.
	 **/
	void setHighscoreGroup(const QString& groupname = QString::null);

	/**
	 * @return The currently used group. This doesn't contain the prefix
	 * ("KHighscore_") but the same as @ref setHighscoreGroup uses. The default is
	 * QString::null
	 **/
	const QString& highscoreGroup() const;

protected:
	/**
	 * @return A groupname to be used in @ref KConfig. Used internally to
	 * prefix the value from @ref highscoreGroup() with "KHighscore_"
	 **/
	QString group() const;
		
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
