/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamethemeprovider.h"
#include "kgameimageprovider_p.h"

// own
#include <kdegames_logging.h>
// KF
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
// Qt
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlContext>
#include <QStandardPaths>
// Std
#include <utility>

class KGameThemeProviderPrivate
{
public:
    KGameThemeProvider *q;
    QString m_name;
    QList<const KGameTheme *> m_themes;
    const QByteArray m_configKey;
    const KGameTheme *m_currentTheme;
    const KGameTheme *m_defaultTheme;
    // this stores the arguments which were passed to discoverThemes()
    QString m_dtDirectory;
    QString m_dtDefaultThemeName;
    const QMetaObject *m_dtThemeClass;
    // this remembers which themes were already discovered
    QStringList m_discoveredThemes;
    // this disables the addTheme() lock during rediscoverThemes()
    bool m_inRediscover;

    KGameThemeProviderPrivate(KGameThemeProvider *parent, const QByteArray &key)
        : q(parent)
        , m_configKey(key)
        , m_currentTheme(nullptr)
        , m_defaultTheme(nullptr)
        , m_inRediscover(false)
    {
    }

    void updateThemeName()
    {
        Q_EMIT q->currentThemeNameChanged(q->currentThemeName());
    }
};

KGameThemeProvider::KGameThemeProvider(const QByteArray &configKey, QObject *parent)
    : QObject(parent)
    , d(new KGameThemeProviderPrivate(this, configKey))
{
    qRegisterMetaType<const KGameTheme *>();
    qRegisterMetaType<KGameThemeProvider *>();
    connect(this, &KGameThemeProvider::currentThemeChanged, this, [this]() {
        d->updateThemeName();
    });
}

KGameThemeProvider::~KGameThemeProvider()
{
    if (!d->m_themes.isEmpty()) {
        // save current theme in config file (no sync() call here; this will most
        // likely be called at application shutdown when others are also writing to
        // KGlobal::config(); also KConfig's dtor will sync automatically)
        // but do not save if there is no choice; this is esp. helpful for the
        // KGameRenderer constructor overload that uses a single KGameTheme instance
        if (d->m_themes.size() > 1 && !d->m_configKey.isEmpty()) {
            KConfigGroup cg(KSharedConfig::openConfig(), "KgTheme");
            cg.writeEntry(d->m_configKey.data(), currentTheme()->identifier());
        }
        // cleanup
        while (!d->m_themes.isEmpty()) {
            delete const_cast<KGameTheme *>(d->m_themes.takeFirst());
        }
    }
}

QString KGameThemeProvider::name() const
{
    return d->m_name;
}

QList<const KGameTheme *> KGameThemeProvider::themes() const
{
    return d->m_themes;
}

void KGameThemeProvider::addTheme(KGameTheme *theme)
{
    // The intended use is to create the KGameThemeProvider object, add themes,
    //*then* start to work with the currentLevel(). The first call to
    // currentTheme() will load the previous selection from the config, and the
    // level list will be considered immutable from this point.
    Q_ASSERT_X(d->m_currentTheme == nullptr || d->m_inRediscover, "KGameThemeProvider::addTheme", "Only allowed before currentTheme() is called.");
    // add theme
    d->m_themes.append(theme);
    theme->setParent(this);
}

const KGameTheme *KGameThemeProvider::defaultTheme() const
{
    return d->m_defaultTheme;
}

void KGameThemeProvider::setDefaultTheme(const KGameTheme *theme)
{
    if (d->m_currentTheme) {
        qCDebug(KDEGAMES_LOG) << "You're calling setDefaultTheme after the current "
                                 "theme has already been determined. That's not gonna work.";
        return;
    }
    Q_ASSERT(d->m_themes.contains(theme));
    d->m_defaultTheme = theme;
}

const KGameTheme *KGameThemeProvider::currentTheme() const
{
    if (d->m_currentTheme) {
        return d->m_currentTheme;
    }
    Q_ASSERT(!d->m_themes.isEmpty());
    // check configuration file for saved theme
    if (!d->m_configKey.isEmpty()) {
        KConfigGroup cg(KSharedConfig::openConfig(), "KTheme");
        const QByteArray id = cg.readEntry(d->m_configKey.data(), QByteArray());
        // look for a theme with this id
        for (const KGameTheme *theme : std::as_const(d->m_themes)) {
            if (theme->identifier() == id) {
                return d->m_currentTheme = theme;
            }
        }
    }
    // fall back to default theme (or first theme if no default specified)
    return d->m_currentTheme = (d->m_defaultTheme ? d->m_defaultTheme : d->m_themes.first());
}

