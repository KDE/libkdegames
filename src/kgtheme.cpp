/***************************************************************************
 *   Copyright 2012 Stefan Majewsky <majewsky@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "kgtheme.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>

Q_LOGGING_CATEGORY(GAMES_LIB, "org.kde.games.lib", QtWarningMsg)

class KgTheme::Private
{
    public:
        const QByteArray m_identifier;
        QString m_name, m_description, m_author, m_authorEmail, m_graphicsPath, m_previewPath;
        QMap<QString, QString> m_customData;

        Private(const QByteArray& id) : m_identifier(id) {}

        static QStringList s_configGroupNames;
};

/*static*/ QStringList KgTheme::Private::s_configGroupNames;

KgTheme::KgTheme(const QByteArray& identifier, QObject* parent)
	: QObject(parent)
	, d(new Private(identifier))
{
}

KgTheme::~KgTheme()
{
	delete d;
}

QByteArray KgTheme::identifier() const
{
	return d->m_identifier;
}

#define KGTHEME_STRING_PROPERTY(PROP, SETTER) \
	QString KgTheme::PROP() const { return d->m_##PROP; } \
	void KgTheme::SETTER(const QString& val) { d->m_##PROP = val; }

KGTHEME_STRING_PROPERTY(name, setName)
KGTHEME_STRING_PROPERTY(description, setDescription)
KGTHEME_STRING_PROPERTY(author, setAuthor)
KGTHEME_STRING_PROPERTY(authorEmail, setAuthorEmail)
KGTHEME_STRING_PROPERTY(graphicsPath, setGraphicsPath)
KGTHEME_STRING_PROPERTY(previewPath, setPreviewPath)

QMap<QString, QString> KgTheme::customData() const
{
	return d->m_customData;
}

QString KgTheme::customData(const QString& key, const QString& defaultValue) const
{
	return d->m_customData.value(key, defaultValue);
}

void KgTheme::setCustomData(const QMap<QString, QString>& customData)
{
	d->m_customData = customData;
}

bool KgTheme::readFromDesktopFile(const QString& path_)
{
	if (path_.isEmpty())
	{
		qCDebug(GAMES_LIB) << "Refusing to load theme with no name";
		return false;
	}
	//legacy support: relative paths are resolved with KStandardDirs/appdata
	QString path(path_);
	if (QFileInfo(path).isRelative())
	{
		path = QStandardPaths::locate(QStandardPaths::AppDataLocation, path);
		if (path.isEmpty())
		{
			qCDebug(GAMES_LIB) << "Could not find theme description" << path;
			return false;
		}
	}
	//default group name
	if (Private::s_configGroupNames.isEmpty())
	{
		Private::s_configGroupNames << QStringLiteral("KGameTheme");
	}
	//open file, look for a known config group
	KConfig config(path, KConfig::SimpleConfig);
	KConfigGroup group;
	for (const QString& groupName : qAsConst(Private::s_configGroupNames)) {
		if (config.hasGroup(groupName))
		{
			group = config.group(groupName);
		}
	}
	if (!group.isValid())
	{
		qCDebug(GAMES_LIB) << "Could not read theme description at" << path;
		return false;
	}
	//check format version
	if (group.readEntry("VersionFormat", 1) > 1)
	{
		qCDebug(GAMES_LIB) << "Format of theme description too new at" << path;
		return false;
	}

	//resolve paths
	const QFileInfo fi(path);
	const QDir dir = fi.dir();
	QString graphicsPath = group.readEntry("FileName", QString());
	if (!graphicsPath.isEmpty() && QFileInfo(graphicsPath).isRelative())
		graphicsPath = dir.absoluteFilePath(graphicsPath);
	QString previewPath = group.readEntry("Preview", QString());
	if (!previewPath.isEmpty() && QFileInfo(previewPath).isRelative())
		previewPath = dir.absoluteFilePath(previewPath);
	//create theme
	setName(group.readEntry("Name", QString()));
	setDescription(group.readEntry("Description", QString()));
	setAuthor(group.readEntry("Author", QString()));
	setAuthorEmail(group.readEntry("AuthorEmail", QString()));
	setGraphicsPath(graphicsPath);
	setPreviewPath(previewPath);
	setCustomData(group.entryMap());
	//store modification date of this file in private property (KGameRenderer
	//wants to clear its cache also if the theme description changes)
	setProperty("_k_themeDescTimestamp", fi.lastModified().toSecsSinceEpoch());
	return true;
}


