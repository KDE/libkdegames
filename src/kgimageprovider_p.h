/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGIMAGEPROVIDER_H
#define KGIMAGEPROVIDER_H

// Qt
#include <QQuickImageProvider>
#include <QSvgRenderer>

class KgThemeProvider;

/**
 * @class KgImageProvider
 * @since 4.11
 * @short A QDeclarativeImageProvider that renders requested sprites and
 * returns corresponding pixmap to the QML view.
 *
 * This class is a QDeclarativeImageProvider that takes a KgThemeProvider
 * in its constructor and uses it to get full path to SVGs. These theme
 * SVGs are read and requested sprite pixmap is extracted and given to
 * the QML image element that requests it.
 *
 * For porting KDE games to QML, there is a KgItem QML component provided
 * by KgCore QML plugin which is a small wrapper to request pixmaps from
 * this KgImageProvider. See KgItem's documentation for details.
 */
class KgImageProvider : public QQuickImageProvider
{
public:
    ///Construcs a new KgImageProvider with the supplied KgThemeProvider
    ///@param provider The KgThemeProvider used to discover the game's
    ///themes.
    explicit KgImageProvider(KgThemeProvider* provider);

    ///Reimplemented method that is called when a sprite pixmap is requested
    QImage requestImage(const QString& source, QSize *size, const QSize &requestedSize) override;

private:
    void reloadRenderer();

    QString m_themeName;
    KgThemeProvider* m_provider;
    QSvgRenderer m_renderer;

};

#endif //KGIMAGEPROVIDER_H
