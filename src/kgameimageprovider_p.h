/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMEIMAGEPROVIDER_H
#define KGAMEIMAGEPROVIDER_H

// Qt
#include <QQuickImageProvider>
#include <QSvgRenderer>

class KGameThemeProvider;

/**
 * @class KGameImageProvider
 * @short A QDeclarativeImageProvider that renders requested sprites and
 * returns corresponding pixmap to the QML view.
 *
 * This class is a QDeclarativeImageProvider that takes a KGameThemeProvider
 * in its constructor and uses it to get full path to SVGs. These theme
 * SVGs are read and requested sprite pixmap is extracted and given to
 * the QML image element that requests it.
 *
 * For porting KDE games to QML, there is a KGameItem QML component provided
 * by KGameCore QML plugin which is a small wrapper to request pixmaps from
 * this KGameImageProvider. See KGameItem's documentation for details.
 * @since 4.11
 */
class KGameImageProvider : public QQuickImageProvider
{
public:
    /// Construcs a new KGameImageProvider with the supplied KGameThemeProvider
    /// @param provider The KGameThemeProvider used to discover the game's
    /// themes.
    explicit KGameImageProvider(KGameThemeProvider *provider);

    /// Reimplemented method that is called when a sprite pixmap is requested
    QImage requestImage(const QString &source, QSize *size, const QSize &requestedSize) override;

private:
    void reloadRenderer();

    QString m_themeName;
    KGameThemeProvider *m_provider;
    QSvgRenderer m_renderer;
};

#endif // KGAMEIMAGEPROVIDER_H
