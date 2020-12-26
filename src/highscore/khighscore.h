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

#ifndef KHIGHSCORE_H
#define KHIGHSCORE_H

// own
#include <libkdegames_export.h>
// Qt
#include <QObject>
#include <QLoggingCategory>

class KConfig;

/**
 * \class KHighscore khighscore.h <KHighscore>
 *
 * @short Class for managing highscore tables
 *
 * This is the KDE class for saving and reading highscore tables. It offers the
 * possibility for system-wide highscore tables (cmake with e.g.
 * -DHIGHSCORE_DIRECTORY=/var/games) and a theoretically unlimited number of
 * entries.
 *
 * You can specify different "keys" for an entry - just like the KConfig
 * keys. But it will be prefixed with the number of the entry. For example you
 * will probably use something like this to save the name of the player on the
 * top of the list (ie the winner):
 * \code
 * highscore->writeEntry(1, "name", myPlayer->name());
 * \endcode
 * Note that it does not really matter if you use "0" or "1" as the first entry
 * of the list as long as your program always uses the same for the first
 * entry. I recommend to use "1", as several convenience methods use this.
 *
 * You can also specify different groups using setHighscoreGroup. Just
 * like the keys mentioned above the groups behave like groups in KConfig
 * but are prefixed with "KHighscore_". The default group is just "KHighscore".
 * You might use this e.g. to create different highscore tables like
 * \code
 * table->setHighscoreGroup("Easy");
 * // write the highscores for level "easy" to the table
 * writeEasyHighscores(table);
 *
 * table->setHighscore("Player_1");
 * // write player specific highscores to the table
 * writePlayerHighscores(table);
 * \endcode
 * As you can see above you can also use this to write the highscores of a
 * single player, so the "best times" of a player. To write highscores for a
 * specific player in a specific level you will have to use a more complex way:
 * \code
 * QString group = QStringLiteral("%1_%2").arg(player).arg(level);
 * table->setGroup(group);
 * writeHighscore(table, player, level);
 * \endcode
 *
 * Also note that you MUST NOT mark the key or the group for translation! I.e.
 * don't use i18n() for the keys or groups! Here is the code to read the above
 * written entry:
 * \code
 * QString firstName = highscore->readEntry(0, "name");
 * \endcode
 * Easy, what?
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KDEGAMES_EXPORT KHighscore : public QObject
{
    Q_OBJECT
    public:
        /**
        * Constructor.
        *
        * @param forceLocal if true, the local highscore file is used even
        * when the configuration has been set to use a system-wide file. This
        * is convenient for converting highscores from legacy applications.
        * @param parent parent widget for this widget
        */
        explicit KHighscore(bool forceLocal=true, QObject *parent=nullptr);

        /**
         * Read the current state of the highscore file. Remember that when
         * it's not locked for writing, this file can change at any time.
         * (This method is only useful for a system-wide highscore file).
         */
        void readCurrentConfig();

        /**
         * This method open the system-wide highscore file using the effective
         * group id of the game executable (which should be "games"). The
         * effective group id is completely dropped afterwards.
         *
         * Note: this method should be called in main() before creating a
         * KApplication and doing anything else (KApplication checks that the
         * program is not suid/sgid and will exit the program for security
         * reason if it is the case).
         */
        static void init(const char *appname);

        /**
         * Lock the system-wide highscore file for writing (does nothing and
         * return true if the local file is used).
         * You should perform writing without GUI interaction to avoid
         * blocking and don't forget to unlock the file as soon as possible
         * with writeAndUnlock().
         *
         * If the config file cannot be locked,
         * the method waits for 1 second and, if it failed again, displays
         * a message box asking for retry or cancel.
         * @param widget used as the parent of the message box.
         *
         * @return false on error or if the config file is locked by another
         * process. In such case, the config stays read-only.
         */
        bool lockForWriting(QWidget *widget = nullptr);

        /**
         * Effectively write and unlock the system-wide highscore file
         * (@see lockForWriting).
         * If using a local highscore file, it will sync the config.
         */
        void writeAndUnlock();

        /**
         * @return true if the highscore file is locked or if a local
         * file is used.
         */
        bool isLocked() const;

        /**
         * Destructor.
         * If necessary, write and unlock the highscore file.
         */
        ~KHighscore();

        /**
         * @param entry The number of the entry / the placing of the player
         * @param key A key for this entry. E.g. "name" for the name of the
         * player. Nearly the same as the usual keys in KConfig - but they
         * are prefixed with the entry number
         * @param value The value of this entry
         **/
        void writeEntry(int entry, const QString& key, const QString& value);

        /**
         * This is an overloaded member function, provided for convenience.
         * It differs from the above function only in what argument(s) it accepts.
         **/
        void writeEntry(int entry, const QString& key, int value);

        /**
         * This is an overloaded member function, provided for convenience.
         * It differs from the above function only in what argument(s) it accepts.
         * See KConfigBase documentation for allowed QVariant::Type.
         **/
        void writeEntry(int entry, const QString& key, const QVariant &value);

        /**
         * Reads an entry from the highscore table.
         * @param entry The number of the entry / the placing to be read
         * @param key The key of the entry. E.g. "name" for the name of the
         * player. Nearly the same as the usual keys in KConfig - but they
         * are prefixed with the entry number
         * @param pDefault This will be used as default value if the key+pair
         * entry can't be found.
         * @return The value of this entry+key pair or pDefault if the entry+key
         * pair doesn't exist
         **/
        QString readEntry(int entry, const QString& key, const QString& pDefault = QLatin1String("")) const;

        /**
         * Read a numeric value.
         * @param entry The number of the entry / the placing to be read
         * @param key The key of the entry. E.g. "name" for the name of the
         * player. Nearly the same as the usual keys in KConfig - but they
         * are prefixed with the entry number
         * @param pDefault This will be used as default value if the key+pair
         * entry can't be found.
         * @return The value of this entry+key pair or pDefault if the entry+key
         * pair doesn't exist
         **/
        int readNumEntry(int entry, const QString& key, int pDefault = -1) const;

        /**
         * Read a QVariant entry.
         * See KConfigBase documentation for allowed QVariant::Type.
         *
         * @return the value of this entry+key pair or pDefault if the entry+key
         * pair doesn't exist or
         */
        QVariant readPropertyEntry(int entry, const QString &key, const QVariant &pDefault) const;

        /**
         * @return True if the highscore table contains the entry/key pair,
         * otherwise false
         **/
        bool hasEntry(int entry, const QString& key) const;

        /**
         * Reads a list of entries from the highscore table starting at 1 until
         * lastEntry. If an entry between those numbers doesn't exist the
         * function aborts reading even if after the missing entry is an
         * existing one. The first entry of the list is the first placing, the
         * last on is the last placing.
         * @return A list of the entries of this key. You could also call
         * readEntry(i, key) where i is from 1 to 20. Note that this function
         * depends on "1" as the first entry!
         * @param key The key of the entry. E.g. "name" for the name of the
         * player. Nearly the same as the usual keys in KConfig - but they
         * are prefixed with the entry number
         * @param lastEntry the last entry which will be includes into the list.
         * 1 will include a list with maximal 1 entry - 20 a list with maximal
         * 20 entries. If lastEntry is <= 0 then rading is only stopped when
         * when an entry does not exist.
         **/
        QStringList readList(const QString& key, int lastEntry = 20) const;

        /**
         * Writes a list of entries to the highscore table.
         *
         * The first entry is prefixed with "1". Using this method is a short
         * way of calling writeEntry(i, key, list[i]) from i = 1 to
         * list.count()
         * @param key A key for the entry. E.g. "name" for the name of the
         * player. Nearly the same as the usual keys in KConfig - but they
         * are prefixed with the entry number
         * @param list The list of values
         **/
        void writeList(const QString& key, const QStringList& list);

        /**
         * You can use this function to indicate whether KHighscore created a
         * highscore table before and - if not - read your old (non-KHighscore)
         * table instead.
         * This way you can safely read an old table and save it using
         * KHighscore without losing any data
         * @return Whether a highscore table exists.
         **/
        bool hasTable() const;

        /**
         * Set the new highscore group. The group is being prefixed with
         * "KHighscore_" in the table.
         * @param groupname The new groupname. E.g. use "easy" for the easy
         * level of your game. If you use QString() (the default) the
         * default group is used.
         **/
        void setHighscoreGroup(const QString& groupname = QLatin1String(""));

        /**
         * Returns a list of group names without the KHighscore_ prexix.
         * E.g, "KHighscore", "KHighscore_Easy", "KHighscore_Medium"
         * will return "", "Easy", "Medium"
         *
         * @return A list of highscore groups.
         **/
        QStringList groupList() const;

        /**
         * @return The currently used group. This doesn't contain the prefix
         * ("KHighscore_") but the same as setHighscoreGroup uses. The
         * default is QString()
         **/
        QString highscoreGroup() const;

    protected:
        /**
         * @return A groupname to be used in KConfig. Used internally to
         * prefix the value from highscoreGroup() with "KHighscore_"
         **/
        QString group() const;

        /**
         * @return A pointer to the KConfig object to be used. This is
         * either KGlobal::config() (default) or a KSimpleConfig object for
         * a system-wide highscore file.
         **/
        KConfig* config() const;

        void init(bool forceLocal);

    private:
        class KHighscorePrivate;
        KHighscorePrivate* const d;
};

Q_DECLARE_LOGGING_CATEGORY(GAMES_HIGHSCORE)
#endif
