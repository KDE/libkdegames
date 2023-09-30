/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgtheme.h"

// own
#include <kdegames_logging.h>
// KF
#include <KConfig>
#include <KConfigGroup>
// Qt
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
// Std
#include <utility>

class KgThemePrivate
{
public:
    const QByteArray m_identifier;
    QString m_name, m_description, m_author, m_authorEmail, m_graphicsPath, m_previewPath;
    QMap<QString, QString> m_customData;

    KgThemePrivate(const QByteArray &id)
        : m_identifier(id)
    {
    }

    static QStringList s_configGroupNames;
};

/*static*/ QStringList KgThemePrivate::s_configGroupNames;

KgTheme::KgTheme(const QByteArray &identifier, QObject *parent)
    : QObject(parent)
    , d(new KgThemePrivate(identifier))
{
}

KgTheme::~KgTheme() = default;

QByteArray KgTheme::identifier() const
{
    return d->m_identifier;
}

#define KGTHEME_STRING_PROPERTY(PROP, SETTER)                                                                                                                  \
    QString KgTheme::PROP() const                                                                                                                              \
    {                                                                                                                                                          \
        return d->m_##PROP;                                                                                                                                    \
    }                                                                                                                                                          \
    void KgTheme::SETTER(const QString &val)                                                                                                                   \
    {                                                                                                                                                          \
        d->m_##PROP = val;                                                                                                                                     \
    }

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

QString KgTheme::customData(const QString &key, const QString &defaultValue) const
{
    return d->m_customData.value(key, defaultValue);
}

void KgTheme::setCustomData(const QMap<QString, QString> &customData)
{
    d->m_customData = customData;
}

bool KgTheme::readFromDesktopFile(const QString &path_)
{
    if (path_.isEmpty()) {
        qCDebug(KDEGAMES_LOG) << "Refusing to load theme with no name";
        return false;
    }
    // legacy support: relative paths are resolved with KStandardDirs/appdata
    QString path(path_);
    if (QFileInfo(path).isRelative()) {
        path = QStandardPaths::locate(QStandardPaths::AppDataLocation, path);
        if (path.isEmpty()) {
            qCDebug(KDEGAMES_LOG) << "Could not find theme description" << path;
            return false;
        }
    }
    // default group name
    if (KgThemePrivate::s_configGroupNames.isEmpty()) {
        KgThemePrivate::s_configGroupNames << QStringLiteral("KGameTheme");
    }
    // open file, look for a known config group
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group;
    for (const QString &groupName : std::as_const(KgThemePrivate::s_configGroupNames)) {
        if (config.hasGroup(groupName)) {
            group = config.group(groupName);
        }
    }
    if (!group.isValid()) {
        qCDebug(KDEGAMES_LOG) << "Could not read theme description at" << path;
        return false;
    }
    // check format version
    if (group.readEntry("VersionFormat", 1) > 1) {
        qCDebug(KDEGAMES_LOG) << "Format of theme description too new at" << path;
        return false;
    }

    // resolve paths
    const QFileInfo fi(path);
    const QDir dir = fi.dir();
    QString graphicsPath = group.readEntry("FileName", QString());
    if (!graphicsPath.isEmpty() && QFileInfo(graphicsPath).isRelative())
        graphicsPath = dir.absoluteFilePath(graphicsPath);
    QString previewPath = group.readEntry("Preview", QString());
    if (!previewPath.isEmpty() && QFileInfo(previewPath).isRelative())
        previewPath = dir.absoluteFilePath(previewPath);
    // create theme
    setName(group.readEntry("Name", QString()));
    setDescription(group.readEntry("Description", QString()));
    setAuthor(group.readEntry("Author", QString()));
    setAuthorEmail(group.readEntry("AuthorEmail", QString()));
    setGraphicsPath(graphicsPath);
    setPreviewPath(previewPath);
    setCustomData(group.entryMap());
    // store modification date of this file in private property (KGameRenderer
    // wants to clear its cache also if the theme description changes)
    setProperty("_k_themeDescTimestamp", fi.lastModified().toSecsSinceEpoch());
    return true;
}

#include "moc_kgtheme.cpp"
