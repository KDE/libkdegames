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

#ifndef KGTHEMEPROVIDER_H
#define KGTHEMEPROVIDER_H

#include <QObject>
#include <QtQml>

#include <kgtheme.h>
#include <libkdegames_export.h>

/**
 * @class KgThemeProvider kgthemeprovider.h <KgThemeProvider>
 *
 * A theme provider manages KgTheme instances, and maintains a selection of
 * the currentTheme(). It can automatically coordinate its selection with a
 * KGameRenderer instance.
 *
 * @note KgThemeProvider instances store selections in the application config,
 *       in the group [KgTheme]. This is documented here because this
 *       information is relevant for kconfig_
 */
class KDEGAMES_EXPORT KgThemeProvider : public QObject
{
	Q_OBJECT
	Q_PROPERTY(const KgTheme* currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
	Q_PROPERTY(QString name READ name NOTIFY nameChanged)
	Q_PROPERTY(QString currentThemeName READ currentThemeName NOTIFY currentThemeNameChanged)
	Q_DISABLE_COPY(KgThemeProvider)
	public:
		///Constructor. If you don't want KgThemeProvider to store the current
		///theme selection in the application config file automatically, set
		///@a configKey to an empty QByteArray.
		///
		///If there are multiple KgThemeProvider instances, make sure they use
		///different config keys to avoid collisions.
		explicit KgThemeProvider(const QByteArray& configKey = QByteArray("Theme"), QObject* parent = 0);
		///Destructor.
		virtual ~KgThemeProvider();

		///@return the name of the KgThemeProvider object. This name can be
		///used as QML element ID to reference the object inside QML.
		///@since 4.11
		QString name() const;

		///@return the themes in this provider
		QList<const KgTheme*> themes() const;
		///@return the default theme, or 0 if the provider does not contain any
		///themes
		const KgTheme* defaultTheme() const;
		///@see defaultTheme()
		///
		///Usually this will be set automatically by discoverThemes(). Call this
		///before the first call to currentTheme(), it won't have any effect
		///afterwards. @a theme must already have been added to this instance.
		void setDefaultTheme(const KgTheme* theme);
		///@return the currently selected theme, or 0 if the provider does not
		///contain any themes
		///
		///After the KgThemeProvider instance has been created, the current
		///theme will not be determined until this method is called
		///for the first time. This allows the application developer to set up
		///the theme provider before it restores the theme selection from the
		///configuration file.
		const KgTheme* currentTheme() const;

		///@return the name of the current theme
		///@since 4.11
		QString currentThemeName() const;

		///Adds a @a theme to this instance. The theme provider takes ownership
		///of @a theme.
		void addTheme(KgTheme* theme);
		///This method reads theme description files from a standard location.
		///The first two arguments are passed to KStandardDirs like this:
		///@code
		///KGlobal::dirs()->findAllResources(resource, directory + "/*.desktop");
		///@endcode
		///The typical usage is to install theme description files in
		///@code ${DATA_INSTALL_DIR}/themes @endcode and then call:
		///@code
		///themeProvider.discoverThemes("appdata", QLatin1String("themes"));
		///@endcode
		///If a @a themeClass's QMetaObject is given, the created themes will be
		///instances of this KgTheme subclass. The @a themeClass must export
		///(with the Q_INVOKABLE marker) a constructor with the same signature
		///as the KgTheme constructor.
		void discoverThemes(const QByteArray& resource, const QString& directory, const QString& defaultThemeName = QStringLiteral("default"), const QMetaObject* themeClass = 0);
		///After this provider has been set up with discoverThemes(), this
		///method may be used to read additional themes which were added since
		///the discoverThemes() call. This is esp. useful for KNewStuff
		///integration.
		void rediscoverThemes();

		///Generate a preview pixmap for the given theme. The application will
		///typically want to reimplement this to load the given theme into a
		///KGameRenderer and then arrange some sprites into a preview.
		///
		///@a size is the maximal allowed size.
		///
		///The default implementation tries to load a preview image from
		///KgTheme::previewPath(), and resizes the result to fit in @a size.
		virtual QPixmap generatePreview(const KgTheme* theme, const QSize& size);

		///Registers this KgThemeProvider with @a engine's root context with ID
		///@a name and constructs a KgImageProvider corresponding
		///to this KgThemeProvider and adds it to the QML engine, also
		///with @a name, which will receive sprite requests
		///@since 4.11
		void setDeclarativeEngine(const QString& name, QQmlEngine* engine);
	Q_SIGNALS:
		///Emitted when the current theme changes. @see setCurrentTheme
		void currentThemeChanged(const KgTheme* theme);
		///Emitted when the name of the provider changes.
		///@since 4.11
		void nameChanged(const QString& name);
		///Emitted when the name of the current theme changes.
		///@since 4.11
		void currentThemeNameChanged(const QString& themeName);
	public Q_SLOTS:
		///Select a new theme. The given theme must already have been added to
		///this instance.
		void setCurrentTheme(const KgTheme* theme);
	private:
		class Private;
		Private* const d;
		Q_PRIVATE_SLOT(d, void updateThemeName());
};

Q_DECLARE_METATYPE(KgThemeProvider*)
QML_DECLARE_TYPE(KgThemeProvider*)

#endif // KGTHEMEPROVIDER_H
