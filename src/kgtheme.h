/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGTHEME_H
#define KGTHEME_H

// own
#include <libkdegames_export.h>
// Qt
#include <QLoggingCategory>
#include <QMetaType>
#include <QObject>
#include <QPixmap>
// Std
#include <memory>

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
    // it is not intended to allow these properties to change after the initial
    // setup (note how KgThemeProvider returns only const KgTheme*), hence
    // a dummy NOTIFY signal is enough
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY readOnlyProperty)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY readOnlyProperty)
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY readOnlyProperty)
    Q_PROPERTY(QString authorEmail READ authorEmail WRITE setAuthorEmail NOTIFY readOnlyProperty)
    Q_PROPERTY(QString graphicsPath READ graphicsPath WRITE setGraphicsPath NOTIFY readOnlyProperty)
    Q_PROPERTY(QString previewPath READ previewPath WRITE setPreviewPath NOTIFY readOnlyProperty)
    Q_DISABLE_COPY(KgTheme)

public:
    /// Constructor. The @a identifier must be application-unique.
    explicit KgTheme(const QByteArray &identifier, QObject *parent = nullptr);
    /// Destructor.
    ~KgTheme() override;

    /// Initializes a KgTheme instance by reading a description file.
    /// @return whether @a path is a valid theme description file (if not,
    ///         the theme instance is not changed by this method call)
    /// @note A non-static member function has been chosen over the more
    ///       common pattern of using a static member function like
    ///       "KgTheme::fromDesktopFile" to accommodate applications which
    ///       want to subclass KgTheme.
    virtual bool readFromDesktopFile(const QString &path);

    /// @return the internal identifier for this theme (used e.g. for
    ///         finding a pixmap cache or storing a theme selection)
    QByteArray identifier() const;

    /// @return the name of this theme
    QString name() const;
    /// @see name()
    void setName(const QString &name);
    /// @return an additional description beyond the name()
    QString description() const;
    /// @see description()
    void setDescription(const QString &description);
    /// @return the name of the theme author
    QString author() const;
    /// @see author()
    void setAuthor(const QString &author);
    /// @return the email address of the theme author
    QString authorEmail() const;
    /// @see authorEmail()
    void setAuthorEmail(const QString &authorEmail);

    /// @return the path of the SVG file which holds the theme contents
    QString graphicsPath() const;
    /// @see graphicsPath()
    void setGraphicsPath(const QString &path);
    /// @return the path to an image file containing a preview, or an empty
    ///         string if no preview file is known
    QString previewPath() const;
    /// @see previewPath()
    void setPreviewPath(const QString &path);

    /// @return custom data
    ///
    /// This API is provided for theme description files which contain
    /// additional application-specific metadata.
    QMap<QString, QString> customData() const;
    /// @overload that returns a single value from the customData() map
    QString customData(const QString &key, const QString &defaultValue = QString()) const;
    /// @see customData()
    void setCustomData(const QMap<QString, QString> &customData);
Q_SIGNALS:
    /// This signal is never emitted. It is provided because QML likes to
    /// complain about properties without NOTIFY signals, even readonly ones.
    void readOnlyProperty();

private:
    std::unique_ptr<class KgThemePrivate> const d;
};

Q_DECLARE_METATYPE(const KgTheme *)

#endif // KGTHEME_H
