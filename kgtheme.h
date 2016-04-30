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

#ifndef KGTHEME_H
#define KGTHEME_H

//The complete API needs to be there when compiling libkdegames.
#ifdef MAKE_KDEGAMES_LIB
#define KGTHEME_PROVIDE_COMPATIBILITY_API
#endif //MAKE_KDEGAMES_LIB

#include <QMetaType>
#include <QObject>
#include <QLoggingCategory>
#include <QPixmap>

#include <libkdegames_export.h>

/**
 * @class KgTheme kgtheme.h <KgTheme>
 *
 * A theme describes the visual appearance of a game. Themes in kdegames usually
 * reference a SVGZ file which is loaded into a KGameRenderer to provide pixmaps
 * for use on the game canvas.
 *
 * Themes are usually managed (and discovered) by a KgThemeProvider.
 *
 * @section fileformat Default file format for theme descriptions
 *
 * Although KgTheme and KgThemeProvider do not need special theme description
 * files for most basic usage, there is a format for theme description files
 * based on the XDG Desktop File Specification. The following example shows the
 * recognized keys:
 *
 * @code
 * [KGameTheme]
 * VersionFormat=1
 * Name=Example
 * Description=This theme serves as an example.
 * FileName=example.svgz
 * Author=John Doe
 * AuthorEmail=john.doe@example.com
 * Preview=example.png
 * @endcode
 *
 * All keys are considered optional, except perhaps for the "FileName" key: If
 * that is not given, the theme cannot be used with KGameRenderer. The paths in
 * "FileName" and "Preview" are resolved relative to the directory that contains
 * the theme description file.
 *
 * If the [KGameTheme] group contains any further keys, their values can be
 * retrieved through the KgTheme::customData() method.
 */

Q_DECLARE_LOGGING_CATEGORY(GAMES_LIB)

class KDEGAMES_EXPORT KgTheme : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QByteArray identifier READ identifier NOTIFY readOnlyProperty)
	//it is not intended to allow these properties to change after the initial
	//setup (note how KgThemeProvider returns only const KgTheme*), hence
	//a dummy NOTIFY signal is enough
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY readOnlyProperty)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY readOnlyProperty)
	Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY readOnlyProperty)
	Q_PROPERTY(QString authorEmail READ authorEmail WRITE setAuthorEmail NOTIFY readOnlyProperty)
	Q_PROPERTY(QString graphicsPath READ graphicsPath WRITE setGraphicsPath NOTIFY readOnlyProperty)
	Q_PROPERTY(QString previewPath READ previewPath WRITE setPreviewPath NOTIFY readOnlyProperty)
	Q_DISABLE_COPY(KgTheme)
	public:
		///Constructor. The @a identifier must be application-unique.
		explicit KgTheme(const QByteArray& identifier, QObject* parent = 0);
		///Destructor.
		virtual ~KgTheme();

		///Initializes a KgTheme instance by reading a description file.
		///@return whether @a path is a valid theme description file (if not,
		///        the theme instance is not changed by this method call)
		///@note A non-static member function has been chosen over the more
		///      common pattern of using a static member function like
		///      "KgTheme::fromDesktopFile" to accommodate applications which
		///      want to subclass KgTheme.
		virtual bool readFromDesktopFile(const QString& path);

#ifdef KGTHEME_PROVIDE_COMPATIBILITY_API
		///Use a different group name in theme description files. For example,
		///KMahjongg backgrounds and tilesets use "[KMahjonggBackground]" and
		///"[KMahjonggTileset]" instead of "[KGameTheme]".
		static void addConfigGroupName(const QString& name);
#endif

		///@return the internal identifier for this theme (used e.g. for
		///        finding a pixmap cache or storing a theme selection)
		QByteArray identifier() const;

		///@return the name of this theme
		QString name() const;
		///@see name()
		void setName(const QString& name);
		///@return an additional description beyond the name()
		QString description() const;
		///@see description()
		void setDescription(const QString& description);
		///@return the name of the theme author
		QString author() const;
		///@see author()
		void setAuthor(const QString& author);
		///@return the email address of the theme author
		QString authorEmail() const;
		///@see authorEmail()
		void setAuthorEmail(const QString& authorEmail);

		///@return the path of the SVG file which holds the theme contents
		QString graphicsPath() const;
		///@see graphicsPath()
		void setGraphicsPath(const QString& path);
		///@return the path to an image file containing a preview, or an empty
		///        string if no preview file is known
		QString previewPath() const;
		///@see previewPath()
		void setPreviewPath(const QString& path);

		///@return custom data
		///
		///This API is provided for theme description files which contain
		///additional application-specific metadata.
		QMap<QString, QString> customData() const;
		///@overload that returns a single value from the customData() map
		QString customData(const QString& key, const QString& defaultValue = QString()) const;
		///@see customData()
		void setCustomData(const QMap<QString, QString>& customData);
	Q_SIGNALS:
		///This signal is never emitted. It is provided because QML likes to
		///complain about properties without NOTIFY signals, even readonly ones.
		void readOnlyProperty();
	private:
		class Private;
		Private* const d;
};

Q_DECLARE_METATYPE(const KgTheme*)

#endif // KGTHEME_H
