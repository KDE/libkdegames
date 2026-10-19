/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMETHEME_H
#define KGAMETHEME_H

// own
#include "kdegames_export.h"
// Qt
#include <QMetaType>
#include <QObject>
// Std
#include <memory>

/*!
 * \class KGameTheme
 * \inmodule LibKDEGames
 * \brief Describes the visual appearance of a game.
 *
 * A theme describes the visual appearance of a game. Themes in kdegames usually
 * reference a SVGZ file which is loaded into a KGameRenderer to provide pixmaps
 * for use on the game canvas.
 *
 * Themes are usually managed (and discovered) by a KGameThemeProvider.
 *
 * \section1 Default file format for theme descriptions
 *
 * Although KGameTheme and KGameThemeProvider do not need special theme description
 * files for most basic usage, there is a format for theme description files
 * based on the XDG Desktop File Specification. The following example shows the
 * recognized keys:
 *
 * \code [ini]
 * [KGameTheme]
 * VersionFormat=1
 * Name=Example
 * Description=This theme serves as an example.
 * License=CC-BY-SA-4.0
 * Copyright=1984 John Doe
 * Version=1.0
 * Website=https://theme.example
 * BugReportUrl=https://theme.example/newbugform
 * FileName=example.svgz
 * Author=John Doe
 * AuthorEmail=john.doe@example.com
 * Preview=example.png
 * \endcode
 *
 * All keys are considered optional, except perhaps for the "FileName" key: If
 * that is not given, the theme cannot be used with KGameRenderer. The paths in
 * "FileName" and "Preview" are resolved relative to the directory that contains
 * the theme description file.
 *
 * If the \c [KGameTheme] group contains any further keys, their values can be
 * retrieved through the KGameTheme::customData() method.
 */

class KDEGAMES_EXPORT KGameTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray identifier READ identifier NOTIFY readOnlyProperty)
    // it is not intended to allow these properties to change after the initial
    // setup (note how KGameThemeProvider returns only const KGameTheme*), hence
    // a dummy NOTIFY signal is enough
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY readOnlyProperty)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY readOnlyProperty)
    Q_PROPERTY(QString license READ license WRITE setLicense NOTIFY readOnlyProperty)
    Q_PROPERTY(QString copyrightText READ copyrightText WRITE setCopyrightText NOTIFY readOnlyProperty)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY readOnlyProperty)
    Q_PROPERTY(QString website READ website WRITE setWebsite NOTIFY readOnlyProperty)
    Q_PROPERTY(QString bugReportUrl READ bugReportUrl WRITE setBugReportUrl NOTIFY readOnlyProperty)
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY readOnlyProperty)
    Q_PROPERTY(QString authorEmail READ authorEmail WRITE setAuthorEmail NOTIFY readOnlyProperty)
    Q_PROPERTY(QString graphicsPath READ graphicsPath WRITE setGraphicsPath NOTIFY readOnlyProperty)
    Q_PROPERTY(QString previewPath READ previewPath WRITE setPreviewPath NOTIFY readOnlyProperty)
    Q_DISABLE_COPY(KGameTheme)

public:
    /*!
     * Constructs a KGameTheme with \a parent. The \a identifier must be application-unique.
     */
    explicit KGameTheme(const QByteArray &identifier, QObject *parent = nullptr);
    /// Destructor.
    ~KGameTheme() override;

    /*!
     * Initializes a KGameTheme instance by reading a description file.
     * Returns whether \a path is a valid theme description file (if not,
     * the theme instance is not changed by this method call).
     * \note A non-static member function has been chosen over the more
     * common pattern of using a static member function like
     * "KGameTheme::fromDesktopFile" to accommodate applications which
     * want to subclass KGameTheme.
     */
    virtual bool readFromDesktopFile(const QString &path);

    /*!
     * \property KGameTheme::identifier
     * \brief The internal identifier for this theme.
     *
     * Used, for example, for finding a pixmap cache or storing a theme selection.
     */
    QByteArray identifier() const;

    /*!
     * \property KGameTheme::name
     * \brief The name of this theme.
     */
    QString name() const;
    void setName(const QString &name);

    /*!
     * \property KGameTheme::description
     * \brief An additional description beyond the name().
     */
    QString description() const;
    void setDescription(const QString &description);

    /*!
     * \property KGameTheme::license
     * \brief The short license identifier, an SPDX expression.
     */
    QString license() const;
    void setLicense(const QString &license);

    /*!
     * \property KGameTheme::copyrightText
     * \brief a short copyright statement.
     */
    QString copyrightText() const;
    void setCopyrightText(const QString &copyrightText);

    /*!
     * \property KGameTheme::version
     * \brief The version of the theme.
     */
    QString version() const;
    void setVersion(const QString &version);

    /*!
     * \property KGameTheme::website
     * \brief The website of the theme.
     */
    QString website() const;
    void setWebsite(const QString &website);

    /*!
     * \property KGameTheme::bugReportUrl
     * \brief The website where people can report a bug found in this theme.
     */
    QString bugReportUrl() const;
    void setBugReportUrl(const QString &bugReportUrl);

    /*!
     * \property KGameTheme::author
     * \brief The name of the theme author.
     */
    QString author() const;
    void setAuthor(const QString &author);

    /*!
     * \property KGameTheme::authorEmail
     * \brief The email address of the theme author
     */
    QString authorEmail() const;
    void setAuthorEmail(const QString &authorEmail);

    /*!
     * \property KGameTheme::graphicsPath
     * \brief The path of the SVG file which holds the theme contents.
     */
    QString graphicsPath() const;
    void setGraphicsPath(const QString &path);

    /*!
     * \property KGameTheme::previewPath
     * \brief The path to an image file containing a preview, or an empty
     * string if no preview file is known.
     */
    QString previewPath() const;
    void setPreviewPath(const QString &path);

    /*!
     * Returns custom data.
     *
     * This API is provided for theme description files which contain
     * additional application-specific metadata.
     */
    QMap<QString, QString> customData() const;
    /*!
     * Returns the value associated with \a key from the customData() map,
     * or \a defaultValue.
     */
    QString customData(const QString &key, const QString &defaultValue = QString()) const;
    /*!
     * Sets the custom data to \a customData.
     */
    void setCustomData(const QMap<QString, QString> &customData);

Q_SIGNALS:
    /// This signal is never emitted. It is provided because QML likes to
    /// complain about properties without NOTIFY signals, even readonly ones.
    void readOnlyProperty();

private:
    std::unique_ptr<class KGameThemePrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KGameTheme)
};

Q_DECLARE_METATYPE(const KGameTheme *)

#endif // KGAMETHEME_H