void KGameThemeProvider::setCurrentTheme(const KGameTheme *theme)
{
    Q_ASSERT(d->m_themes.contains(theme));
    if (d->m_currentTheme != theme) {
        d->m_currentTheme = theme;
        Q_EMIT currentThemeChanged(theme);
    }
}

QString KGameThemeProvider::currentThemeName() const
{
    return currentTheme()->name();
}

void KGameThemeProvider::discoverThemes(const QString &directory, const QString &defaultThemeName, const QMetaObject *themeClass)
{
    d->m_dtDirectory = directory;
    d->m_dtDefaultThemeName = defaultThemeName;
    d->m_dtThemeClass = themeClass;
    rediscoverThemes();
}

static QStringList findSubdirectories(const QStringList &dirs)
{
    QStringList result;

    for (const QString &dir : dirs) {
        const QStringList subdirNames = QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        result.reserve(result.size() + subdirNames.size());
        for (const QString &subdirName : subdirNames) {
            const QString subdir = dir + QLatin1Char('/') + subdirName;
            result << subdir;
        }
    }
    if (!result.isEmpty()) {
        result += findSubdirectories(result);
    }

    return result;
}

void KGameThemeProvider::rediscoverThemes()
{
    if (d->m_dtDirectory.isEmpty()) {
        return; // discoverThemes() was never called
    }

    KGameTheme *defaultTheme = nullptr;

    d->m_inRediscover = true;
    const QString defaultFileName = d->m_dtDefaultThemeName + QLatin1String(".desktop");

    QStringList themePaths;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, d->m_dtDirectory, QStandardPaths::LocateDirectory);
    const QStringList allDirs = dirs + findSubdirectories(dirs);
    for (const QString &dir : allDirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.desktop"));
        for (const QString &file : fileNames) {
            if (!themePaths.contains(file)) {
                themePaths.append(dir + QLatin1Char('/') + file);
            }
        }
    }

    // create themes from result, order default theme at the front (that's not
    // needed by KGameThemeProvider, but nice for the theme selector)
    QList<KGameTheme *> themes;
    themes.reserve(themePaths.size());
    if (d->m_discoveredThemes.isEmpty()) {
        d->m_discoveredThemes.reserve(themePaths.size());
    }
    for (const QString &themePath : std::as_const(themePaths)) {
        const QFileInfo fi(themePath);
        if (d->m_discoveredThemes.contains(fi.fileName())) {
            continue;
        }
        d->m_discoveredThemes << fi.fileName();
        // the identifier is constructed such that it is compatible with
        // KGameTheme (e.g. "themes/default.desktop")
        const QByteArray id = QString(d->m_dtDirectory + QLatin1Char('/') + fi.fileName()).toUtf8();

        // create theme
        KGameTheme *theme;
        if (d->m_dtThemeClass) {
            theme = qobject_cast<KGameTheme *>(d->m_dtThemeClass->newInstance(Q_ARG(QByteArray, id), Q_ARG(QObject *, this)));
            Q_ASSERT_X(theme, "KGameThemeProvider::discoverThemes", "Could not create theme instance. Is your constructor Q_INVOKABLE?");
        } else {
            theme = new KGameTheme(id, this);
        }
        // silently discard invalid theme files
        if (!theme->readFromDesktopFile(themePath)) {
            delete theme;
            continue;
        }
        // order default theme at the front (that's not necessarily needed by
        // KGameThemeProvider, but nice for the theme selector)
        if (fi.fileName() == defaultFileName) {
            themes.prepend(theme);
            defaultTheme = theme;
        } else {
            themes.append(theme);
        }
    }
    // add themes in the determined order
    for (KGameTheme *theme : std::as_const(themes)) {
        addTheme(theme);
    }

    if (defaultTheme != nullptr) {
        setDefaultTheme(defaultTheme);
    } else if (d->m_defaultTheme == nullptr && themes.count() != 0) {
        setDefaultTheme(themes.value(0));
    }

    d->m_inRediscover = false;
}

QPixmap KGameThemeProvider::generatePreview(const KGameTheme *theme, QSize size)
{
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pixmap = QPixmap(theme->previewPath()).scaled(size * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(dpr);
    return pixmap;
}

void KGameThemeProvider::setDeclarativeEngine(const QString &name, QQmlEngine *engine)
{
    if (d->m_name != name) { // prevent multiple declarations
        d->m_name = name;
        engine->addImageProvider(name, new KGameImageProvider(this));
        engine->rootContext()->setContextProperty(name, this);
    }
}

#include "moc_kgamethemeprovider.cpp"
