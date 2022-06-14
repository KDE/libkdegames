/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgthemeprovider.h"
#include "kgimageprovider_p.h"

// KF
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
// Qt
#include <QFileInfo>
#include <QStandardPaths>
#include <QGuiApplication>

class KgThemeProviderPrivate
{
    public:
        KgThemeProvider *q;
        QString m_name;
        QList<const KgTheme*> m_themes;
        const QByteArray m_configKey;
        const KgTheme* m_currentTheme;
        const KgTheme* m_defaultTheme;
        //this stores the arguments which were passed to discoverThemes()
        QString m_dtDirectory;
        QString m_dtDefaultThemeName;
        const QMetaObject* m_dtThemeClass;
        //this remembers which themes were already discovered
        QStringList m_discoveredThemes;
        //this disables the addTheme() lock during rediscoverThemes()
        bool m_inRediscover;

        KgThemeProviderPrivate(KgThemeProvider *parent, const QByteArray& key)
            : q(parent)
            , m_configKey(key)
            , m_currentTheme(nullptr)
            , m_defaultTheme(nullptr)
            , m_inRediscover(false)
        {}

        void updateThemeName()
        {
            Q_EMIT q->currentThemeNameChanged(q->currentThemeName());
        }
};

KgThemeProvider::KgThemeProvider(const QByteArray& configKey, QObject* parent)
	: QObject(parent)
	, d(new KgThemeProviderPrivate(this, configKey))
{
	qRegisterMetaType<const KgTheme*>();
	qRegisterMetaType<KgThemeProvider*>();
	connect(this, &KgThemeProvider::currentThemeChanged, this, [this]() { d->updateThemeName(); });
}

KgThemeProvider::~KgThemeProvider()
{
        if (!d->m_themes.isEmpty())
        {
	//save current theme in config file (no sync() call here; this will most
	//likely be called at application shutdown when others are also writing to
	//KGlobal::config(); also KConfig's dtor will sync automatically)
	//but do not save if there is no choice; this is esp. helpful for the
	//KGameRenderer constructor overload that uses a single KgTheme instance
	if (d->m_themes.size() > 1 && !d->m_configKey.isEmpty())
	{
		KConfigGroup cg(KSharedConfig::openConfig(), "KgTheme");
		cg.writeEntry(d->m_configKey.data(), currentTheme()->identifier());
	}
	//cleanup
	while (!d->m_themes.isEmpty())
	{
		delete const_cast<KgTheme*>(d->m_themes.takeFirst());
        }
        }
}

QString KgThemeProvider::name() const
{
	return d->m_name;
}

QList<const KgTheme*> KgThemeProvider::themes() const
{
	return d->m_themes;
}

void KgThemeProvider::addTheme(KgTheme* theme)
{
	//The intended use is to create the KgThemeProvider object, add themes,
	//*then* start to work with the currentLevel(). The first call to
	//currentTheme() will load the previous selection from the config, and the
	//level list will be considered immutable from this point.
	Q_ASSERT_X(d->m_currentTheme == nullptr || d->m_inRediscover,
		"KgThemeProvider::addTheme",
		"Only allowed before currentTheme() is called."
	);
	//add theme
	d->m_themes.append(theme);
	theme->setParent(this);
}

const KgTheme* KgThemeProvider::defaultTheme() const
{
	return d->m_defaultTheme;
}

void KgThemeProvider::setDefaultTheme(const KgTheme* theme)
{
	if (d->m_currentTheme)
	{
		qCDebug(GAMES_LIB) << "You're calling setDefaultTheme after the current "
			"theme has already been determined. That's not gonna work.";
		return;
	}
	Q_ASSERT(d->m_themes.contains(theme));
	d->m_defaultTheme = theme;
}

const KgTheme* KgThemeProvider::currentTheme() const
{
	if (d->m_currentTheme)
	{
		return d->m_currentTheme;
	}
	Q_ASSERT(!d->m_themes.isEmpty());
	//check configuration file for saved theme
	if (!d->m_configKey.isEmpty())
	{
		KConfigGroup cg(KSharedConfig::openConfig(), "KgTheme");
		const QByteArray id = cg.readEntry(d->m_configKey.data(), QByteArray());
		//look for a theme with this id
		for (const KgTheme* theme : qAsConst(d->m_themes)) {
			if (theme->identifier() == id)
			{
				return d->m_currentTheme = theme;
			}
		}
	}
	//fall back to default theme (or first theme if no default specified)
	return d->m_currentTheme = (d->m_defaultTheme ? d->m_defaultTheme : d->m_themes.first());
}

