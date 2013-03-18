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

#include "kgthemeprovider.h"

#include <QtCore/QFileInfo>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KStandardDirs>

class KgThemeProvider::Private
{
    public:
        KgThemeProvider *q;
        QString m_name;
        QList<const KgTheme*> m_themes;
        const QByteArray m_configKey;
        const KgTheme* m_currentTheme;
        const KgTheme* m_defaultTheme;
        //this stores the arguments which were passed to discoverThemes()
        QByteArray m_dtResource;
        QString m_dtDirectory;
        QString m_dtDefaultThemeName;
        const QMetaObject* m_dtThemeClass;
        //this remembers which themes were already discovered
        QStringList m_discoveredThemes;
        //this disables the addTheme() lock during rediscoverThemes()
        bool m_inRediscover;

        Private(KgThemeProvider *parent, const QByteArray& key) : q(parent), m_configKey(key), m_currentTheme(0), m_defaultTheme(0), m_inRediscover(false) {}

        void updateThemeName()
        {
            emit q->currentThemeNameChanged(q->currentThemeName());
        }
};

KgThemeProvider::KgThemeProvider(const QByteArray& configKey, QObject* parent)
	: QObject(parent)
	, d(new Private(this, configKey))
{
	qRegisterMetaType<const KgTheme*>();
	qRegisterMetaType<KgThemeProvider*>();
	connect(this, SIGNAL(currentThemeChanged(const KgTheme*)), this, SLOT(updateThemeName()));
}

KgThemeProvider::~KgThemeProvider()
{
	if (d->m_themes.isEmpty())
	{
		return;
	}
	//save current theme in config file (no sync() call here; this will most
	//likely be called at application shutdown when others are also writing to
	//KGlobal::config(); also KConfig's dtor will sync automatically)
	//but do not save if there is no choice; this is esp. helpful for the
	//KGameRenderer constructor overload that uses a single KgTheme instance
	if (d->m_themes.size() > 1 && !d->m_configKey.isEmpty())
	{
		KConfigGroup cg(KGlobal::config(), "KgTheme");
		cg.writeEntry(d->m_configKey.data(), currentTheme()->identifier());
	}
	//cleanup
	while (!d->m_themes.isEmpty())
	{
		delete const_cast<KgTheme*>(d->m_themes.takeFirst());
	}
}

QString KgThemeProvider::name() const
{
	return d->m_name;
}

void KgThemeProvider::setName(const QString& name)
{
	if(d->m_name != name) {
		d->m_name = name;
		emit nameChanged(name);
	}
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
	Q_ASSERT_X(d->m_currentTheme == 0 || d->m_inRediscover,
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
		kDebug(11000) << "You're calling setDefaultTheme after the current "
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
		KConfigGroup cg(KGlobal::config(), "KgTheme");
		const QByteArray id = cg.readEntry(d->m_configKey.data(), QByteArray());
		//look for a theme with this id
		foreach (const KgTheme* theme, d->m_themes)
		{
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
		emit currentThemeChanged(theme);
	}
}

QString KgThemeProvider::currentThemeName() const
{
	return currentTheme()->name();
}

void KgThemeProvider::discoverThemes(const QByteArray& resource, const QString& directory, const QString& defaultThemeName, const QMetaObject* themeClass)
{
	d->m_dtResource = resource;
	d->m_dtDirectory = directory;
	d->m_dtDefaultThemeName = defaultThemeName;
	d->m_dtThemeClass = themeClass;
	rediscoverThemes();
}

void KgThemeProvider::rediscoverThemes()
{
	if (d->m_dtResource.isEmpty())
	{
		return; //discoverThemes() was never called
	}
	
	KgTheme* defaultTheme = NULL;
	
	d->m_inRediscover = true;
	const QString defaultFileName = d->m_dtDefaultThemeName + QLatin1String(".desktop");
	const QStringList themePaths = KGlobal::dirs()->findAllResources(
		d->m_dtResource, d->m_dtDirectory + QLatin1String("/*.desktop"),
		KStandardDirs::NoDuplicates
	);
	//create themes from result, order default theme at the front (that's not
	//needed by KgThemeProvider, but nice for the theme selector)
	QList<KgTheme*> themes;
	foreach (const QString& themePath, themePaths)
	{
		const QFileInfo fi(themePath);
		if (d->m_discoveredThemes.contains(fi.fileName()))
		{
			continue;
		}
		d->m_discoveredThemes << fi.fileName();
		//the identifier is constructed such that it is compatible with
		//KGameTheme (e.g. "themes/default.desktop")
		const QByteArray id = KGlobal::dirs()->relativeLocation(
			d->m_dtResource, themePath).toUtf8();
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
	foreach (KgTheme* theme, themes)
	{
		addTheme(theme);
	}
	
	if(defaultTheme != NULL)
	{
		setDefaultTheme(defaultTheme);
	}
	else if(d->m_defaultTheme == NULL && themes.count() != 0)
	{
		setDefaultTheme(themes.value(0));
	}
	
	d->m_inRediscover = false;
}

QPixmap KgThemeProvider::generatePreview(const KgTheme* theme, const QSize& size)
{
	return QPixmap(theme->previewPath()).scaled(size, Qt::KeepAspectRatio);
}

#include "kgthemeprovider.moc"