void KgThemeProvider::setCurrentTheme(const KgTheme* theme)
{
	Q_ASSERT(d->m_themes.contains(theme));
	if (d->m_currentTheme != theme)
	{
		d->m_currentTheme = theme;
		Q_EMIT currentThemeChanged(theme);
	}
}

QString KgThemeProvider::currentThemeName() const
{
	return currentTheme()->name();
}

#if KDEGAMES_BUILD_DEPRECATED_SINCE(7, 4)
void KgThemeProvider::discoverThemes(const QByteArray& resource, const QString& directory, const QString& defaultThemeName, const QMetaObject* themeClass)
{
    Q_UNUSED(resource);
    discoverThemes(directory, defaultThemeName, themeClass);
}
#endif

void KgThemeProvider::discoverThemes(const QString& directory, const QString& defaultThemeName, const QMetaObject* themeClass)
{
	d->m_dtDirectory = directory;
	d->m_dtDefaultThemeName = defaultThemeName;
	d->m_dtThemeClass = themeClass;
	rediscoverThemes();
}

// Function to replace KStandardDirs::relativeLocation()
static QString relativeToApplications(const QString& file)
{
	const QString canonical = QFileInfo(file).canonicalFilePath();
	const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	for (const QString& base : dirs) {
		if (canonical.startsWith(base))
			return canonical.mid(base.length()+1);
	}
	return file;
}

void KgThemeProvider::rediscoverThemes()
{
	if (d->m_dtDirectory.isEmpty())
	{
		return; //discoverThemes() was never called
	}

	KgTheme* defaultTheme = nullptr;

	d->m_inRediscover = true;
	const QString defaultFileName = d->m_dtDefaultThemeName + QLatin1String(".desktop");

	QStringList themePaths;
	const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, d->m_dtDirectory, QStandardPaths::LocateDirectory);
	for (const QString &dir : dirs) {
		const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.desktop"));
		for (const QString &file : fileNames) {
			if (!themePaths.contains(file)) {
				themePaths.append(dir + QLatin1Char('/') + file);
			}
		}
	}

	//create themes from result, order default theme at the front (that's not
	//needed by KgThemeProvider, but nice for the theme selector)
	QList<KgTheme*> themes;
	for (const QString& themePath : qAsConst(themePaths)) {
		const QFileInfo fi(themePath);
		if (d->m_discoveredThemes.contains(fi.fileName()))
		{
			continue;
		}
		d->m_discoveredThemes << fi.fileName();
		//the identifier is constructed such that it is compatible with
		//KGameTheme (e.g. "themes/default.desktop")

		const QByteArray id = relativeToApplications(themePath).toUtf8();
		//create theme
		KgTheme* theme;
		if (d->m_dtThemeClass)
		{
			theme = qobject_cast<KgTheme*>(d->m_dtThemeClass->newInstance(
				Q_ARG(QByteArray, id), Q_ARG(QObject*, this)
			));
			Q_ASSERT_X(theme,
				"KgThemeProvider::discoverThemes",
				"Could not create theme instance. Is your constructor Q_INVOKABLE?"
			);
		}
		else
		{
			theme = new KgTheme(id, this);
		}
		//silently discard invalid theme files
		if (!theme->readFromDesktopFile(themePath))
		{
			delete theme;
			continue;
		}
		//order default theme at the front (that's not necessarily needed by
		//KgThemeProvider, but nice for the theme selector)
		if (fi.fileName() == defaultFileName)
		{
			themes.prepend(theme);
			defaultTheme = theme;
		}
		else
		{
			themes.append(theme);
		}
	}
	//add themes in the determined order
	for (KgTheme* theme : qAsConst(themes))
	{
		addTheme(theme);
	}

	if(defaultTheme != nullptr)
	{
		setDefaultTheme(defaultTheme);
	}
	else if(d->m_defaultTheme == nullptr && themes.count() != 0)
	{
		setDefaultTheme(themes.value(0));
	}

	d->m_inRediscover = false;
}

QPixmap KgThemeProvider::generatePreview(const KgTheme* theme, const QSize& size)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	const qreal dpr = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1;
#else
    const qreal dpr = qApp->devicePixelRatio();
#endif
	QPixmap pixmap = QPixmap(theme->previewPath()).scaled(size * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	pixmap.setDevicePixelRatio(dpr);
	return pixmap;
}

void KgThemeProvider::setDeclarativeEngine(const QString& name, QQmlEngine* engine)
{
	if (d->m_name != name) { // prevent multiple declarations
		d->m_name = name;
		engine->addImageProvider(name, new KgImageProvider(this));
		engine->rootContext()->setContextProperty(name, this);
	}
}

#include "moc_kgthemeprovider.cpp"
